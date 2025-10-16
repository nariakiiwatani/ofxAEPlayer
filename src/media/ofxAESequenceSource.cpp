#include "ofxAESequenceSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool SequenceSource::loadMedia(const std::filesystem::path &filepath) {
    video_player_ = VideoLoader::load(filepath.string());
    if (video_player_ && video_player_->isLoaded()) {
        sequence_path_ = filepath.string();
        // SequencePlayer defaults to 30fps, could be configurable
        frame_rate_ = 30.0f;
        return true;
    }
    ofLogError("SequenceSource") << "Failed to load media: " << filepath;
    return false;
}

int SequenceSource::getTotalFrames() const {
    return video_player_ ? video_player_->getTotalNumFrames() : 0;
}

void SequenceSource::update() {
    // Remove redundant update call - base class already calls video_player_->update()
    MediaSource::update();
}

}} // namespace ofx::ae