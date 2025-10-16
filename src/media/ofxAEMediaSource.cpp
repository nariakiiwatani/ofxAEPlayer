#include "ofxAEMediaSource.h"
#include "ofLog.h"
#include "../utils/JsonFuncs.h"

namespace ofx { namespace ae {

MediaSource::MediaSource()
: LayerSource()
, video_player_(nullptr)
, frame_rate_(30.0f)
, composition_fps_(30.0f)
, current_ae_frame_(-1)
, media_position_(0.0f)
{
}

bool MediaSource::setup(const ofJson &json) {
    std::filesystem::path asset_path;
    
    if (json.contains("source") && json["source"].is_string()) {
        // Explicit source path provided
        std::string source_path = json["source"].get<std::string>();
        asset_path = resolveAssetPath(base_dir_, source_path);
    } else {
        // Try to infer from layer name
        std::string name = json.value("name", "");
        asset_path = resolveAssetPath(base_dir_, "sources/" + name);
        
        // For sequences, name might be directory
        if (!std::filesystem::exists(asset_path)) {
            asset_path = resolveAssetPath(base_dir_, "sources");
            asset_path /= std::filesystem::path(name).stem(); // Remove extension
        }
    }
    
    if (!loadMedia(asset_path)) {
        ofLogError("MediaSource") << "Failed to load media from asset path: " << asset_path;
        return false;
    }
    return true;
}

bool MediaSource::load(const std::filesystem::path &filepath) {
    base_dir_ = filepath.parent_path();
    if (!loadMedia(filepath)) {
        ofLogError("MediaSource") << "Failed to load media from filepath: " << filepath;
        return false;
    }
    return true;
}

void MediaSource::update() {
    if (video_player_) {
        video_player_->update();
    }
}

void MediaSource::draw(float x, float y, float w, float h) const {
    if (video_player_ && video_player_->isLoaded()) {
        video_player_->draw(x, y, w, h);
    }
}

bool MediaSource::setFrame(int frame) {
    if (current_ae_frame_ == frame) return false;
    
    current_ae_frame_ = frame;
    media_position_ = aeFrameToMediaTime(frame);
    
    if (video_player_ && video_player_->isLoaded()) {
        video_player_->setPosition(media_position_);
        return true;
    }
    
    return false;
}

float MediaSource::getWidth() const {
    if (!video_player_) {
        ofLogWarning("MediaSource") << "Attempting to get width from null video player";
        return 0.0f;
    }
    return video_player_->getWidth();
}

float MediaSource::getHeight() const {
    if (!video_player_) {
        ofLogWarning("MediaSource") << "Attempting to get height from null video player";
        return 0.0f;
    }
    return video_player_->getHeight();
}

std::string MediaSource::getDebugInfo() const {
    return "MediaSource";
}

float MediaSource::aeFrameToMediaTime(int ae_frame) const {
    // Convert AE frame to time using composition frame rate
    float ae_time = ae_frame / composition_fps_;
    
    // Convert to media position [0-1] with safety checks
    if (frame_rate_ <= 0) {
        ofLogWarning("MediaSource") << "Invalid frame rate: " << frame_rate_;
        return 0.0f;
    }
    float media_duration = getTotalFrames() / frame_rate_;
    return (media_duration > 0) ? (ae_time / media_duration) : 0.0f;
}

int MediaSource::mediaTimeToFrame(float time) const {
    return static_cast<int>(time * frame_rate_);
}

std::filesystem::path MediaSource::resolveAssetPath(
    const std::filesystem::path &base_dir,
    const std::string &source_path) const {
    
    // Handle relative paths from AE export
    if (source_path.starts_with("../")) {
        return base_dir / source_path;
    }
    
    // Handle absolute paths
    if (std::filesystem::path(source_path).is_absolute()) {
        return source_path;
    }
    
    // Handle filename-only (search in sources directory)
    return base_dir / "sources" / source_path;
}

}} // namespace ofx::ae