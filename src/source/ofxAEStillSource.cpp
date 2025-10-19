#include "ofxAEStillSource.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

void StillSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
