#include "ofxAETimeProperty.h"
#include "../core/ofxAEVisitor.h"

namespace ofx { namespace ae {

void TimePropertyBase::accept(Visitor &visitor) {
	// Base implementation - override in derived classes if needed
}

void TimePropertyGroup::accept(Visitor &visitor) {
	for(auto &&[k,v] : props_) {
		if(v) {
			v->accept(visitor);
		}
	}
}

void TimePropertyArray::accept(Visitor &visitor) {
	for(auto &p : properties_) {
		if(p) {
			p->accept(visitor);
		}
	}
}

}} // namespace ofx::ae