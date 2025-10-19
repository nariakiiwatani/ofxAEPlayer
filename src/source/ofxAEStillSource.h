#pragma once

#include "ofxAELayerSource.h"

namespace ofx { namespace ae {

class Visitor;

class StillSource : public LayerSource
{
public:
	void accept(Visitor& visitor) override;
	bool load(const std::filesystem::path &filepath) override { return ofLoadImage(texture_, filepath); }
	void draw(float x, float y, float w, float h) const override { texture_.draw(x,y,w,h); }
	float getWidth() const override { return texture_.getWidth(); }
	float getHeight() const override { return texture_.getHeight(); }
	   SourceType getSourceType() const override { return STILL; }
	   std::string getDebugInfo() const override { return "StillSource"; }
private:
	ofTexture texture_;
};

}} // namespace ofx::ae
