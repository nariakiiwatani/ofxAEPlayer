#include "ofxAEStillSource.h"
#include "ofxAEVisitor.h"
#include "ofxAEAssetManager.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool StillSource::load(const std::filesystem::path &filepath) {
	filepath_ = filepath;
	texture_ = AssetManager::getInstance().getTexture(filepath);
	
	if (texture_) {
		ofLogVerbose("StillSource") << "Loaded texture via AssetManager: " << filepath;
		return true;
	} else {
		ofLogError("StillSource") << "Failed to load texture: " << filepath;
		return false;
	}
}

void StillSource::draw(float x, float y, float w, float h) const {
	if (texture_) {
		texture_->draw(x, y, w, h);
	}
}

float StillSource::getWidth() const {
	return texture_ ? texture_->getWidth() : 0.0f;
}

float StillSource::getHeight() const {
	return texture_ ? texture_->getHeight() : 0.0f;
}

std::string StillSource::getDebugInfo() const {
	std::ostringstream oss;
	oss << "StillSource[";
	if (texture_) {
		oss << filepath_.filename().string() << ", "
		    << getWidth() << "x" << getHeight();
	} else {
		oss << "no texture";
	}
	oss << "]";
	return oss.str();
}

void StillSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
