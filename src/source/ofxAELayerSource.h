#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "ofRectangle.h"
#include "ofxAERenderContext.h"
#include "../data/Enums.h"
#include <string>
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class LayerSource : public ofBaseDraws, public ofBaseUpdates
{
public:
	virtual ~LayerSource() = default;

	virtual void accept(Visitor &visitor);

	virtual bool setup(const ofJson &json) { return false; }
	virtual bool load(const std::filesystem::path &filepath) { return setup(ofLoadJson(filepath)); }

	virtual void update() override {}
	
	virtual bool setTime(double time) { return false; }
	virtual double getTime() const { return 0.0; }
	virtual double getDuration() const { return 0.0; }

	using ofBaseDraws::draw;
	virtual void draw(float x, float y, float w, float h) const override {}

	virtual ofRectangle getBoundingBox() const { return ofRectangle{0,0,getWidth(),getHeight()}; }

	virtual SourceType getSourceType() const = 0;

	virtual std::string getDebugInfo() const { return "LayerSource"; }

 static std::unique_ptr<LayerSource> createSourceOfType(SourceType type);
 static std::unique_ptr<LayerSource> createSourceOfType(std::string type) {
	 return createSourceOfType(sourceTypeFromString(type));
 }

};
}} // namespace ofx::ae
