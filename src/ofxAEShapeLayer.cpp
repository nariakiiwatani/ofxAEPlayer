#include "ofxAEShapeLayer.h"
#include "ofGraphics.h"
#include "ofLog.h"

namespace ofx {
namespace ae {

// ===== SHAPE PATH IMPLEMENTATION =====

ShapePath::ShapePath() : closed(false) {}

ShapePath::~ShapePath() {}

void ShapePath::addVertex(const PathVertex& vertex) {
    vertices.push_back(vertex);
}

void ShapePath::addVertex(const glm::vec2& position) {
    vertices.emplace_back(position);
}

void ShapePath::addVertex(const glm::vec2& position, const glm::vec2& inTangent, const glm::vec2& outTangent) {
    vertices.emplace_back(position, inTangent, outTangent);
}

void ShapePath::clear() {
    vertices.clear();
}

glm::vec2 ShapePath::evaluateAt(float t) const {
    if (vertices.empty()) return glm::vec2(0, 0);
    if (vertices.size() == 1) return vertices[0].position;
    
    // Normalize t to path segments
    float segmentCount = closed ? vertices.size() : vertices.size() - 1;
    if (segmentCount <= 0) return vertices[0].position;
    
    float segmentFloat = t * segmentCount;
    int segmentIndex = (int)floor(segmentFloat);
    float segmentT = segmentFloat - segmentIndex;
    
    // Handle boundary conditions
    if (segmentIndex >= segmentCount) {
        segmentIndex = segmentCount - 1;
        segmentT = 1.0f;
    }
    
    // Get segment vertices
    const PathVertex& v0 = vertices[segmentIndex];
    const PathVertex& v1 = vertices[(segmentIndex + 1) % vertices.size()];
    
    // Calculate bezier control points
    glm::vec2 p0 = v0.position;
    glm::vec2 p1 = v0.position + v0.outTangent;
    glm::vec2 p2 = v1.position + v1.inTangent;
    glm::vec2 p3 = v1.position;
    
    // Cubic bezier evaluation
    float oneMinusT = 1.0f - segmentT;
    float t2 = segmentT * segmentT;
    float t3 = t2 * segmentT;
    float oneMinusT2 = oneMinusT * oneMinusT;
    float oneMinusT3 = oneMinusT2 * oneMinusT;
    
    return oneMinusT3 * p0 + 3.0f * oneMinusT2 * segmentT * p1 + 
           3.0f * oneMinusT * t2 * p2 + t3 * p3;
}

void ShapePath::generatePolyline(ofPolyline& polyline, int resolution) const {
    polyline.clear();
    
    if (vertices.empty()) return;
    
    if (vertices.size() == 1) {
        polyline.addVertex(vertices[0].position.x, vertices[0].position.y);
        return;
    }
    
    // Generate points along the path
    for (int i = 0; i <= resolution; i++) {
        float t = (float)i / (float)resolution;
        glm::vec2 point = evaluateAt(t);
        polyline.addVertex(point.x, point.y);
    }
    
    if (closed) {
        polyline.close();
    }
}

void ShapePath::generatePath(ofPath& path) const {
    path.clear();
    
    if (vertices.empty()) return;
    
    if (vertices.size() == 1) {
        path.moveTo(vertices[0].position.x, vertices[0].position.y);
        return;
    }
    
    // Start path
    const PathVertex& firstVertex = vertices[0];
    path.moveTo(firstVertex.position.x, firstVertex.position.y);
    
    // Add bezier curves for each segment
    for (size_t i = 0; i < vertices.size(); i++) {
        size_t nextIndex = (i + 1) % vertices.size();
        
        // Skip last segment if not closed
        if (!closed && nextIndex == 0) break;
        
        const PathVertex& currentVertex = vertices[i];
        const PathVertex& nextVertex = vertices[nextIndex];
        
        // Calculate control points
        glm::vec2 cp1 = currentVertex.position + currentVertex.outTangent;
        glm::vec2 cp2 = nextVertex.position + nextVertex.inTangent;
        
        // Add bezier curve
        path.bezierTo(cp1.x, cp1.y, cp2.x, cp2.y, nextVertex.position.x, nextVertex.position.y);
    }
    
    if (closed) {
        path.close();
    }
}

ofRectangle ShapePath::getBounds() const {
    if (vertices.empty()) return ofRectangle();
    
    // Start with first vertex
    glm::vec2 min = vertices[0].position;
    glm::vec2 max = vertices[0].position;
    
    // Check all vertices and control points
    for (const auto& vertex : vertices) {
        // Check vertex position
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
        
        // Check control points
        glm::vec2 cp1 = vertex.position + vertex.inTangent;
        glm::vec2 cp2 = vertex.position + vertex.outTangent;
        
        min.x = std::min(min.x, std::min(cp1.x, cp2.x));
        min.y = std::min(min.y, std::min(cp1.y, cp2.y));
        max.x = std::max(max.x, std::max(cp1.x, cp2.x));
        max.y = std::max(max.y, std::max(cp1.y, cp2.y));
    }
    
    return ofRectangle(min.x, min.y, max.x - min.x, max.y - min.y);
}

bool ShapePath::loadFromJson(const ofJson& pathData) {
    try {
        clear();
        
        if (!pathData.contains("vertices")) return false;
        
        const auto& verticesData = pathData["vertices"];
        const auto& inTangentsData = pathData.contains("inTangents") ? pathData["inTangents"] : ofJson::array();
        const auto& outTangentsData = pathData.contains("outTangents") ? pathData["outTangents"] : ofJson::array();
        
        for (size_t i = 0; i < verticesData.size(); i++) {
            const auto& vertexData = verticesData[i];
            if (!vertexData.is_array() || vertexData.size() < 2) continue;
            
            PathVertex vertex;
            vertex.position = glm::vec2(vertexData[0], vertexData[1]);
            
            // Load tangents if available
            if (i < inTangentsData.size() && inTangentsData[i].is_array() && inTangentsData[i].size() >= 2) {
                vertex.inTangent = glm::vec2(inTangentsData[i][0], inTangentsData[i][1]);
            }
            
            if (i < outTangentsData.size() && outTangentsData[i].is_array() && outTangentsData[i].size() >= 2) {
                vertex.outTangent = glm::vec2(outTangentsData[i][0], outTangentsData[i][1]);
            }
            
            addVertex(vertex);
        }
        
        if (pathData.contains("closed")) {
            setClosed(pathData["closed"]);
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapePath") << "Failed to load path from JSON: " << e.what();
        return false;
    }
}

ofJson ShapePath::toJson() const {
    ofJson json;
    
    // Export vertices
    ofJson verticesJson = ofJson::array();
    ofJson inTangentsJson = ofJson::array();
    ofJson outTangentsJson = ofJson::array();
    
    for (const auto& vertex : vertices) {
        verticesJson.push_back({vertex.position.x, vertex.position.y});
        inTangentsJson.push_back({vertex.inTangent.x, vertex.inTangent.y});
        outTangentsJson.push_back({vertex.outTangent.x, vertex.outTangent.y});
    }
    
    json["vertices"] = verticesJson;
    json["inTangents"] = inTangentsJson;
    json["outTangents"] = outTangentsJson;
    json["closed"] = closed;
    
    return json;
}

// ===== SHAPE ELLIPSE IMPLEMENTATION =====

ShapeEllipse::ShapeEllipse() : ShapeGeometry(ELLIPSE), size_(100, 100), position_(0, 0) {}

void ShapeEllipse::generatePath(ofPath& path) const {
    path.clear();
    
    // Generate ellipse using bezier curves (4 control points)
    float radiusX = size_.x * 0.5f;
    float radiusY = size_.y * 0.5f;
    float centerX = position_.x;
    float centerY = position_.y;
    
    // Magic number for bezier approximation of circle: 4/3 * (sqrt(2) - 1)
    float kappa = 0.5522847498f;
    float kappaX = kappa * radiusX;
    float kappaY = kappa * radiusY;
    
    // Start at top
    path.moveTo(centerX, centerY - radiusY);
    
    // Top-right quadrant
    path.bezierTo(centerX + kappaX, centerY - radiusY,
                  centerX + radiusX, centerY - kappaY,
                  centerX + radiusX, centerY);
    
    // Bottom-right quadrant
    path.bezierTo(centerX + radiusX, centerY + kappaY,
                  centerX + kappaX, centerY + radiusY,
                  centerX, centerY + radiusY);
    
    // Bottom-left quadrant
    path.bezierTo(centerX - kappaX, centerY + radiusY,
                  centerX - radiusX, centerY + kappaY,
                  centerX - radiusX, centerY);
    
    // Top-left quadrant
    path.bezierTo(centerX - radiusX, centerY - kappaY,
                  centerX - kappaX, centerY - radiusY,
                  centerX, centerY - radiusY);
    
    path.close();
}

ofRectangle ShapeEllipse::getBounds() const {
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    return ofRectangle(position_.x - halfWidth, position_.y - halfHeight, size_.x, size_.y);
}

bool ShapeEllipse::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("size") && data["size"].is_array() && data["size"].size() >= 2) {
            size_ = glm::vec2(data["size"][0], data["size"][1]);
        }
        
        if (data.contains("position") && data["position"].is_array() && data["position"].size() >= 2) {
            position_ = glm::vec2(data["position"][0], data["position"][1]);
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapeEllipse") << "Failed to load from JSON: " << e.what();
        return false;
    }
}

ofJson ShapeEllipse::toJson() const {
    ofJson json;
    json["type"] = "ellipse";
    json["size"] = {size_.x, size_.y};
    json["position"] = {position_.x, position_.y};
    return json;
}

std::unique_ptr<ShapeGeometry> ShapeEllipse::clone() const {
    auto copy = std::make_unique<ShapeEllipse>();
    copy->size_ = size_;
    copy->position_ = position_;
    return std::move(copy);
}

// ===== SHAPE RECTANGLE IMPLEMENTATION =====

ShapeRectangle::ShapeRectangle() : ShapeGeometry(RECTANGLE), size_(100, 100), position_(0, 0), roundness_(0) {}

void ShapeRectangle::generatePath(ofPath& path) const {
    path.clear();
    
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    float left = position_.x - halfWidth;
    float right = position_.x + halfWidth;
    float top = position_.y - halfHeight;
    float bottom = position_.y + halfHeight;
    
    if (roundness_ <= 0) {
        // Simple rectangle
        path.rectangle(left, top, size_.x, size_.y);
    } else {
        // Rounded rectangle
        float radius = std::min(roundness_, std::min(halfWidth, halfHeight));
        
        // Start at top-left corner (after radius)
        path.moveTo(left + radius, top);
        
        // Top edge
        path.lineTo(right - radius, top);
        
        // Top-right corner
        path.arc(right - radius, top + radius, radius, radius, -90, 0);
        
        // Right edge
        path.lineTo(right, bottom - radius);
        
        // Bottom-right corner
        path.arc(right - radius, bottom - radius, radius, radius, 0, 90);
        
        // Bottom edge
        path.lineTo(left + radius, bottom);
        
        // Bottom-left corner
        path.arc(left + radius, bottom - radius, radius, radius, 90, 180);
        
        // Left edge
        path.lineTo(left, top + radius);
        
        // Top-left corner
        path.arc(left + radius, top + radius, radius, radius, 180, 270);
        
        path.close();
    }
}

ofRectangle ShapeRectangle::getBounds() const {
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    return ofRectangle(position_.x - halfWidth, position_.y - halfHeight, size_.x, size_.y);
}

bool ShapeRectangle::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("size") && data["size"].is_array() && data["size"].size() >= 2) {
            size_ = glm::vec2(data["size"][0], data["size"][1]);
        }
        
        if (data.contains("position") && data["position"].is_array() && data["position"].size() >= 2) {
            position_ = glm::vec2(data["position"][0], data["position"][1]);
        }
        
        if (data.contains("roundness")) {
            roundness_ = data["roundness"];
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapeRectangle") << "Failed to load from JSON: " << e.what();
        return false;
    }
}

ofJson ShapeRectangle::toJson() const {
    ofJson json;
    json["type"] = "rectangle";
    json["size"] = {size_.x, size_.y};
    json["position"] = {position_.x, position_.y};
    json["roundness"] = roundness_;
    return json;
}

std::unique_ptr<ShapeGeometry> ShapeRectangle::clone() const {
    auto copy = std::make_unique<ShapeRectangle>();
    copy->size_ = size_;
    copy->position_ = position_;
    copy->roundness_ = roundness_;
    return std::move(copy);
}

// ===== SHAPE PATH GEOMETRY IMPLEMENTATION =====

ShapePathGeometry::ShapePathGeometry() : ShapeGeometry(PATH) {}

void ShapePathGeometry::generatePath(ofPath& path) const {
    path_.generatePath(path);
}

ofRectangle ShapePathGeometry::getBounds() const {
    return path_.getBounds();
}

bool ShapePathGeometry::loadFromJson(const ofJson& data) {
    return path_.loadFromJson(data);
}

ofJson ShapePathGeometry::toJson() const {
    ofJson json = path_.toJson();
    json["type"] = "path";
    return json;
}

std::unique_ptr<ShapeGeometry> ShapePathGeometry::clone() const {
    auto copy = std::make_unique<ShapePathGeometry>();
    copy->path_ = path_;
    return std::move(copy);
}

// ===== SHAPE PROPERTIES IMPLEMENTATION =====

ShapeProperties::ShapeProperties() {}

bool ShapeProperties::loadFromJson(const ofJson& data) {
    try {
        // Load fill properties
        if (data.contains("fill")) {
            const auto& fillData = data["fill"];
            
            if (fillData.contains("color") && fillData["color"].is_array() && fillData["color"].size() >= 3) {
                const auto& colorArray = fillData["color"];
                fill_.color = ofColor(
                    static_cast<int>(colorArray[0].get<float>() * 255),
                    static_cast<int>(colorArray[1].get<float>() * 255),
                    static_cast<int>(colorArray[2].get<float>() * 255),
                    colorArray.size() > 3 ? static_cast<int>(colorArray[3].get<float>() * 255) : 255
                );
            }
            
            if (fillData.contains("opacity")) {
                fill_.opacity = fillData["opacity"];
            }
            
            if (fillData.contains("enabled")) {
                fill_.enabled = fillData["enabled"];
            }
        }
        
        // Load stroke properties
        if (data.contains("stroke")) {
            const auto& strokeData = data["stroke"];
            
            if (strokeData.contains("color") && strokeData["color"].is_array() && strokeData["color"].size() >= 3) {
                const auto& colorArray = strokeData["color"];
                stroke_.color = ofColor(
                    static_cast<int>(colorArray[0].get<float>() * 255),
                    static_cast<int>(colorArray[1].get<float>() * 255),
                    static_cast<int>(colorArray[2].get<float>() * 255),
                    colorArray.size() > 3 ? static_cast<int>(colorArray[3].get<float>() * 255) : 255
                );
            }
            
            if (strokeData.contains("opacity")) {
                stroke_.opacity = strokeData["opacity"];
            }
            
            if (strokeData.contains("width")) {
                stroke_.width = strokeData["width"];
            }
            
            if (strokeData.contains("enabled")) {
                stroke_.enabled = strokeData["enabled"];
            }
            
            if (strokeData.contains("lineCap")) {
                stroke_.lineCap = static_cast<ShapeStroke::LineCap>(static_cast<int>(strokeData["lineCap"]));
            }
            
            if (strokeData.contains("lineJoin")) {
                stroke_.lineJoin = static_cast<ShapeStroke::LineJoin>(static_cast<int>(strokeData["lineJoin"]));
            }
            
            if (strokeData.contains("miterLimit")) {
                stroke_.miterLimit = strokeData["miterLimit"];
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapeProperties") << "Failed to load from JSON: " << e.what();
        return false;
    }
}

ofJson ShapeProperties::toJson() const {
    ofJson json;
    
    // Export fill
    json["fill"] = {
        {"enabled", fill_.enabled},
        {"color", {fill_.color.r / 255.0f, fill_.color.g / 255.0f, fill_.color.b / 255.0f, fill_.color.a / 255.0f}},
        {"opacity", fill_.opacity}
    };
    
    // Export stroke
    json["stroke"] = {
        {"enabled", stroke_.enabled},
        {"color", {stroke_.color.r / 255.0f, stroke_.color.g / 255.0f, stroke_.color.b / 255.0f, stroke_.color.a / 255.0f}},
        {"opacity", stroke_.opacity},
        {"width", stroke_.width},
        {"lineCap", static_cast<int>(stroke_.lineCap)},
        {"lineJoin", static_cast<int>(stroke_.lineJoin)},
        {"miterLimit", stroke_.miterLimit}
    };
    
    return json;
}

} // namespace ae
} // namespace ofx