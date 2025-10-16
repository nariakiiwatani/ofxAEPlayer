#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "../ofxAELayerSource.h"
#include "../utils/VideoLoader.h"
#include <filesystem>
#include <memory>

namespace ofx { namespace ae {

class MediaSource : public LayerSource
{
public:
    MediaSource();
    virtual ~MediaSource() = default;

    // LayerSource interface
    bool setup(const ofJson &json) override;
    bool load(const std::filesystem::path &filepath) override;
    void update() override;
    void draw(float x, float y, float w, float h) const override;
    bool setFrame(int frame) override;
    
    float getWidth() const override;
    float getHeight() const override;
    std::string getDebugInfo() const override;

    // Media-specific interface
    virtual bool loadMedia(const std::filesystem::path &filepath) = 0;
    virtual void setFrameRate(float fps) { frame_rate_ = fps; }
    virtual float getFrameRate() const { return frame_rate_; }
    virtual int getTotalFrames() const = 0;
    
    // Frame synchronization
    void setCompositionFrameRate(float comp_fps) { composition_fps_ = comp_fps; }
    float getCompositionFrameRate() const { return composition_fps_; }

protected:
    std::shared_ptr<ofVideoPlayer> video_player_;
    float frame_rate_ = 30.0f;           // Media frame rate
    float composition_fps_ = 30.0f;      // AE composition frame rate
    int current_ae_frame_ = -1;          // Current AE frame
    float media_position_ = 0.0f;        // Current media position [0-1]
    std::filesystem::path base_dir_;     // Base directory for asset resolution
    
    // Time-based frame synchronization
    float aeFrameToMediaTime(int ae_frame) const;
    int mediaTimeToFrame(float time) const;
    
    // Asset path resolution
    std::filesystem::path resolveAssetPath(
        const std::filesystem::path &base_dir,
        const std::string &source_path) const;
};

}} // namespace ofx::ae