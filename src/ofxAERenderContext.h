#pragma once

#include "ofMain.h"

namespace ofx { namespace ae {

// Forward declaration
class RenderEngine;

/**
 * After Effects compatible blend modes
 * Basic implementation for essential blending operations
 */
enum class BlendMode {
    NORMAL = 0,
    ADD = 1,
    MULTIPLY = 2,
    SCREEN = 3,
    SUBTRACT = 4,
    OVERLAY = 5,
    SOFT_LIGHT = 6,
    HARD_LIGHT = 7,
    COLOR_DODGE = 8,
    COLOR_BURN = 9,
    DARKEN = 10,
    LIGHTEN = 11,
    DIFFERENCE = 12,
    EXCLUSION = 13
};

/**
 * RenderContext encapsulates all rendering parameters for a LayerSource.
 * This provides a clean interface for passing rendering state and enables
 * future extensions without breaking the LayerSource interface.
 */
struct RenderContext {
    float x, y, w, h;                    // Rendering viewport dimensions
    float currentTime;                   // Current composition time in seconds
    float opacity;                       // Layer opacity (0.0-1.0)
    const ofMatrix4x4& transform;        // Combined transformation matrix
    BlendMode blendMode;                 // After Effects compatible blend mode
    
    /**
     * Constructor with all required parameters
     * @param x Left coordinate of render area
     * @param y Top coordinate of render area  
     * @param w Width of render area
     * @param h Height of render area
     * @param time Current time in seconds
     * @param op Opacity value (0.0-1.0)
     * @param trans Reference to transformation matrix
     * @param blend Blend mode for compositing
     */
    RenderContext(float x, float y, float w, float h, float time, 
                  float op, const ofMatrix4x4& trans, BlendMode blend)
        : x(x), y(y), w(w), h(h)
        , currentTime(time)
        , opacity(op)
        , transform(trans)
        , blendMode(blend) {}
    
    /**
     * Copy constructor
     */
    RenderContext(const RenderContext& other)
        : x(other.x), y(other.y), w(other.w), h(other.h)
        , currentTime(other.currentTime)
        , opacity(other.opacity)
        , transform(other.transform)
        , blendMode(other.blendMode) {}
    
    /**
     * Get the aspect ratio of the render area
     */
    float getAspectRatio() const {
        return (h != 0.0f) ? (w / h) : 1.0f;
    }
    
    /**
     * Check if the render area has valid dimensions
     */
    bool isValidSize() const {
        return w > 0.0f && h > 0.0f;
    }
    
    /**
     * Get center point of render area
     */
    ofVec2f getCenter() const {
        return ofVec2f(x + w * 0.5f, y + h * 0.5f);
    }
};

}}