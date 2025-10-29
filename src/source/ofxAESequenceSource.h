#pragma once

#include "ofxAELayerSource.h"

namespace ofx { namespace ae {

class Visitor;

class SequenceSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	bool setFrame(int frame) override;
	void draw(float x, float y, float w, float h) const override { if(texture_) (*texture_)->draw(x,y,w,h); }
	float getWidth() const override { return pool_.empty() ? 0.f : pool_[0].getWidth(); }
	float getHeight() const override { return pool_.empty() ? 0.f : pool_[0].getHeight(); }
	SourceType getSourceType() const override { return SourceType::SEQUENCE; }
	std::string getDebugInfo() const override { return "SequenceSource"; }
private:
	std::vector<ofTexture> pool_;
	std::optional<ofTexture*> texture_;
};

}} // namespace ofx::ae
