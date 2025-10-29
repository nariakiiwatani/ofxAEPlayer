#pragma once

#include "ofGraphics.h"
#include "../utils/ofxAEBlendMode.h"

namespace ofx { namespace ae {

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
