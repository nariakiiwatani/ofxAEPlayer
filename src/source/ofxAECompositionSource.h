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
	
	// Time API
	bool setTime(double time) override;
	double getTime() const override;
	double getDuration() const override;
	
	void update() override;
	void draw(float x, float y, float w, float h) const override;

	SourceType getSourceType() const override { return SourceType::COMPOSITION; }
	float getWidth() const override;
	float getHeight() const override;
	std::string getDebugInfo() const override;
	
	void setComposition(std::shared_ptr<Composition> comp) { composition_ = comp; }
	std::shared_ptr<Composition> getComposition() { return composition_; }
	
private:
	std::shared_ptr<Composition> composition_;
	std::filesystem::path filepath_;
};

}}
