#include "ofxAEBlendMode.h"
#include "ofGraphics.h"

namespace ofx { namespace ae {

void applyBlendMode(BlendMode mode) {
	switch (mode) {
		case BlendMode::NORMAL:   ofEnableBlendMode(OF_BLENDMODE_ALPHA);   return;
		case BlendMode::ADD:      ofEnableBlendMode(OF_BLENDMODE_ADD);     return;
		case BlendMode::SUBTRACT: ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);return;
		case BlendMode::MULTIPLY: ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);return;
		case BlendMode::SCREEN:   ofEnableBlendMode(OF_BLENDMODE_SCREEN);  return;
		case BlendMode::LIGHTEN:  ofEnableBlendMode(OF_BLENDMODE_MAX);     return;
		case BlendMode::DARKEN:   ofEnableBlendMode(OF_BLENDMODE_MIN);     return;

		default:
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			return;
	}}

}}
