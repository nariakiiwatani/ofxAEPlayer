#include "ofxAESolidSource.h"
#include "ofxAEVisitor.h"
#include "../utils/ofxAETimeUtils.h"

namespace ofx { namespace ae {

void SolidSource::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

bool SolidSource::setFrame(Frame frame)
{
	if(util::isNearFrame(current_frame_, frame)) {
		return false;
	}
	
	current_frame_ = frame;
	return false;  // Solid color never changes
}

}} // namespace ofx::ae
