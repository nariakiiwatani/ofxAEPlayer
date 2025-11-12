#include "ofxAEProperty.h"
#include "../core/ofxAEVisitor.h"

namespace ofx { namespace ae {

void PropertyBase::accept(Visitor &visitor) {
}

void PropertyGroup::accept(Visitor &visitor) {
	for(auto &&[k,v] : props_) {
		if(v) {
			v->accept(visitor);
		}
	}
}

void PropertyArray::accept(Visitor &visitor) {
	for(auto &p : properties_) {
		if(p) {
			p->accept(visitor);
		}
	}
}

}} // namespace ofx::ae
