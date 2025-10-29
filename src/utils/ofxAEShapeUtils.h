#pragma once

#include "ofMain.h"
#include "../prop/ofxAEShapeProp.h"
#include "../prop/ofxAEMaskProp.h"
#include "../data/Enums.h"

namespace ofx { namespace ae { namespace utils {

class ShapePathGenerator
{
public:
	static float signedArea(const ofPolyline &pl);
	static ofPath enforceWinding(const ofPath &src, WindingDirection direction);
	static ofPath createPath(const EllipseData &data);
	static ofPath createPath(const RectangleData &data);
	static ofPath createPath(const PolygonData &data);
	static ofPath createPath(const PathData &data);

	static std::optional<ofRectangle> getBoundingBox(const EllipseData &data);
	static std::optional<ofRectangle> getBoundingBox(const RectangleData &data);
	static std::optional<ofRectangle> getBoundingBox(const PolygonData &data);
	static std::optional<ofRectangle> getBoundingBox(const PathData &data);
};

}}} // namespace ofx::ae::utils
