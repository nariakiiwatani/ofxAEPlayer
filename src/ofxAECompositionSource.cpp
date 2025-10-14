#include "ofxAECompositionSource.h"
#include <sstream>

namespace ofx { namespace ae {

CompositionSource::CompositionSource()
    : LayerSource()
    , composition_(nullptr)
{
}

bool CompositionSource::load(const std::filesystem::path& filepath) {
	composition_ = std::make_shared<Composition>();
	return composition_->load(filepath);
}

void CompositionSource::update() {
    if (!composition_) {
        return;
    }
	composition_->update();
}

void CompositionSource::draw(float x, float y, float w, float h) const {
    if (!composition_) {
        return;
    }
	composition_->draw(x, y, w, h);
}

float CompositionSource::getWidth() const {
    if (!composition_) {
        return 0.0f;
    }
    return composition_->getWidth();
}

float CompositionSource::getHeight() const {
    if (!composition_) {
        return 0.0f;
    }
    return composition_->getHeight();
}

std::string CompositionSource::getDebugInfo() const {
    std::stringstream ss;
    ss << "CompositionSource[";
    if (composition_) {
        const auto& info = composition_->getInfo();
		ss << "name=" << info.layers.size() << "layers";
        ss << ", size=" << getWidth() << "x" << getHeight();
    } else {
        ss << "no composition";
    }
    ss << "]";
    return ss.str();
}

}}
