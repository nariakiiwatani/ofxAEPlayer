#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEComposition.h"

namespace ofx { namespace ae {

class Visitor;

class CompositionSource : public LayerSource
{
public:
    CompositionSource();
    
	void accept(Visitor& visitor) override;
	bool load(const std::filesystem::path &filepath) override;
    void update() override;
    void draw(float x, float y, float w, float h) const override;

    SourceType getSourceType() const override { return COMPOSITION; }
    float getWidth() const override;
    float getHeight() const override;
    std::string getDebugInfo() const override;
    
private:
    std::shared_ptr<Composition> composition_;
};

}}
