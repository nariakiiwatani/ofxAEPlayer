#include "ofxAELayerSource.h"
#include "ofxAESolidSource.h"
#include "ofxAECompositionSource.h"
#include "ofxAEShapeSource.h"
#include "ofxAEStillSource.h"
#include "ofxAEVideoSource.h"
#include "ofxAESequenceSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

std::unique_ptr<LayerSource> LayerSource::createSourceOfType(SourceType type)
{
	switch(type) {
		case SOLID: return std::make_unique<SolidSource>();
		case COMPOSITION: return std::make_unique<CompositionSource>();
		case SHAPE: return std::make_unique<ShapeSource>();
		
		// New media source types
		case STILL: return std::make_unique<StillSource>();
		case VIDEO: return std::make_unique<VideoSource>();
		case SEQUENCE: return std::make_unique<SequenceSource>();
		
		default:
			return nullptr;
//		case CAMERA: return "camera";
//		case LIGHT: return "light";
//		case ADJUSTMENT: return "adjustment";
//		case TEXT: return "text";
//		case NULL_OBJECT: return "null";
//		default: return "unknown";
	}
}

}} // namespace ofx::ae
