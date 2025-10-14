#pragma once

#include "ofGraphics.h"

namespace ofx { namespace ae {

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
	ofFloatColor color{1,1,1,1};
	BlendMode blendMode{BlendMode::NORMAL};
	static void push();
	static void pop();
	static void setOpacity(float opacity);
	static void setColorRGB(ofFloatColor color);
	static void setBlendMode(BlendMode mode);
};

}}
