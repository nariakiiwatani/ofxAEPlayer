#include "ofxAELayer.h"
#include "utils/TransformNode.h"
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
, current_frame_(-1)
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
	
	// Time remap setup - automatically enable if timeRemap data exists
	if(json.contains("timeRemap")) {
		auto &&time_remap_kf = json.value("/keyframes/timeRemap"_json_pointer, ofJson{});
		time_remap_.setup(json["timeRemap"], time_remap_kf);
	}
	
	if(json.contains("mask")) {
		auto &&mask_kf = json.value("/keyframes/mask"_json_pointer, ofJson{});
		mask_.setup(json["mask"], mask_kf);
		mask_collection_.setupFromMaskProp(mask_);
	}

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


bool Layer::setFrame(int frame)
{
	if(current_frame_ == frame) {
		return false;
	}

	bool ret = false;
	bool need_mask_update = false;
	
	if(transform_.setFrame(frame)) {
		TransformData t;
		if (!transform_.tryExtract(t)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract TransformData, using defaults";
		}
		TransformNode::setAnchorPoint(t.anchor);
		TransformNode::setTranslation(t.position);
		TransformNode::setScale(t.scale);
		TransformNode::setRotationZ(t.rotateZ);
		opacity_ = t.opacity;
		ret |= true;
	}

	if(isActiveAtFrame(frame) || isTrackMatte()) {
		if(mask_.setFrame(frame)) {
			mask_collection_.setupFromMaskProp(mask_);
			ret |= true;
			need_mask_update |= true;
		}

		if(source_) {
			int source_frame = frame;
			if(time_remap_.setFrame(frame)) {
				time_remap_.tryExtract(source_frame);
			}
			if(source_->setFrame(source_frame)) {
				ret |= true;
				need_mask_update |= true;
			}
		}
	}
	if(need_mask_update || isTrackMatte()) {
		updateLayerFBO(getWidth(), getHeight());
	}
	current_frame_ = frame;
	return ret;
}

void Layer::draw(float x, float y, float w, float h) const
{
	if(!isActiveAtFrame(current_frame_)) return;

	TransformNode::pushMatrix();
	RenderContext::push();
	RenderContext::setOpacity(opacity_);
	RenderContext::setBlendMode(blend_mode_);
	
	if(isUseFbo()) {
		layer_fbo_.draw(x, y);
	}
	else {
		if(source_) {
			source_->draw(x,y,w,h);
		}
	}
	
	RenderContext::pop();
	TransformNode::popMatrix();
}

void Layer::updateLayerFBO(float w, float h)
{
	if (layer_fbo_.getWidth() != w || layer_fbo_.getHeight() != h) {
		layer_fbo_.allocate(w, h, GL_RGBA);
	}

	layer_fbo_.begin();
	ofClear(0, 0, 0, 0);
	ofPushStyle();
	ofSetColor(255, 255, 255, 255);

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	for (const auto& mask : mask_collection_) {
		ofPath maskPath = mask.toOfPath();
		maskPath.setFilled(true);
		maskPath.setColor({1,1,1,mask.getOpacity()});
		maskPath.draw();
	}
	ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
	if(auto matte = track_matte_layer_.lock()) {
		ofMatrix4x4 relative_mat = *matte->getWorldMatrixInversed() * *getWorldMatrix();
		track_matte_shader_->begin();
		track_matte_shader_->setUniformMatrix4f("uLayerToMatte", relative_mat);
		track_matte_shader_->setUniformTexture("matte", matte->getTexture(), 0);
		ofDrawRectangle(ofGetCurrentViewport());
		track_matte_shader_->end();
	}
	if(source_) {
		source_->draw(0, 0, w, h);
	}
	ofPopStyle();
	layer_fbo_.end();
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
