#include "ofxAEShapeSource.h"
#include "ofLog.h"

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

	// RenderGroupシステムを初期化
	renderProcessor_ = std::make_unique<RenderGroupProcessor>();
	renderer_ = std::make_unique<RenderItemRenderer>();

	return true;
}

void ShapeSource::update() {
}

bool ShapeSource::setFrame(int frame) {
    return shape_props_.setFrame(frame);
}

void ShapeSource::draw(float x, float y, float w, float h) const {
    RenderContext::push();
    
    try {
        // ShapeDataを取得
        ShapeData data;
        shape_props_.extract(data);
        
        // RenderGroupProcessorでcompositeOrder順にRenderItemsを生成
        auto renderItems = renderProcessor_->processRenderOrder(data, x, y, w, h);
        
        // RenderItemRendererで描画実行
        renderer_->render(renderItems);
    }
    catch (const std::exception& e) {
        ofLogError("ShapeSource") << "Error during rendering: " << e.what();
    }
    
    RenderContext::pop();
}

float ShapeSource::getWidth() const {
    // ShapeDataからサイズを計算
    ShapeData data;
    shape_props_.extract(data);
    
    float maxWidth = 0.0f;
    for (const auto& shapePtr : data) {
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
    // ShapeDataからサイズを計算
    ShapeData data;
    shape_props_.extract(data);
    
    float maxHeight = 0.0f;
    for (const auto& shapePtr : data) {
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

}} // namespace ofx::ae
