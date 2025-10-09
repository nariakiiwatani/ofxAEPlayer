#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAEKeyframe.h"
#include "ofxAEMarker.h"
#include "ofxAERenderEngine.h"
#include "events/ofxAEEventTypes.h"

// ofxAEPlayer is an alias for ofx::ae::Composition
using ofxAEPlayer = ofx::ae::Composition;

// Import key types into global namespace for convenience
using PlaybackState = ofx::ae::PlaybackState;
using BlendMode = ofx::ae::BlendMode;