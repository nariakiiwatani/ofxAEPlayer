#include "ofxAEBlendMode.h"
#include "ofGraphics.h"

namespace ofx { namespace ae {

void applyBlendMode(BlendMode mode) {
	switch (mode) {
		case BlendMode::NORMAL:   ofEnableBlendMode(OF_BLENDMODE_ALPHA);   return;
		case BlendMode::ADD:      ofEnableBlendMode(OF_BLENDMODE_ADD);     return;
		case BlendMode::SUBTRACT: ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);return;
		case BlendMode::MULTIPLY:
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			return;
		case BlendMode::SCREEN:   ofEnableBlendMode(OF_BLENDMODE_SCREEN);  return;
		case BlendMode::LIGHTEN:  ofEnableBlendMode(OF_BLENDMODE_MAX);     return;
		case BlendMode::DARKEN:   ofEnableBlendMode(OF_BLENDMODE_MIN);     return;

		// TODO: Add support for additional blend modes beyond basic set
		default:
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			return;
	}}

}}
