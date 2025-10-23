#include "ofxAEShapeUtils.h"
#include "ofxAEMaskProp.h"
#include "ofMath.h"
#include <algorithm>

namespace ofx { namespace ae { namespace utils {

float ShapePathGenerator::signedArea(const ofPolyline& pl) {
    const auto& v = pl.getVertices();
    if(v.size() < 3) return 0.f;
    double a = 0.0;
    for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
        a += (double)v[j].x * v[i].y - (double)v[i].x * v[j].y;
    }
    return (float)(0.5 * a);
}

ofPath ShapePathGenerator::enforceWinding(const ofPath& src, WindingDirection direction) {
    int desiredSign = getDesiredSign(direction);
    
    ofPath out;
    out.setMode(src.getMode());
    out.setCurveResolution(src.getCurveResolution());
    for(const auto& poly : src.getOutline()){
        auto v = poly.getVertices();
        if(v.size() < 3) continue;
        bool ccw = signedArea(poly) > 0;
        if((ccw ? +1 : -1) != desiredSign){
            std::reverse(v.begin(), v.end());
        }
        out.moveTo(v[0]);
        for(size_t i=1;i<v.size();++i) out.lineTo(v[i]);
        out.close();
    }
    return out;
}

ofPath ShapePathGenerator::createPath(const EllipseData& e) {
    ofPath path;
    path.ellipse(e.position, e.size.x, e.size.y);
    return enforceWinding(path, e.direction);
}

ofPath ShapePathGenerator::createPath(const RectangleData& r) {
    ofPath path;
    float x=r.position.x - r.size.x*0.5f;
    float y=r.position.y - r.size.y*0.5f;
    if(r.roundness>0) path.rectRounded(x,y,r.size.x,r.size.y,r.roundness);
    else              path.rectangle(x,y,r.size.x,r.size.y);
    return enforceWinding(path, r.direction);
}

ofPath ShapePathGenerator::createPath(const PolygonData& polygon) {
    ofPath path;
    
    int numPoints = polygon.points;
    if (numPoints < 3) {
        return path;
    }
    
    bool isStar = (polygon.type == 2);
    float outerRadius = polygon.outerRadius;
    float innerRadius = isStar ? polygon.innerRadius : outerRadius;
    
    float angleStep = TWO_PI / numPoints;
    float startAngle = polygon.rotation * DEG_TO_RAD;
    
    bool firstPoint = true;
    for (int i = 0; i < numPoints; i++) {
        float angle = startAngle + i * angleStep;

        float pointX = polygon.position.x + cos(angle) * outerRadius;
        float pointY = polygon.position.y + sin(angle) * outerRadius;
        
        if (firstPoint) {
            path.moveTo(pointX, pointY);
            firstPoint = false;
        } else {
            path.lineTo(pointX, pointY);
        }
        
        if (isStar) {
            float innerAngle = angle + angleStep * 0.5f;
            float innerPointX = polygon.position.x + cos(innerAngle) * innerRadius;
            float innerPointY = polygon.position.y + sin(innerAngle) * innerRadius;
            path.lineTo(innerPointX, innerPointY);
        }
    }
    path.close();
    
    path = enforceWinding(path, polygon.direction);
    return path;
}

ofPath ShapePathGenerator::createPath(const PathData& data) {
    return data.toOfPath();
}

}}} // namespace ofx::ae::utils
