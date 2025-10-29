#include "ofxAEShapeSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "ofMath.h"
#include "ofxAEShapeUtils.h"
#include "ofxAERenderContext.h"
#include "ofxAEBlendMode.h"
#include "ofxAEVisitorUtils.h"

namespace ofx { namespace ae {

bool ShapeSource::setup(const ofJson &json)
{
	visitor_ = std::make_shared<PathExtractionVisitor>();
	if(!json.contains("shape")) {
		ofLogWarning("ShapeSource") << "No shape data found in JSON";
		return false;
	}

	const auto &shapeJson = json["shape"];

	ofJson keyframes = {};
	if(json.contains("keyframes") && json["keyframes"].contains("shape")) {
		keyframes = json["keyframes"]["shape"];
	}

	shape_props_.setup(shapeJson, keyframes);

	return true;
}

void ShapeSource::update()
{
	if(shape_props_.tryExtract(shape_data_)) {
		visitor_ = std::make_shared<PathExtractionVisitor>();
		visitor_->visit(shape_data_);
	}
}

bool ShapeSource::setFrame(int frame)
{
    return shape_props_.setFrame(frame);
}

bool ShapeSource::tryExtract(ShapeData &dst) const
{
	return shape_props_.tryExtract(dst);
}

void ShapeSource::draw(float x, float y, float w, float h) const
{
	auto bb = visitor_->getBoundingBox();
	ofPushMatrix();
	ofTranslate(x,y);
	ofScale(w/bb.width, h/bb.height);
	visitor_->getRenderer().draw();
	ofPopMatrix();
}

float ShapeSource::getWidth() const
{
	return visitor_->getBoundingBox().width;
}

float ShapeSource::getHeight() const
{
	return visitor_->getBoundingBox().height;
}

ofRectangle ShapeSource::getBoundingBox() const
{
	return visitor_->getBoundingBox();
}


void ShapeSource::accept(Visitor &visitor)
{
	visitor.visit(*this);
}

}} // namespace ofx::ae
