#pragma once

#include "ofxAELayerSource.h"
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class StillSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	void draw(float x, float y, float w, float h) const override;
	float getWidth() const override;
	float getHeight() const override;
	SourceType getSourceType() const override { return SourceType::STILL; }
	std::string getDebugInfo() const override;
	
private:
	std::shared_ptr<ofTexture> texture_;
	std::filesystem::path filepath_;
};

}} // namespace ofx::ae
