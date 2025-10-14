#include "ofxAELayer.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include <algorithm>

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

Layer::~Layer() = default;


Layer::TransformProps::TransformProps()
: position(glm::vec3(0.0f))
, scale(glm::vec3(1.0f))
, rotation(glm::vec3(0.0f))
, anchorPoint(glm::vec3(0.0f))
, opacity(1.0f)
{

}

namespace {
template<typename T> bool extract(const ofJson &data, std::string key, T &dst) {
	if(!data.contains(key)) return false;
	dst = data[key].get<T>();
	return true;
}
bool hasKeyframesFor(const ofJson &data, std::string key) {
	return data.contains("keyframes") && data["keyframes"].contains(key);
}
}

void Layer::TransformProps::loadInitialValue(const ofJson &data)
{
}
void Layer::TransformProps::loadAnimation(const ofJson &data)
{
}

bool Layer::setup(const ofJson& json, const std::filesystem::path &base_dir) {
#define EXTRACT(n) extract(json, #n, n)
#define EXTRACT_(n) extract(json, #n, n##_)
	EXTRACT_(name);
	EXTRACT_(in);
	EXTRACT_(out);
	if(json.contains("transform")) {
		transform_.loadInitialValue(json["transform"]);
	}

	if(hasKeyframesFor(json, "transform")) {
		transform_.loadAnimation(json["keyframes"]["transform"]);
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

    return true;
#undef EXTRACT_
#undef EXTRACT
}

void Layer::update()
{
// TODO: frameが変わってたら処理対象
}

void Layer::draw(float x, float y, float w, float h) const
{
	
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
