#include "ofxAEShapeSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "ofMath.h"
#include "ofxAEShapeUtils.h"
#include "ofxAEVisitorUtils.h"

namespace ofx { namespace ae {

bool ShapeSource::setup(const ofJson &json)
{
	if (!json.contains("shape")) {
		ofLogWarning("ShapeSource") << "No shape data found in JSON";
		return false;
	}

	const auto& shapeJson = json["shape"];

	ofJson keyframes = {};
	if (json.contains("keyframes") && json["keyframes"].contains("shape")) {
		keyframes = json["keyframes"]["shape"];
	}

	shape_props_.setup(shapeJson, keyframes);

	return true;
}

void ShapeSource::update() {
}

bool ShapeSource::setFrame(int frame) {
    return shape_props_.setFrame(frame);
}

bool ShapeSource::tryExtract(ShapeData &dst) const
{
	return shape_props_.tryExtract(dst);
}

void ShapeSource::draw(float x, float y, float w, float h) const {
    try {
        ShapeData data;
        if (!shape_props_.tryExtract(data)) {
            ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in draw(), using defaults";
        }
        utils::PathExtractionVisitor gather;
		data.accept(gather);
		auto paths = gather.getPaths();
		for(auto &&path : paths) {
			float opacity = RenderContext::getCurrentStyle().color.a;
			auto fc = path.getFillColor(); fc.a *= opacity; path.setFillColor(fc);
			auto sc = path.getStrokeColor(); sc.a *= opacity; path.setStrokeColor(sc);
			path.draw();
		}
    }
    catch (const std::exception& e) {
        ofLogError("ShapeSource") << "Error during rendering: " << e.what();
    }
}

float ShapeSource::getWidth() const {
    ShapeData data;
    if (!shape_props_.tryExtract(data)) {
        ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in getWidth(), using defaults";
    }
    
    float maxWidth = 0.0f;
    for (const auto& shapePtr : data.data) {
        if (!shapePtr) continue;
        
        if (auto ellipse = dynamic_cast<const EllipseData*>(shapePtr.get())) {
            maxWidth = std::max(maxWidth, ellipse->size.x);
        }
        else if (auto rectangle = dynamic_cast<const RectangleData*>(shapePtr.get())) {
            maxWidth = std::max(maxWidth, rectangle->size.x);
        }
        else if (auto polygon = dynamic_cast<const PolygonData*>(shapePtr.get())) {
            maxWidth = std::max(maxWidth, polygon->outerRadius * 2.0f);
        }
    }
    
    return maxWidth;
}

float ShapeSource::getHeight() const {
    ShapeData data;
    if (!shape_props_.tryExtract(data)) {
        ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in getHeight(), using defaults";
    }
    
    float maxHeight = 0.0f;
    for (const auto& shapePtr : data.data) {
        if (!shapePtr) continue;
        
        if (auto ellipse = dynamic_cast<const EllipseData*>(shapePtr.get())) {
            maxHeight = std::max(maxHeight, ellipse->size.y);
        }
        else if (auto rectangle = dynamic_cast<const RectangleData*>(shapePtr.get())) {
            maxHeight = std::max(maxHeight, rectangle->size.y);
        }
        else if (auto polygon = dynamic_cast<const PolygonData*>(shapePtr.get())) {
            maxHeight = std::max(maxHeight, polygon->outerRadius * 2.0f);
        }
    }
    
    return maxHeight;
}

void ShapeSource::accept(Visitor& visitor) {
 visitor.visit(*this);
}

}} // namespace ofx::ae
