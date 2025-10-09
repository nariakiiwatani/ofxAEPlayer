#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "ofxAEShapeLayer.h"
#include <string>
#include <memory>

namespace ofx {
namespace ae {

// After Effects shape data parser
class AEShapeParser {
public:
    AEShapeParser();
    ~AEShapeParser() = default;
    
    // Parse shape data from After Effects JSON export
    static std::unique_ptr<ShapeLayer> parseShapeLayer(const ofJson& layerData);
    static std::unique_ptr<ShapeGroup> parseShapeGroup(const ofJson& groupData);
    static std::unique_ptr<ShapeGeometry> parseShapeGeometry(const ofJson& shapeData);
    static bool parseShapeProperties(const ofJson& data, ShapeProperties& properties);
    
    // Utility functions for AE data conversion
    static glm::vec2 parseVector2(const ofJson& data, const glm::vec2& defaultValue = glm::vec2(0, 0));
    static ofColor parseColor(const ofJson& data, const ofColor& defaultColor = ofColor::white);
    static float parseFloat(const ofJson& data, float defaultValue = 0.0f);
    static bool parseBool(const ofJson& data, bool defaultValue = false);
    
    // Path parsing from AE bezier data
    static bool parsePathData(const ofJson& pathData, ShapePath& path);
    
    // Transform parsing
    static bool parseTransformData(const ofJson& transformData, ShapeGroup& group);
    
    // Property animation parsing (for future keyframe support)
    static bool parseAnimatedProperty(const ofJson& propData, const std::string& propertyName);
    
private:
    // Error handling
    static void logParseError(const std::string& context, const std::string& error);
    static void logParseWarning(const std::string& context, const std::string& warning);
    
    // AE specific property name mappings
    static const std::map<std::string, std::string> AE_PROPERTY_MAP;
};

} // namespace ae
} // namespace ofx