#pragma once

#include "ofxAELayerSource.h"
#include "ofJson.h"
#include "ofxAERenderContext.h"

namespace ofx { namespace ae {

class Visitor;

class SolidSource : public LayerSource
{
public:
	void accept(Visitor& visitor) override;
	bool setup(const ofJson &json) override {
		// TODO: error handling
		size_.x = json["width"];
		size_.y = json["height"];
		color_.r = json["color"][0];
		color_.g = json["color"][1];
		color_.b = json["color"][2];
		return true;
	}
	void update() override {}

	void draw(float x, float y, float w, float h) const override {
		RenderContext::push();
		RenderContext::setColorRGB(color_);
		ofDrawRectangle(x,y,w,h);
		RenderContext::pop();
	}
	float getWidth() const override { return size_.x; }
	float getHeight() const override { return size_.y; }

	SourceType getSourceType() const override { return SOLID; }
	std::string getDebugInfo() const override { return "SolidSource"; }
private:
	glm::ivec2 size_;
	ofFloatColor color_;
};
}} // namespace ofx::ae
