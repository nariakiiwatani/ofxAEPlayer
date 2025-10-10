#pragma once

// Main header file for ofxAEPlayer addon
// Includes all necessary components for After Effects composition playback

#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAELayerFactory.h"
#include "ofxAERenderEngine.h"
#include "ofxAEShapeLayer.h"
#include "ofxAEKeyframe.h"
#include "ofxAEMarker.h"
#include "ofxAEMask.h"

// Convenient alias for the main composition class
// This allows demo code to use "ofxAEPlayer" while actually using the Composition class
namespace ofx {
namespace ae {
    // Main player class - alias for Composition for backward compatibility
    using ofxAEPlayer = Composition;
}
}

// Global namespace alias for convenience 
using ofxAEPlayer = ofx::ae::Composition;
