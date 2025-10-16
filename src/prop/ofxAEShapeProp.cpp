#include "ofxAEShapeProp.h"

namespace ofx { namespace ae {

void EllipseData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

void RectangleData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

void PolygonData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

void FillData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

void StrokeData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

void GroupData::accept(ShapeVisitor& visitor) const {
	visitor.visit(*this);
}

GroupProp::GroupProp() {
	registerProperty<IntProp>("/blendMode");
	registerProperty<ShapeProp>("/shape");

	registerExtractor<GroupData>([this](GroupData& g) -> bool {
		bool success = true;

		if (!getProperty<IntProp>("/blendMode")->tryExtract(g.blendMode)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group blendMode, using default";
			g.blendMode = 1;
			success = false;
		}

		// Extract child shapes into GroupData::data
		if (!getProperty<ShapeProp>("/shape")->tryExtract(g.data)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group shape data, using empty";
			g.data.clear();
			success = false;
		}

		return success;
	});
}

}}
