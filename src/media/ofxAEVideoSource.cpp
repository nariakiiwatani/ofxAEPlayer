#include "ofxAEVideoSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool VideoSource::loadMedia(const std::filesystem::path &filepath) {
    video_player_ = VideoLoader::load(filepath.string());
    if (video_player_ && video_player_->isLoaded()) {
        asset_path_ = filepath.string();
        // Add safety check for division by zero
        float duration = video_player_->getDuration();
        if (duration > 0) {
            frame_rate_ = video_player_->getTotalNumFrames() / duration;
        } else {
            frame_rate_ = 30.0f; // Default fallback frame rate
            ofLogWarning("VideoSource") << "Video duration is zero, using default frame rate: " << frame_rate_;
        }
        return true;
    }
    ofLogError("VideoSource") << "Failed to load media: " << filepath;
    return false;
}

int VideoSource::getTotalFrames() const {
    return video_player_ ? video_player_->getTotalNumFrames() : 0;
}

void VideoSource::update() {
    // Remove redundant update call - base class already calls video_player_->update()
    MediaSource::update();
}

}} // namespace ofx::ae