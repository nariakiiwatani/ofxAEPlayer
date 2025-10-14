#include "ofxAELayerSource.h"
#include "ofxAESolidSource.h"
#include "ofxAECompositionSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

std::unique_ptr<LayerSource> LayerSource::createSourceOfType(SourceType type)
{
	switch(type) {
		case SOLID: return std::make_unique<SolidSource>();
		case COMPOSITION: return std::make_unique<CompositionSource>();
		default:
			return nullptr;
//		case SHAPE: return "shape";
//		case CAMERA: return "camera";
//		case LIGHT: return "light";
//		case ADJUSTMENT: return "adjustment";
//		case TEXT: return "text";
//		case NULL_OBJECT: return "null";
//		default: return "unknown";
	}
}

}} // namespace ofx::ae
