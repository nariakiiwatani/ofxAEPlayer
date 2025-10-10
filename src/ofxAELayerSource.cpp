#include "ofxAELayerSource.h"
#include "ofxAEFootageSource.h"
#include "ofxAECompositionSource.h"
#include "ofxAEShapeSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

// Since LayerSource is a pure virtual interface, this implementation file
// primarily contains utility functions and any non-virtual method implementations.

// The static utility methods are already implemented in the header file,
// but we can add any additional implementation details here if needed.

// Example of how concrete LayerSource implementations might look:

/**
 * Example base implementation for common LayerSource functionality.
 * This is not part of the interface but shows how derived classes might be structured.
 */
namespace {

/**
 * BaseLayerSource provides common functionality for LayerSource implementations.
 * This is an internal helper class that concrete sources can inherit from.
 */
class BaseLayerSource : public LayerSource {
protected:
    SourceType sourceType_;
    float width_;
    float height_;
    bool visible_;
    float lastUpdateTime_;
    
public:
    BaseLayerSource(SourceType type) 
        : sourceType_(type)
        , width_(0.0f)
        , height_(0.0f)
        , visible_(true)
        , lastUpdateTime_(-1.0f) {
    }
    
    virtual ~BaseLayerSource() = default;
    
    // Implement common interface methods
    SourceType getSourceType() const override {
        return sourceType_;
    }
    
    float getWidth() const override {
        return width_;
    }
    
    float getHeight() const override {
        return height_;
    }
    
    ofRectangle getBounds() const override {
        return createBounds(width_, height_);
    }
    
    bool isVisible() const override {
        return visible_;
    }
    
    bool needsUpdate(float currentTime) const override {
        // Default implementation: always update unless time hasn't changed
        return lastUpdateTime_ != currentTime;
    }
    
    void update(float currentTime) override {
        lastUpdateTime_ = currentTime;
        // Derived classes should call this base implementation
    }
    
    std::string getDebugInfo() const override {
        return "BaseLayerSource[" + sourceTypeToString(sourceType_) + 
               ", " + ofToString(width_) + "x" + ofToString(height_) + "]";
    }

protected:
    /**
     * Helper method for derived classes to set dimensions
     */
    void setDimensions(float width, float height) {
        width_ = width;
        height_ = height;
    }
    
    /**
     * Helper method for derived classes to set visibility
     */
    void setVisible(bool visible) {
        visible_ = visible;
    }
    
    /**
     * Helper method to parse common properties from JSON
     */
    bool parseCommonProperties(const ofJson& json) {
        try {
            // Parse width and height if available
            if (json.contains("width") && json["width"].is_number()) {
                width_ = json["width"].get<float>();
            }
            
            if (json.contains("height") && json["height"].is_number()) {
                height_ = json["height"].get<float>();
            }
            
            // Parse visibility if available
            if (json.contains("visible") && json["visible"].is_boolean()) {
                visible_ = json["visible"].get<bool>();
            }
            
            return true;
        } catch (const std::exception& e) {
            ofLogError("BaseLayerSource") << "Error parsing common properties: " << e.what();
            return false;
        }
    }
};

/**
 * Example implementation of a solid color source
 */
class SolidLayerSource : public BaseLayerSource {
private:
    ofColor color_;
    
public:
    SolidLayerSource() : BaseLayerSource(SOLID), color_(128, 128, 128, 255) {}
    
    bool setup(const ofJson& json) override {
        if (!parseCommonProperties(json)) {
            return false;
        }
        
        try {
            // Parse solid color properties
            if (json.contains("color")) {
                const auto& colorJson = json["color"];
                if (colorJson.is_array() && colorJson.size() >= 3) {
                    color_.r = colorJson[0].get<float>() * 255;
                    color_.g = colorJson[1].get<float>() * 255;
                    color_.b = colorJson[2].get<float>() * 255;
                    if (colorJson.size() >= 4) {
                        color_.a = colorJson[3].get<float>() * 255;
                    }
                }
            }
            
            ofLogNotice("SolidLayerSource") << "Setup complete: " << getDebugInfo();
            return true;
        } catch (const std::exception& e) {
            ofLogError("SolidLayerSource") << "Setup failed: " << e.what();
            return false;
        }
    }
    
    void update(float currentTime) override {
        BaseLayerSource::update(currentTime);
        // Solid colors typically don't need time-based updates
    }
    
    void draw(const RenderContext& context) const override {
        if (!isVisible() || !context.isValidSize()) {
            return;
        }
        
        // Apply context opacity to color
        ofColor renderColor = color_;
        renderColor.a *= context.opacity;
        
        // Save current style
        ofPushStyle();
        
        // Apply transformation matrix
        ofPushMatrix();
        ofMultMatrix(context.transform);
        
        // Set color and draw rectangle
        ofSetColor(renderColor);
        ofDrawRectangle(context.x, context.y, context.w, context.h);
        
        // Restore transformation and style
        ofPopMatrix();
        ofPopStyle();
    }
    
    std::string getDebugInfo() const override {
        return "SolidLayerSource[" + ofToString(color_) + ", " + 
               ofToString(getWidth()) + "x" + ofToString(getHeight()) + "]";
    }
};

} // anonymous namespace

}} // namespace ofx::ae
