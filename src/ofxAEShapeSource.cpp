#include "ofxAEShapeSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

namespace renderer {
struct Context {
	Context() { push(); }
	void visit(const ShapeDataBase &data) {
		// pseudo code
		/*
		 switch(data.type):
			case if PathData: appendPath();
			case if Group: push(); visitChildren(data); pop();
			case if RenderData:
				var fn = draw current path in data style
				if composite order is 1: prepend fn to current command
				if else: append fn to current command
		 */
	}
	void render() const { for(auto &&fn : work.top().command) { fn(); } }
private:
	struct Work {
		ofPath path;
		std::deque<std::function<void()>> command;
		void append(Work &w) {
			path.append(w.path);
			command.insert(command.begin(), w.command.begin(), w.command.end());
		}
	};
	std::stack<Work> work;

	void appendPath(const ofPath &p) { work.top().path.append(p); }
	void push() { work.push(Work()); }
	void pop() { auto &&w = work.top(); work.pop(); work.top().append(w); }
};
}

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
        if (!shape_props_.tryExtract(data)) {
            // Log warning and use default values
            ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in draw(), using defaults";
            data = ShapeDataHelper::getDefault();
        }
        
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
    if (!shape_props_.tryExtract(data)) {
        // Log warning and use default values
        ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in getWidth(), using defaults";
        data = ShapeDataHelper::getDefault();
    }
    
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
    if (!shape_props_.tryExtract(data)) {
        // Log warning and use default values
        ofLogWarning("PropertyExtraction") << "Failed to extract ShapeData in getHeight(), using defaults";
        data = ShapeDataHelper::getDefault();
    }
    
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
