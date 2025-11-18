#include "ofLog.h"

#include "ofxAEVisitor.h"
#include "../utils/ofxAEAssetManager.h"
#include "../utils/ofxAETimeUtils.h"

#include "ofxAEVideoSource.h"

namespace ofx { namespace ae {

bool VideoSource::load(const std::filesystem::path &filepath)
{
	filepath_ = filepath;
	player_ = AssetManager::getInstance().getVideo(filepath);
	
	if(player_) {
		ofLogVerbose("VideoSource") << "Loaded video via AssetManager: " << filepath;
		return true;
	}
	else {
		ofLogError("VideoSource") << "Failed to load video: " << filepath;
		return false;
	}
}

bool VideoSource::setFrame(Frame frame)
{
	if(!player_) return false;
	
	if(util::isNearFrame(current_frame_, frame)) {
		return false;
	}
	
	current_frame_ = frame;
	
	double video_time = util::frameToTime(frame, fps_);
	double duration = player_->getDuration();
	
	if(duration > 0.0) {
		float normalized_position = static_cast<float>(video_time / duration);
		player_->setPosition(normalized_position);
	}
	
	return true;
}

FrameCount VideoSource::getDurationFrames() const
{
	if(!player_) return 0.0f;
	return player_->getTotalNumFrames();
}

void VideoSource::update()
{
	if(player_) {
		player_->update();
	}
}

void VideoSource::draw(float x, float y, float w, float h) const
{
	if(player_) {
		player_->draw(x, y, w, h);
	}
}

float VideoSource::getWidth() const
{
	return player_ ? player_->getWidth() : 0.0f;
}

float VideoSource::getHeight() const
{
	return player_ ? player_->getHeight() : 0.0f;
}

std::string VideoSource::getDebugInfo() const
{
	std::ostringstream oss;
	oss << "VideoSource[";
	if(player_) {
		oss << filepath_.filename().string() << ", "
			<< getWidth() << "x" << getHeight();
	}
	else {
		oss << "no video";
	}
	oss << "]";
	return oss.str();
}

void VideoSource::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

}} // namespace ofx::ae
