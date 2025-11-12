#pragma once

#include "ofxAELayerSource.h"
#include <memory>
#include <limits>

namespace ofx { namespace ae {

class Visitor;

class StillSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	
	// Time API (still images don't change with time, but need interface)
	bool setTime(double time) override { current_time_ = time; return false; }
	double getTime() const override { return current_time_; }
	double getDuration() const override { return std::numeric_limits<double>::max(); }
	
	void draw(float x, float y, float w, float h) const override;
	float getWidth() const override;
	float getHeight() const override;
	SourceType getSourceType() const override { return SourceType::STILL; }
	std::string getDebugInfo() const override;
	
private:
	std::shared_ptr<ofTexture> texture_;
	std::filesystem::path filepath_;
	double current_time_ = 0.0;
};

}} // namespace ofx::ae
