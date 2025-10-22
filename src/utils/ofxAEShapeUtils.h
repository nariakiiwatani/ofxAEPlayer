#pragma once

#include "ofMain.h"
#include "ofxAEShapeProp.h"
#include "ofxAEMaskProp.h"

namespace ofx { namespace ae { namespace utils {

class ShapePathGenerator {
public:
    static float signedArea(const ofPolyline& pl);
    static ofPath enforceWinding(const ofPath& src, int direction);
    static ofPath createEllipsePath(const EllipseData& data);
    static ofPath createRectanglePath(const RectangleData& data);
    static ofPath createPolygonPath(const PolygonData& data);
    static ofPath createMaskPath(const MaskShapeData& data);
};

}}} // namespace ofx::ae::utils
