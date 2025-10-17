#pragma once

#include "ofxAELayerSource.h"

namespace ofx { namespace ae {

class SequenceSource : public LayerSource
{
public:
	bool load(const std::filesystem::path &filepath) override;
	bool setFrame(int frame) override;
	void draw(float x, float y, float w, float h) const override { if(texture_) (*texture_)->draw(x,y,w,h); }
	float getWidth() const override { return texture_ ? (*texture_)->getWidth() : 0.f; }
	float getHeight() const override { return texture_ ? (*texture_)->getHeight() : 0.f; }
    SourceType getSourceType() const override { return SEQUENCE; }
    std::string getDebugInfo() const override { return "SequenceSource"; }
private:
	std::vector<ofTexture> pool_;
	std::optional<ofTexture*> texture_;
};

}} // namespace ofx::ae
