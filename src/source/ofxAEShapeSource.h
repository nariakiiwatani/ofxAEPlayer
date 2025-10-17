#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEShapeProp.h"
#include "ofxAEContentVisitor.h"

namespace ofx { namespace ae {

class ShapeSource : public LayerSource
{
public:
	void accept(ContentVisitor &visitor) override { visitor.visit(*this); }

	bool setup(const ofJson &json) override;
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	bool setFrame(int frame) override;

	SourceType getSourceType() const override { return SHAPE; }
	bool tryExtract(ShapeData &dst) const;
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override { return "ShapeSource"; }

private:
	ShapeProp shape_props_;
};

}} // namespace ofx::ae
