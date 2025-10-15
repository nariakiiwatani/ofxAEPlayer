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
	auto &&keyframes = json.value("/keyframes"_json_pointer, ofJson{});
	if(json.contains("transform")) {
		if(keyframes.contains("transform")) {
			transform_.setup(json["transform"], keyframes["transform"]);
		}
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
//	else if(json.contains("shape")) {
//		auto source = LayerSourceFactory::createSourceOfType("shape");
//		setSource(std::move(source));
//		source_->loadInitialValue(json["shape"]);
//		if(hasKeyframesFor(json, "shape")) {
//			source_->loadAnimation(json["keyframes"]["shape"]);
//		}
//	}

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
		transform_.extract(t);
		TransformNode::setAnchorPoint(t.anchor);
		setTranslation(t.position);
		setScale(t.scale);
		opacity_ = t.opacity;
		ret |= true;
	}
	current_frame_ = frame;
	return ret;
}

void Layer::draw(float x, float y, float w, float h) const
{
	pushMatrix();
	if(source_) {
		source_->draw(x,y,w,h);
	}
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
