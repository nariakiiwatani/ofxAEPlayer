#pragma once

#include "ofxAEMediaSource.h"

namespace ofx { namespace ae {

class VideoSource : public MediaSource
{
public:
    VideoSource() = default;
    virtual ~VideoSource() = default;

    // LayerSource interface
    SourceType getSourceType() const override { return VIDEO; }
    std::string getDebugInfo() const override { return "VideoSource"; }
    
    // MediaSource interface
    bool loadMedia(const std::filesystem::path &filepath) override;
    int getTotalFrames() const override;
    
    void update() override;

private:
    std::string asset_path_;
};

}} // namespace ofx::ae