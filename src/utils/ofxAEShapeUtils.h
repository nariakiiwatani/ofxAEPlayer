#pragma once

#include "ofMain.h"
#include "ofxAEShapeProp.h"
#include "ofxAEMaskProp.h"
#include "ofxAEWindingDirection.h"

namespace ofx { namespace ae { namespace utils {

class ShapePathGenerator {
public:
	static float signedArea(const ofPolyline& pl);
	static ofPath enforceWinding(const ofPath& src, WindingDirection direction);
	static ofPath createPath(const EllipseData& data);
	static ofPath createPath(const RectangleData& data);
	static ofPath createPath(const PolygonData& data);
	static ofPath createPath(const PathData& data);
};

}}} // namespace ofx::ae::utils
