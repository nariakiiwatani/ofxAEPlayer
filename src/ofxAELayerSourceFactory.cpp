#include "ofxAELayerSourceFactory.h"
#include "ofxAEShapeSource.h"
#include "ofxAECompositionSource.h"

namespace ofx { namespace ae {

std::unique_ptr<LayerSource> LayerSourceFactory::createSourceOfType(std::string type)
{
	if(type == "shape") return std::make_unique<ShapeSource>();
	if(type == "composition") return std::make_unique<CompositionSource>();
	return nullptr;
}

}} // namespace ofx::ae
