#include "ofxAEVideoSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"

namespace ofx { namespace ae {

bool VideoSource::load(const std::filesystem::path &filepath)
{
	return player_.load(filepath);
}

bool VideoSource::setFrame(int frame)
{
	player_.setFrame(frame);
	player_.update();
	return player_.isFrameNew();
}

void VideoSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
