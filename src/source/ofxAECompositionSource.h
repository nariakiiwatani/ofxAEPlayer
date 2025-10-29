#pragma once

#include "ofxAELayerSource.h"
#include "../core/ofxAEComposition.h"
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class CompositionSource : public LayerSource
{
public:
	CompositionSource();
	
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	bool setFrame(int frame) override;
	void update() override;
	void draw(float x, float y, float w, float h) const override;

	SourceType getSourceType() const override { return SourceType::COMPOSITION; }
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override;
	
private:
	std::shared_ptr<Composition> composition_;
	std::filesystem::path filepath_;
};

}}
