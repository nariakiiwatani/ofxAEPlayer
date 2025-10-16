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

}}