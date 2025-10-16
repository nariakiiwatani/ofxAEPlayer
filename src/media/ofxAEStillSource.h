#pragma once

#include "ofxAEMediaSource.h"

namespace ofx { namespace ae {

class StillSource : public MediaSource
{
public:
    StillSource() = default;
    virtual ~StillSource() = default;

    // LayerSource interface
    SourceType getSourceType() const override { return STILL; }
    std::string getDebugInfo() const override { return "StillSource"; }
    
    // MediaSource interface
    bool loadMedia(const std::filesystem::path &filepath) override;
    int getTotalFrames() const override { return 1; }
    
    // Still images don't need frame updates
    bool setFrame(int frame) override {
        // Call parent method to maintain proper state management
        MediaSource::setFrame(frame);
        return false; // No visual change
    }

private:
    std::string asset_path_;
};

}} // namespace ofx::ae