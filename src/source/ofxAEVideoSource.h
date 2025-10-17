#pragma once

#include "ofxAELayerSource.h"
#include "ofVideoPlayer.h"

namespace ofx { namespace ae {

class VideoSource : public LayerSource
{
public:
	bool load(const std::filesystem::path &filepath) override;
	bool setFrame(int frame) override;
	void draw(float x, float y, float w, float h) const override { player_.draw(x,y,w,h); }
	float getWidth() const override { return player_.getWidth(); }
	float getHeight() const override { return player_.getHeight(); }
    SourceType getSourceType() const override { return VIDEO; }
    std::string getDebugInfo() const override { return "VideoSource"; }

private:
	ofVideoPlayer player_;
};

}} // namespace ofx::ae
