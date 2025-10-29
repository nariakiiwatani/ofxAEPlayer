#pragma once

#include "stddef.h"
#include <memory>

class Hierarchical : public std::enable_shared_from_this<Hierarchical>
{
public:
	Hierarchical():parent_(),child_(nullptr),sibling_(nullptr),dirty_flags_(NONE){}
	
	void setParent(std::shared_ptr<Hierarchical> p);
	std::shared_ptr<Hierarchical> getParent() { return parent_.lock(); }
	std::shared_ptr<Hierarchical> getFirstChild() { return child_; }
	std::shared_ptr<Hierarchical> getSibling() { return sibling_; }

	bool isDirty(unsigned int chk=0xFFFFFFFF) const{return (dirty_flags_&chk)!=0;}

protected:
	std::weak_ptr<Hierarchical> parent_;
	std::shared_ptr<Hierarchical> child_;
	std::shared_ptr<Hierarchical> sibling_;
	enum {
		NONE	= 0x00000000,
		LOCAL	= 0x00000001,
		PARENT	= 0x00000002,
	};
	void dirty(unsigned int flag);
	void clsDirtyFlag(unsigned int flag){dirty_flags_&=~flag;}
private:
	unsigned int dirty_flags_;
};

