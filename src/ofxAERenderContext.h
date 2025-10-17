#pragma once

#include "ofGraphics.h"

namespace ofx { namespace ae {

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

struct RenderContext {
	struct Style {
		ofFloatColor color{1,1,1,1};
		BlendMode blendMode{BlendMode::NORMAL};
	};
	static void push();
	static void pop();
	static void setOpacity(float opacity);
	static void setColorRGB(ofFloatColor color);
	static void mulColorRGBA(ofFloatColor color);
	static void setBlendMode(BlendMode mode);
	static const Style& getCurrentStyle();
};

}}
