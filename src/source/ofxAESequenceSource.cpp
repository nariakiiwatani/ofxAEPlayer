#include "ofxAESequenceSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "../utils/ofxAETimeUtils.h"
#include <algorithm>

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

bool SequenceSource::setTime(double time)
{
	if(util::isNearTime(current_time_, time)) {
		return false;
	}
	
	// Calculate frame index from time
	int new_index = static_cast<int>(time * fps_);
	new_index = std::clamp(new_index, 0, static_cast<int>(pool_.size()) - 1);
	
	bool changed = (new_index != current_index_);
	current_index_ = new_index;
	current_time_ = time;
	
	// Update texture pointer if index is valid
	if(new_index >= 0 && static_cast<size_t>(new_index) < pool_.size()) {
		texture_ = &pool_[new_index];
	}
	else {
		texture_.reset();
	}
	
	return changed;
}

double SequenceSource::getDuration() const
{
	if(pool_.empty() || fps_ <= 0.0) {
		return 0.0;
	}
	return static_cast<double>(pool_.size()) / fps_;
}

void SequenceSource::accept(Visitor &visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
