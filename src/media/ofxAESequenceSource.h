#pragma once

#include "ofxAEMediaSource.h"

namespace ofx { namespace ae {

class SequenceSource : public MediaSource
{
public:
    SequenceSource() = default;
    virtual ~SequenceSource() = default;

    // LayerSource interface
    SourceType getSourceType() const override { return SEQUENCE; }
    std::string getDebugInfo() const override { return "SequenceSource"; }
    
    // MediaSource interface
    bool loadMedia(const std::filesystem::path &filepath) override;
    int getTotalFrames() const override;
    
    void update() override;

private:
    std::string sequence_path_;
};

}} // namespace ofx::ae