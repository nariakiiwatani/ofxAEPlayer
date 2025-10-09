#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAEKeyframe.h"
#include "ofxAEMarker.h"
#include "ofxAERenderEngine.h"

// ofxAEPlayer is an alias for ofx::ae::Composition
using ofxAEPlayer = ofx::ae::Composition;

// Import key types into global namespace for convenience
using BlendMode = ofx::ae::BlendMode;
