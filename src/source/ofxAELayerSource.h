#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "ofRectangle.h"
#include "ofxAERenderContext.h"
#include "../data/Enums.h"
#include "../utils/ofxAETimeUtils.h"
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
	
	virtual bool setFrame(Frame frame) = 0;
	virtual Frame getFrame() const { return current_frame_; }

	virtual bool setTime(double time);
	virtual double getTime() const;
	
	virtual FrameCount getDurationFrames() const { return 0.0f; }
	virtual double getDuration() const;
	
	virtual void setFps(float fps) { fps_ = fps; }

	using ofBaseDraws::draw;
	virtual void draw(float x, float y, float w, float h) const override {}

	virtual ofRectangle getBoundingBox() const { return ofRectangle{0,0,getWidth(),getHeight()}; }

	virtual SourceType getSourceType() const = 0;

	virtual std::string getDebugInfo() const { return "LayerSource"; }

 static std::unique_ptr<LayerSource> createSourceOfType(SourceType type);
 static std::unique_ptr<LayerSource> createSourceOfType(std::string type) {
	 return createSourceOfType(sourceTypeFromString(type));
 }

protected:
	Frame current_frame_ = 0.0f;
	float fps_ = 30.0f;
};
}} // namespace ofx::ae
