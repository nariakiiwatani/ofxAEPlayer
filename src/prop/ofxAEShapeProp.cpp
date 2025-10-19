#include "ofxAEShapeProp.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

void EllipseData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void RectangleData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void PolygonData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void FillData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void StrokeData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void GroupData::accept(Visitor& visitor) const {
	visitor.visit(*this);
}

void ShapeData::accept(Visitor& visitor) const {
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

		if (!getProperty<ShapeProp>("/shape")->tryExtract(g.data)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group shape data, using empty";
			g.data.clear();
			success = false;
		}

		return success;
	});
}

}}
