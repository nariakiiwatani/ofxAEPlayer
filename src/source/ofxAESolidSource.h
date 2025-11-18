#pragma once

#include "ofxAELayerSource.h"
#include "ofJson.h"
#include "ofxAERenderContext.h"
#include <limits>

namespace ofx { namespace ae {

class Visitor;

class SolidSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool setup(const ofJson &json) override {
		size_.x = json["width"];
		size_.y = json["height"];
		color_.r = json["color"][0];
		color_.g = json["color"][1];
		color_.b = json["color"][2];
		return true;
	}
	void update() override {}
	
	bool setFrame(Frame frame) override;
	
	FrameCount getDurationFrames() const override { return std::numeric_limits<FrameCount>::max(); }

	void draw(float x, float y, float w, float h) const override {
		RenderContext::push();
		RenderContext::setColorRGB(color_);
		ofDrawRectangle(x,y,w,h);
		RenderContext::pop();
	}
	float getWidth() const override { return size_.x; }
	float getHeight() const override { return size_.y; }

	SourceType getSourceType() const override { return SourceType::SOLID; }
	std::string getDebugInfo() const override { return "SolidSource"; }
	
	void setColor(const ofFloatColor &color) { color_ = color; }
	void setSize(float width, float height) { size_.x = width; size_.y = height; }
	
private:
	glm::ivec2 size_;
	ofFloatColor color_;
};
}} // namespace ofx::ae
