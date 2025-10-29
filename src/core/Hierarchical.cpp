#include "Hierarchical.h"

void Hierarchical::setParent(std::shared_ptr<Hierarchical> new_parent)
{
	auto this_p = shared_from_this();
	if(auto p = parent_.lock()) {
		if(this_p == p->child_) {
			p->child_ = sibling_;
		}
		else {
			std::shared_ptr<Hierarchical> c = p->child_;
			while(c) {
				if(c->sibling_ == this_p) {
					c->sibling_ = sibling_;
					break;
				}
				c = c->sibling_;
			}
		}
		sibling_ = nullptr;
	}
	if(new_parent) {
		sibling_ = new_parent->child_;
		new_parent->child_ = this_p;
	}
	parent_ = new_parent;
	dirty(PARENT);
}

void Hierarchical::dirty(unsigned int flag)
{
	dirty_flags_ |= flag;
	if(child_) {
		child_->dirty(PARENT);
	}
	if(sibling_ && flag & PARENT) {
		sibling_->dirty(flag);
	}
}


