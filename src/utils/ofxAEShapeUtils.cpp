#include "ofxAEShapeUtils.h"
#include "ofxAEMaskProp.h"
#include "ofMath.h"
#include <algorithm>
#include <limits>

namespace ofx { namespace ae { namespace utils {

float ShapePathGenerator::signedArea(const ofPolyline &pl)
{
    const auto& v = pl.getVertices();
    if(v.size() < 3) return 0.f;
    double a = 0.0;
    for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
        a += (double)v[j].x * v[i].y - (double)v[i].x * v[j].y;
    }
    return (float)(0.5 * a);
}

ofPath ShapePathGenerator::enforceWinding(const ofPath &src, WindingDirection direction)
{
    int desiredSign = getDesiredSign(direction);
    
    ofPath out;
    out.setMode(src.getMode());
    out.setCurveResolution(src.getCurveResolution());
    for(const auto& poly : src.getOutline()){
        auto v = poly.getVertices();
        if(v.size() < 3) continue;
        bool ccw = signedArea(poly) > 0;
        if((ccw ? +1 : -1) != desiredSign) {
            std::reverse(v.begin(), v.end());
        }
        out.moveTo(v[0]);
        for(size_t i=1;i<v.size();++i) out.lineTo(v[i]);
        out.close();
    }
    return out;
}

ofPath ShapePathGenerator::createPath(const EllipseData &e)
{
    ofPath path;
    path.ellipse(e.position, e.size.x, e.size.y);
    return enforceWinding(path, e.direction);
}

ofPath ShapePathGenerator::createPath(const RectangleData &r)
{
    ofPath path;
    float x=r.position.x - r.size.x*0.5f;
    float y=r.position.y - r.size.y*0.5f;
	if(r.roundness>0) {
		path.rectRounded(x,y,r.size.x,r.size.y,r.roundness);
	}
	else {
		path.rectangle(x,y,r.size.x,r.size.y);
	}
    return enforceWinding(path, r.direction);
}

ofPath ShapePathGenerator::createPath(const PolygonData &polygon)
{
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
    for(int i = 0; i < numPoints; i++) {
        float angle = startAngle + i * angleStep;

        float pointX = polygon.position.x + cos(angle) * outerRadius;
        float pointY = polygon.position.y + sin(angle) * outerRadius;
        
        if (firstPoint) {
            path.moveTo(pointX, pointY);
            firstPoint = false;
        } else {
            path.lineTo(pointX, pointY);
        }
        
        if(isStar) {
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

ofPath ShapePathGenerator::createPath(const PathData &data)
{
    return data.toOfPath();
}

std::optional<ofRectangle> ShapePathGenerator::getBoundingBox(const EllipseData &data)
{
    float halfWidth = data.size.x * 0.5f;
    float halfHeight = data.size.y * 0.5f;
    
    return ofRectangle(
        data.position.x - halfWidth,
        data.position.y - halfHeight,
        data.size.x,
        data.size.y
    );
}

std::optional<ofRectangle> ShapePathGenerator::getBoundingBox(const RectangleData &data)
{
    float halfWidth = data.size.x * 0.5f;
    float halfHeight = data.size.y * 0.5f;
    
    return ofRectangle(
        data.position.x - halfWidth,
        data.position.y - halfHeight,
        data.size.x,
        data.size.y
    );
}

std::optional<ofRectangle> ShapePathGenerator::getBoundingBox(const PolygonData &data)
{
    if(data.points < 3) {
        return std::nullopt;
    }
    
    bool isStar = (data.type == 2);
    float outerRadius = data.outerRadius;
    float innerRadius = isStar ? data.innerRadius : outerRadius;
    
    float angleStep = TWO_PI / data.points;
    float startAngle = data.rotation * DEG_TO_RAD;
    
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    
    for(int i = 0; i < data.points; i++) {
        float angle = startAngle + i * angleStep;
        float x = data.position.x + cos(angle) * outerRadius;
        float y = data.position.y + sin(angle) * outerRadius;
        
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
        
        if(isStar) {
            float innerAngle = angle + angleStep * 0.5f;
            float innerX = data.position.x + cos(innerAngle) * innerRadius;
            float innerY = data.position.y + sin(innerAngle) * innerRadius;
            
            minX = std::min(minX, innerX);
            maxX = std::max(maxX, innerX);
            minY = std::min(minY, innerY);
            maxY = std::max(maxY, innerY);
        }
    }
    
    return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

std::optional<ofRectangle> ShapePathGenerator::getBoundingBox(const PathData &data)
{
    if(data.vertices.empty()) {
        return std::nullopt;
    }
    
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    
    for(const auto &vertex : data.vertices) {
        minX = std::min(minX, vertex.x);
        maxX = std::max(maxX, vertex.x);
        minY = std::min(minY, vertex.y);
        maxY = std::max(maxY, vertex.y);
    }
    
    for(const auto &tangent : data.inTangents) {
        for(size_t i = 0; i < data.vertices.size() && i < data.inTangents.size(); i++) {
            glm::vec2 controlPoint = data.vertices[i] + data.inTangents[i];
            minX = std::min(minX, controlPoint.x);
            maxX = std::max(maxX, controlPoint.x);
            minY = std::min(minY, controlPoint.y);
            maxY = std::max(maxY, controlPoint.y);
        }
    }
    
    for(const auto &tangent : data.outTangents) {
        for(size_t i = 0; i < data.vertices.size() && i < data.outTangents.size(); i++) {
            glm::vec2 controlPoint = data.vertices[i] + data.outTangents[i];
            minX = std::min(minX, controlPoint.x);
            maxX = std::max(maxX, controlPoint.x);
            minY = std::min(minY, controlPoint.y);
            maxY = std::max(maxY, controlPoint.y);
        }
    }
    
    return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

}}} // namespace ofx::ae::utils
