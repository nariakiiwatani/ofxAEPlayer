#include "ofxAEProperty.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

void PropertyBase::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

void PropertyGroup::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

void PropertyArray::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

}} // namespace ofx::ae
