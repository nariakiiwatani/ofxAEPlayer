#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEShapeProp.h"

namespace ofx { namespace ae {

class Visitor;

class ShapeSource : public LayerSource
{
public:
	void accept(Visitor& visitor) override;

	bool setup(const ofJson &json) override;
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	bool setFrame(int frame) override;
	ofRectangle getBoundingBox() const override;

	SourceType getSourceType() const override { return SHAPE; }
	bool tryExtract(ShapeData &dst) const;
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override { return "ShapeSource"; }

private:
	ShapeProp shape_props_;
};

}} // namespace ofx::ae
