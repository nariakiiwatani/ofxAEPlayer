#include "ofxAESolidSource.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

void SolidSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
