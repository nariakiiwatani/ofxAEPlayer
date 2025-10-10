#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "ofRectangle.h"
#include "ofxAERenderContext.h"
#include <string>
#include <memory>

namespace ofx { namespace ae {

/**
 * LayerSource is the base interface for all After Effects layer source types.
 * This abstraction allows different source implementations (footage, shapes, 
 * compositions, etc.) to be handled uniformly by the rendering engine.
 * 
 * The design follows openFrameworks conventions and provides a foundation
 * for extensible layer source implementations.
 */
class LayerSource {
public:
    /**
     * Enumeration of supported After Effects layer source types.
     * These correspond to the various source types that can be used
     * in After Effects compositions.
     */
    enum SourceType {
        FOOTAGE,      // Static images and video files
        SHAPE,        // Vector shapes and paths
        COMPOSITION,  // Nested compositions (pre-comps)
        SOLID,        // Solid color layers
        CAMERA,       // 3D camera layers
        LIGHT,        // 3D light layers
        ADJUSTMENT,   // Adjustment layers (effects only)
        TEXT,         // Text layers with typography
        NULL_OBJECT   // Null objects for hierarchy/parenting
    };

    /**
     * Virtual destructor ensures proper cleanup of derived classes
     */
    virtual ~LayerSource() = default;
    
    // ========================================================================
    // Core Lifecycle Methods
    // ========================================================================
    
	virtual bool setup(const ofJson &json) { return false; }
	virtual bool load(const std::filesystem::path &filepath) { return setup(ofLoadJson(filepath)); }

    /**
     * Update the layer source for the current frame/time.
     * This is called once per frame and should handle any time-dependent
     * updates such as animated properties or video frame advancement.
     * 
     * @param currentTime Current composition time in seconds
     */
    virtual void update(float currentTime) = 0;
    
    /**
     * Render the layer source within the provided context.
     * This method should render the source content using the transformation,
     * opacity, and other parameters specified in the RenderContext.
     * 
     * @param context Rendering context with transform, opacity, blend mode, etc.
     */
    virtual void draw(const RenderContext& context) const = 0;
    
    // ========================================================================
    // Essential Properties
    // ========================================================================
    
    /**
     * Get the source type of this layer source.
     * @return The SourceType enum value
     */
    virtual SourceType getSourceType() const = 0;
    
    /**
     * Get the native width of the source content.
     * For footage, this is the image/video width.
     * For compositions, this is the composition width.
     * @return Width in pixels
     */
    virtual float getWidth() const = 0;
    
    /**
     * Get the native height of the source content.
     * For footage, this is the image/video height.
     * For compositions, this is the composition height.
     * @return Height in pixels
     */
    virtual float getHeight() const = 0;
    
    /**
     * Get the bounding rectangle of the source content.
     * This should return the native bounds of the content, which may
     * be transformed during rendering.
     * @return Rectangle representing content bounds
     */
    virtual ofRectangle getBounds() const = 0;
    
    // ========================================================================
    // Rendering Control (with default implementations)
    // ========================================================================
    
    /**
     * Check if the source should be rendered.
     * This can be overridden to implement layer-specific visibility logic.
     * @return true if the source should be rendered
     */
    virtual bool isVisible() const { return true; }
    
    /**
     * Check if the source needs to be updated for the given time.
     * This can be overridden to optimize updates for static content.
     * @param currentTime Current composition time in seconds
     * @return true if update() should be called
     */
    virtual bool needsUpdate(float currentTime) const { return true; }
    
    /**
     * Check if this source can be cached.
     * Static sources may return true to enable render caching optimizations.
     * @return true if the source supports caching
     */
    virtual bool canCache() const { return false; }
    
    // ========================================================================
    // Resource Management (with default implementations)
    // ========================================================================
    
    /**
     * Called when the current frame changes.
     * This provides an opportunity for sources to perform frame-specific
     * operations, such as loading video frames or updating animations.
     * @param frame Current frame number
     */
    virtual void onFrameChanged(int frame) {}
    
    /**
     * Clean up resources used by the source.
     * This should release any allocated resources such as textures,
     * file handles, or memory buffers.
     */
    virtual void cleanup() {}
    
    // ========================================================================
    // Debug and Diagnostics (with default implementation)
    // ========================================================================
    
    /**
     * Get debug information about this source.
     * This is useful for logging and troubleshooting rendering issues.
     * @return String containing debug information
     */
    virtual std::string getDebugInfo() const { return "LayerSource"; }
    
    // ========================================================================
    // Utility Methods
    // ========================================================================
    
    /**
     * Convert SourceType enum to string for debugging/logging
     * @param type Source type to convert
     * @return String representation of the source type
     */
    static std::string sourceTypeToString(SourceType type) {
        switch (type) {
            case FOOTAGE: return "FOOTAGE";
            case SHAPE: return "SHAPE";
            case COMPOSITION: return "COMPOSITION";
            case SOLID: return "SOLID";
            case CAMERA: return "CAMERA";
            case LIGHT: return "LIGHT";
            case ADJUSTMENT: return "ADJUSTMENT";
            case TEXT: return "TEXT";
            case NULL_OBJECT: return "NULL_OBJECT";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * Create a default bounding rectangle based on width and height
     * @param width Source width
     * @param height Source height
     * @return Rectangle with origin at (0,0)
     */
    static ofRectangle createBounds(float width, float height) {
        return ofRectangle(0, 0, width, height);
    }
};
}} // namespace ofx::ae
