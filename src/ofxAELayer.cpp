#include "ofxAELayer.h"
#include "ofxAEKeyframe.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include <algorithm>
#include "JsonFuncs.h"

namespace ofx { namespace ae {

// ========================================================================
// Constructor & Destructor
// ========================================================================

Layer::Layer()
: TransformNode()
, source_(nullptr)
, name_("")
, in_(0)
, out_(0)
, blendMode_(BlendMode::NORMAL)
{
}


bool Layer::setup(const ofJson& json, const std::filesystem::path &base_dir) {
#define EXTRACT(n) json::extract(json, #n, n)
#define EXTRACT_(n) json::extract(json, #n, n##_)
	EXTRACT_(name);
	EXTRACT_(in);
	EXTRACT_(out);
	if(json.contains("transform")) {
		auto &&kf = json.value("/keyframes/transform"_json_pointer, ofJson{});
		transform_.setup(json["transform"], kf);
	}

    std::string sourceType = "unknown";
	if(EXTRACT(sourceType)) {
		auto source = LayerSource::createSourceOfType(sourceType);
		if (!source) {
			ofLogWarning("Layer") << "No source created for layer: " << name_;
		} else {
			setSource(std::move(source));
			if (json.contains("source") && json["source"].is_string()) {
				std::string sourcePath = json["source"].get<std::string>();
				source_->load(base_dir / sourcePath);
			}
		}
	}
	else if(json.contains("shape")) {
		auto source = LayerSource::createSourceOfType("shape");
		if (source) {
			// Setup the shape source with the complete JSON data
			if (source->setup(json)) {
				setSource(std::move(source));
			} else {
				ofLogError("Layer") << "Failed to setup shape source for layer: " << name_;
			}
		}
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
	if(transform_.setFrame(frame)) {
		TransformData t;
		if (!transform_.tryExtract(t)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract TransformData, using defaults";
			t = TransformData::getDefault();
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
}} // namespace ofx::ae
