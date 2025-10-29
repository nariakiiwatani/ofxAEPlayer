#include "ofxAESequenceSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool SequenceSource::load(const std::filesystem::path &filepath)
{
	if(!ofDirectory::doesDirectoryExist(filepath)) {
		return false;
	}
	pool_.clear();
	texture_.reset();
	ofDirectory dir;
	dir.open(filepath);
	dir.sort();
	pool_.reserve(dir.size());
	for(auto &&file : dir.getFiles()) {
		ofTexture tex;
		if(ofLoadImage(tex, file.path())) {
			pool_.push_back(tex);
		}
	}
	return !pool_.empty();
}

bool SequenceSource::setFrame(int frame)
{
	if(frame < 0 || pool_.size() <= frame) {
		return false;
	}
	auto *tex = &pool_[frame];
	if(texture_.has_value() && texture_.value() == tex) {
		return false;
	}
	texture_ = tex;
	return true;
}

void SequenceSource::accept(Visitor &visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
