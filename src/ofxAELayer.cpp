#include "ofxAELayer.h"
#include "ofxAEKeyframe.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include <algorithm>
#include "JsonFuncs.h"

namespace ofx { namespace ae {

namespace {
std::vector<Layer::SourceResolver> BUILTIN_RESOLVERS = {
	[](const ofJson& json, const std::filesystem::path& base_dir) -> std::unique_ptr<LayerSource> {
		std::string sourceType = json.value("sourceType", "none");
		auto source = LayerSource::createSourceOfType(sourceType);

		if (!source) {
			ofLogWarning("SourceTypeResolver") << "No source created for sourceType: " << sourceType;
			return nullptr;
		}

		if (json.contains("source") && json["source"].is_string()) {
			std::string sourcePath = json["source"].get<std::string>();
			if (!source->load(base_dir / sourcePath)) {
				ofLogWarning("SourceTypeResolver") << "Failed to load source: " << sourcePath;
				return nullptr;
			}
		}

		return source;
	},
	[](const ofJson& json, const std::filesystem::path& base_dir) -> std::unique_ptr<LayerSource> {
		auto source = LayerSource::createSourceOfType("shape");
		if (!source) {
			ofLogError("ShapeResolver") << "Failed to create shape source";
			return nullptr;
		}

		if (!source->setup(json)) {
			ofLogError("ShapeResolver") << "Failed to setup shape source";
			return nullptr;
		}

		return source;
	}
};
}

std::vector<Layer::SourceResolver> Layer::resolvers_;

void Layer::registerResolver(SourceResolver resolver) {
	resolvers_.push_back(resolver);
}

void Layer::clearResolvers() {
	resolvers_.clear();
}

Layer::Layer()
: TransformNode()
, source_(nullptr)
, name_("")
, in_(0)
, out_(0)
, blend_mode_(BlendMode::NORMAL)
{
}


std::unique_ptr<LayerSource> Layer::resolveSource(const ofJson& json, const std::filesystem::path& base_dir)
{
	std::vector<SourceResolver> resolvers = BUILTIN_RESOLVERS;
	std::copy(resolvers_.begin(), resolvers_.end(), std::back_inserter(resolvers));

	for(const auto& resolver : resolvers) {
		if(auto source = resolver(json, base_dir)) {
			return source;
		}
	}
	return nullptr;
}

bool Layer::setup(const ofJson& json, const std::filesystem::path &base_dir) {
#define EXTRACT(n) json::extract(json, #n, n)
#define EXTRACT_(n) json::extract(json, #n, n##_)
	EXTRACT_(name);
	EXTRACT_(in);
	EXTRACT_(out);
	std::string blendingMode = "NORMAL";
	EXTRACT(blendingMode);
	blend_mode_ = blendModeFromString(blendingMode);
	if(json.contains("transform")) {
		auto &&kf = json.value("/keyframes/transform"_json_pointer, ofJson{});
		transform_.setup(json["transform"], kf);
	}

	// Use resolver chain to create source
	auto source = resolveSource(json, base_dir);
	if (source) {
		setSource(std::move(source));
	} else {
		ofLogVerbose("Layer") << "No source resolved for layer: " << name_;
	}

	current_frame_ = -1;
    return true;
#undef EXTRACT_
#undef EXTRACT
}

void Layer::update()
{
	refreshMatrix();

	if (source_) {
		source_->update();
	}
}

bool Layer::tryExtractTransform(TransformData &transform) const
{
	return transform_.tryExtract(transform);
}


bool Layer::setFrame(int frame)
{
	if(current_frame_ == frame) {
		return false;
	}
	bool ret = false;
	if(transform_.setFrame(frame)) {
		TransformData t;
		if (!transform_.tryExtract(t)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract TransformData, using defaults";
		}
		TransformNode::setAnchorPoint(t.anchor);
		setTranslation(t.position);
		setScale(t.scale);
		setRotationZ(t.rotateZ);
		opacity_ = t.opacity;
		ret |= true;
	}
	if(source_) {
		ret |= source_->setFrame(frame);
	}
	current_frame_ = frame;
	return ret;
}

void Layer::draw(float x, float y, float w, float h) const
{
	pushMatrix();
	RenderContext::push();
	RenderContext::setOpacity(opacity_);
	RenderContext::setBlendMode(blend_mode_);
	if(source_) {
		source_->draw(x,y,w,h);
	}
	RenderContext::pop();
	popMatrix();
}



float Layer::getHeight() const {
    if (source_) {
        return source_->getHeight();
    }
    return 0.0f;
}

float Layer::getWidth() const {
    if (source_) {
        return source_->getWidth();
    }
    return 0.0f;
}

void Layer::setSource(std::unique_ptr<LayerSource> source) {
    source_ = std::move(source);
}

LayerSource::SourceType Layer::getSourceType() const {
    if (source_) {
        return source_->getSourceType();
    }
    
	return LayerSource::UNKNOWN;
}

bool Layer::isActiveAtFrame(int frame) const {
    return in_ <= frame && frame < out_;
}

int Layer::getLocalFrame(int global) const {
    return global - in_;
}

bool Layer::shouldRender(int frame) const {
    return isActiveAtFrame(frame) && source_;
}


std::string Layer::getDebugInfo() const {
    std::stringstream info;
    info << "Layer[" << name_ << "] ";
    info << "Source: " << (source_ ? source_->getDebugInfo() : "None") << " ";
    return info.str();
}

bool Layer::load(const std::string &filepath) {
	ofJson json = ofLoadJson(filepath);
	auto base_dir = ofFilePath::getEnclosingDirectory(filepath);
	return setup(json, base_dir);
}

void Layer::accept(Visitor& visitor) {
	visitor.visit(*this);
}
}} // namespace ofx::ae
