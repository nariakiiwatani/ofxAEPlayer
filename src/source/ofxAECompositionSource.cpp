#include "ofxAECompositionSource.h"
#include "ofxAEVisitor.h"
#include "../utils/ofxAEAssetManager.h"
#include "ofLog.h"
#include <sstream>

namespace ofx { namespace ae {

CompositionSource::CompositionSource()
    : LayerSource()
    , composition_(nullptr)
{
}

bool CompositionSource::load(const std::filesystem::path& filepath) {
	filepath_ = filepath;
	composition_ = AssetManager::getInstance().getComposition(filepath);
	
	if (composition_) {
		ofLogVerbose("CompositionSource") << "Loaded composition via AssetManager: " << filepath;
		return true;
	} else {
		ofLogError("CompositionSource") << "Failed to load composition: " << filepath;
		return false;
	}
}

bool CompositionSource::setFrame(int frame)
{
	std::cout << frame << std::endl;
	return composition_ && composition_->setFrame(frame);
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
		ss << filepath_.filename().string() << ", " << info.layers.size() << " layers";
        ss << ", size=" << getWidth() << "x" << getHeight();
    } else {
        ss << "no composition";
    }
    ss << "]";
    return ss.str();
}

void CompositionSource::accept(Visitor& visitor) {
	visitor.visit(*this);
}

}}
