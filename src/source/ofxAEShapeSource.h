#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEShapeProp.h"
#include <limits>

namespace ofx { namespace ae {

class Visitor;
class PathExtractionVisitor;

class ShapeSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;

	bool setup(const ofJson &json) override;
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	
	// Time API
	bool setTime(double time) override;
	double getTime() const override { return current_time_; }
	double getDuration() const override { return std::numeric_limits<double>::max(); }
	
	ofRectangle getBoundingBox() const override;

	SourceType getSourceType() const override { return SourceType::SHAPE; }
	bool tryExtract(ShapeData &dst) const;
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override { return "ShapeSource"; }

private:
	ShapeProp shape_props_;
	ShapeData shape_data_;
	std::shared_ptr<PathExtractionVisitor> visitor_;
	double current_time_ = 0.0;
};

}} // namespace ofx::ae
