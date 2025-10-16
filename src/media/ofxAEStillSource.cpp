#include "ofxAEStillSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool StillSource::loadMedia(const std::filesystem::path &filepath) {
    video_player_ = VideoLoader::load(filepath.string());
    if (video_player_ && video_player_->isLoaded()) {
        asset_path_ = filepath.string();
        return true;
    }
    ofLogError("StillSource") << "Failed to load media: " << filepath;
    return false;
}

}} // namespace ofx::ae