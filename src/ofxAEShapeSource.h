#pragma once

#include "ofxAELayerSource.h"
#include "prop/ofxAEShapeProp.h"
#include "ofxAERenderGroup.h"

namespace ofx { namespace ae {

class ShapeSource : public LayerSource
{
public:
	bool setup(const ofJson &json) override;
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	bool setFrame(int frame) override;

	SourceType getSourceType() const override { return SHAPE; }
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override { return "ShapeSource"; }

private:
	ShapeProp shape_props_;
	
	// RenderGroupシステムのコンポーネント
	std::unique_ptr<RenderGroupProcessor> renderProcessor_;
	std::unique_ptr<RenderItemRenderer> renderer_;
};

}} // namespace ofx::ae
