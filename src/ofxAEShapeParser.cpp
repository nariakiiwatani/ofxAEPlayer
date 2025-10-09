#include "ofxAEShapeParser.h"
#include "ofLog.h"

namespace ofx {
namespace ae {

// AE property name mappings
const std::map<std::string, std::string> AEShapeParser::AE_PROPERTY_MAP = {
    {"ADBE Vector Shape - Ellipse", "ellipse"},
    {"ADBE Vector Shape - Rect", "rectangle"},
    {"ADBE Vector Shape - Group", "path"},
    {"ADBE Vector Shape - Star", "polygon"},
    {"ADBE Vector Group", "group"},
    {"ADBE Vector Graphic - Fill", "fill"},
    {"ADBE Vector Graphic - Stroke", "stroke"}
};

AEShapeParser::AEShapeParser() {}

std::unique_ptr<ShapeLayer> AEShapeParser::parseShapeLayer(const ofJson& layerData) {
    try {
        auto shapeLayer = std::make_unique<ShapeLayer>();
        
        // Parse basic layer properties would be handled by base Layer class
        
        // Parse shape-specific data
        if (layerData.contains("properties")) {
            auto rootGroup = parseShapeGroup(layerData["properties"]);
            if (rootGroup) {
                shapeLayer->setRootGroup(std::move(rootGroup));
            }
        }
        
        ofLogNotice("AEShapeParser") << "Successfully parsed shape layer";
        return shapeLayer;
        
    } catch (const std::exception& e) {
        logParseError("parseShapeLayer", e.what());
        return nullptr;
    }
}

std::unique_ptr<ShapeGroup> AEShapeParser::parseShapeGroup(const ofJson& groupData) {
    try {
        auto group = std::make_unique<ShapeGroup>();
        
        // Parse group properties
        if (groupData.contains("name")) {
            group->setName(groupData["name"]);
        }
        
        if (groupData.contains("visible")) {
            group->setVisible(parseBool(groupData["visible"], true));
        }
        
        if (groupData.contains("opacity")) {
            group->setOpacity(parseFloat(groupData["opacity"], 1.0f));
        }
        
        // Parse transform
        if (groupData.contains("transform")) {
            parseTransformData(groupData["transform"], *group);
        }
        
        // Parse child properties/items
        if (groupData.contains("properties") && groupData["properties"].is_array()) {
            for (const auto& itemData : groupData["properties"]) {
                if (itemData.contains("type")) {
                    std::string type = itemData["type"];
                    
                    if (type == "group") {
                        auto childGroup = parseShapeGroup(itemData);
                        if (childGroup) {
                            group->addItem(std::move(childGroup));
                        }
                    } else {
                        // Parse geometry item
                        auto geometry = parseShapeGeometry(itemData);
                        if (geometry) {
                            auto geometryItem = std::make_unique<ShapeGeometryItem>();
                            geometryItem->setGeometry(std::move(geometry));
                            
                            // Parse properties if available
                            if (itemData.contains("properties")) {
                                ShapeProperties properties;
                                if (parseShapeProperties(itemData["properties"], properties)) {
                                    geometryItem->setProperties(properties);
                                }
                            }
                            
                            group->addItem(std::move(geometryItem));
                        }
                    }
                }
            }
        }
        
        return group;
        
    } catch (const std::exception& e) {
        logParseError("parseShapeGroup", e.what());
        return nullptr;
    }
}

std::unique_ptr<ShapeGeometry> AEShapeParser::parseShapeGeometry(const ofJson& shapeData) {
    try {
        if (!shapeData.contains("type")) {
            logParseError("parseShapeGeometry", "Missing type field");
            return nullptr;
        }
        
        std::string type = shapeData["type"];
        std::unique_ptr<ShapeGeometry> geometry;
        
        if (type == "ellipse") {
            auto ellipse = std::make_unique<ShapeEllipse>();
            
            if (shapeData.contains("size")) {
                ellipse->setSize(parseVector2(shapeData["size"], glm::vec2(100, 100)));
            }
            
            if (shapeData.contains("position")) {
                ellipse->setPosition(parseVector2(shapeData["position"], glm::vec2(0, 0)));
            }
            
            geometry = std::move(ellipse);
            
        } else if (type == "rectangle") {
            auto rectangle = std::make_unique<ShapeRectangle>();
            
            if (shapeData.contains("size")) {
                rectangle->setSize(parseVector2(shapeData["size"], glm::vec2(100, 100)));
            }
            
            if (shapeData.contains("position")) {
                rectangle->setPosition(parseVector2(shapeData["position"], glm::vec2(0, 0)));
            }
            
            if (shapeData.contains("roundness")) {
                rectangle->setRoundness(parseFloat(shapeData["roundness"], 0.0f));
            }
            
            geometry = std::move(rectangle);
            
        } else if (type == "path") {
            auto pathGeometry = std::make_unique<ShapePathGeometry>();
            
            if (shapeData.contains("path")) {
                ShapePath path;
                if (parsePathData(shapeData["path"], path)) {
                    pathGeometry->setPath(path);
                }
            }
            
            geometry = std::move(pathGeometry);
            
        } else {
            logParseWarning("parseShapeGeometry", "Unknown geometry type: " + type);
            return nullptr;
        }
        
        return geometry;
        
    } catch (const std::exception& e) {
        logParseError("parseShapeGeometry", e.what());
        return nullptr;
    }
}

bool AEShapeParser::parseShapeProperties(const ofJson& data, ShapeProperties& properties) {
    try {
        // Parse fill properties
        if (data.contains("fill")) {
            const auto& fillData = data["fill"];
            ShapeFill fill;
            
            fill.enabled = parseBool(fillData["enabled"], true);
            fill.color = parseColor(fillData["color"], ofColor::white);
            fill.opacity = parseFloat(fillData["opacity"], 1.0f);
            
            properties.setFill(fill);
        }
        
        // Parse stroke properties
        if (data.contains("stroke")) {
            const auto& strokeData = data["stroke"];
            ShapeStroke stroke;
            
            stroke.enabled = parseBool(strokeData["enabled"], false);
            stroke.color = parseColor(strokeData["color"], ofColor::black);
            stroke.opacity = parseFloat(strokeData["opacity"], 1.0f);
            stroke.width = parseFloat(strokeData["width"], 1.0f);
            
            if (strokeData.contains("lineCap")) {
                int lineCap = static_cast<int>(parseFloat(strokeData["lineCap"], 0));
                stroke.lineCap = static_cast<ShapeStroke::LineCap>(lineCap);
            }
            
            if (strokeData.contains("lineJoin")) {
                int lineJoin = static_cast<int>(parseFloat(strokeData["lineJoin"], 0));
                stroke.lineJoin = static_cast<ShapeStroke::LineJoin>(lineJoin);
            }
            
            stroke.miterLimit = parseFloat(strokeData["miterLimit"], 4.0f);
            
            properties.setStroke(stroke);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logParseError("parseShapeProperties", e.what());
        return false;
    }
}

glm::vec2 AEShapeParser::parseVector2(const ofJson& data, const glm::vec2& defaultValue) {
    try {
        if (data.is_array() && data.size() >= 2) {
            return glm::vec2(data[0], data[1]);
        }
    } catch (const std::exception& e) {
        logParseWarning("parseVector2", "Failed to parse vector2: " + std::string(e.what()));
    }
    return defaultValue;
}

ofColor AEShapeParser::parseColor(const ofJson& data, const ofColor& defaultColor) {
    try {
        if (data.is_array() && data.size() >= 3) {
            float r = data[0];
            float g = data[1];
            float b = data[2];
            float a = data.size() > 3 ? static_cast<float>(data[3]) : 1.0f;
            
            return ofColor(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                static_cast<int>(a * 255)
            );
        }
    } catch (const std::exception& e) {
        logParseWarning("parseColor", "Failed to parse color: " + std::string(e.what()));
    }
    return defaultColor;
}

float AEShapeParser::parseFloat(const ofJson& data, float defaultValue) {
    try {
        if (data.is_number()) {
            return data;
        }
    } catch (const std::exception& e) {
        logParseWarning("parseFloat", "Failed to parse float: " + std::string(e.what()));
    }
    return defaultValue;
}

bool AEShapeParser::parseBool(const ofJson& data, bool defaultValue) {
    try {
        if (data.is_boolean()) {
            return data;
        }
    } catch (const std::exception& e) {
        logParseWarning("parseBool", "Failed to parse bool: " + std::string(e.what()));
    }
    return defaultValue;
}

bool AEShapeParser::parsePathData(const ofJson& pathData, ShapePath& path) {
    try {
        path.clear();
        
        if (!pathData.contains("vertices")) {
            return false;
        }
        
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
            
            path.addVertex(vertex);
        }
        
        if (pathData.contains("closed")) {
            path.setClosed(parseBool(pathData["closed"], false));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logParseError("parsePathData", e.what());
        return false;
    }
}

bool AEShapeParser::parseTransformData(const ofJson& transformData, ShapeGroup& group) {
    try {
        if (transformData.contains("position")) {
            glm::vec2 pos = parseVector2(transformData["position"]);
            group.setTranslation(pos.x, pos.y, 0);
        }
        
        if (transformData.contains("scale")) {
            glm::vec2 scale = parseVector2(transformData["scale"], glm::vec2(1, 1));
            group.setScale(scale.x, scale.y, 1);
        }
        
        if (transformData.contains("rotation")) {
            float rotation = parseFloat(transformData["rotation"], 0);
            group.setRotationZ(rotation);
        }
        
        if (transformData.contains("anchor")) {
            glm::vec2 anchor = parseVector2(transformData["anchor"]);
            group.setAnchorPoint(anchor.x, anchor.y, 0);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logParseError("parseTransformData", e.what());
        return false;
    }
}

bool AEShapeParser::parseAnimatedProperty(const ofJson& propData, const std::string& propertyName) {
    // Placeholder for animated property parsing
    // Will be implemented in Phase 3 (Animation integration)
    logParseWarning("parseAnimatedProperty", "Animated properties not yet implemented for: " + propertyName);
    return false;
}

void AEShapeParser::logParseError(const std::string& context, const std::string& error) {
    ofLogError("AEShapeParser") << context << ": " << error;
}

void AEShapeParser::logParseWarning(const std::string& context, const std::string& warning) {
    ofLogWarning("AEShapeParser") << context << ": " << warning;
}

} // namespace ae
} // namespace ofx