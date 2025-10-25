#include "ofxAEVideoSource.h"
#include "ofxAEVisitor.h"
#include "../utils/ofxAEAssetManager.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool VideoSource::load(const std::filesystem::path &filepath)
{
	filepath_ = filepath;
	player_ = AssetManager::getInstance().getVideo(filepath);
	
	if (player_) {
		ofLogVerbose("VideoSource") << "Loaded video via AssetManager: " << filepath;
		return true;
	} else {
		ofLogError("VideoSource") << "Failed to load video: " << filepath;
		return false;
	}
}

bool VideoSource::setFrame(int frame)
{
	if (!player_) return false;
	
	player_->setFrame(frame);
	player_->update();
	return player_->isFrameNew();
}

void VideoSource::draw(float x, float y, float w, float h) const {
	if (player_) {
		player_->draw(x, y, w, h);
	}
}

float VideoSource::getWidth() const {
	return player_ ? player_->getWidth() : 0.0f;
}

float VideoSource::getHeight() const {
	return player_ ? player_->getHeight() : 0.0f;
}

std::string VideoSource::getDebugInfo() const {
	std::ostringstream oss;
	oss << "VideoSource[";
	if (player_) {
		oss << filepath_.filename().string() << ", "
		    << getWidth() << "x" << getHeight();
	} else {
		oss << "no video";
	}
	oss << "]";
	return oss.str();
}

void VideoSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
