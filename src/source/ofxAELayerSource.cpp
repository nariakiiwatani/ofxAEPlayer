#include "ofxAELayerSource.h"
#include "ofxAESolidSource.h"
#include "ofxAECompositionSource.h"
#include "ofxAEShapeSource.h"
#include "ofxAEStillSource.h"
#include "ofxAEVideoSource.h"
#include "ofxAESequenceSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"

namespace ofx { namespace ae {

std::unique_ptr<LayerSource> LayerSource::createSourceOfType(SourceType type)
{
	switch(type) {
		case SourceType::SOLID:
			return std::make_unique<SolidSource>();
		case SourceType::COMPOSITION:
			return std::make_unique<CompositionSource>();
		case SourceType::SHAPE:
			return std::make_unique<ShapeSource>();
		
		case SourceType::STILL:
			return std::make_unique<StillSource>();
		case SourceType::VIDEO:
			return std::make_unique<VideoSource>();
		case SourceType::SEQUENCE:
			return std::make_unique<SequenceSource>();
		
		// NULL_OBJECT layers have no visual content - return nullptr
		case SourceType::NULL_OBJECT:
			return nullptr;
		
		// TODO: Implement support for CAMERA, LIGHT, ADJUSTMENT, TEXT source types
		default:
			return nullptr;
	}
}

void LayerSource::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

bool LayerSource::setTime(double time)
{
	return setFrame(util::timeToFrame(time, fps_));
}

double LayerSource::getTime() const
{
	return util::frameToTime(current_frame_, fps_);
}

double LayerSource::getDuration() const
{
	return util::frameToTime(getDurationFrames(), fps_);
}

}} // namespace ofx::ae
