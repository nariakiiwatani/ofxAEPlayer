#include "ofxAEShapeSource.h"
#include "ofLog.h"
#include "ofMath.h"

namespace ofx { namespace ae {

namespace renderer {
struct Context : public ShapeVisitor {
	Context() { push(); }
	
	void visit(const EllipseData &ellipse) override {
		ofPath path = createEllipsePath(ellipse);
		appendPath(path);
	}
	
	void visit(const RectangleData &rectangle) override {
		ofPath path = createRectanglePath(rectangle);
		appendPath(path);
	}
	
	void visit(const PolygonData &polygon) override {
		ofPath path = createPolygonPath(polygon);
		appendPath(path);
	}
	
	void visit(const FillData &fill) override {
		auto command = [=](const ofPath &path) {
			auto p = path;
			p.setFillColor(fill.color);
			p.setFilled(true);
			p.setStrokeWidth(0);
			p.draw();
		};
		
		if (fill.compositeOrder == 1) {
			prependCommand(command);
		} else {
			appendCommand(command);
		}
	}
	
	void visit(const StrokeData &stroke) override {
		auto command = [=](const ofPath &path) {
			auto p = path;
			p.setStrokeColor(stroke.color);
			p.setStrokeWidth(stroke.width);
			p.setFilled(false);
			p.draw();
		};

		if (stroke.compositeOrder == 1) {
			prependCommand(command);
		} else {
			appendCommand(command);
		}
	}
	
	void visit(const GroupData &group) override {
		push();
		// Set blend mode for group if needed
		// TODO: Implement blend mode setting based on group.blendMode
		
		// Recursively visit all children in the group
		for (auto &&shapePtr : group.data) {
			if (shapePtr) {
				shapePtr->accept(*this);
			}
		}
		
		pop();
	}
	
	void render() const {
		for(auto &&fn : work.top().command) {
			fn();
		}
	}

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

	ofPath& getPath() { return work.top().path; }
	void appendCommand(std::function<void(const ofPath&)> fn) { auto p = getPath(); appendCommand([p,fn]{fn(p);}); }
	void prependCommand(std::function<void(const ofPath&)> fn) { auto p = getPath(); prependCommand([p,fn]{fn(p);}); }
	void appendCommand(std::function<void()> fn) { work.top().command.push_back(fn); }
	void prependCommand(std::function<void()> fn) { work.top().command.push_front(fn); }

	// Helper methods for creating paths from shape data
	ofPath createEllipsePath(const EllipseData &ellipse) {
		ofPath path;
		float ellipseX = ellipse.position.x - ellipse.size.x * 0.5f;
		float ellipseY = ellipse.position.y - ellipse.size.y * 0.5f;
		path.ellipse(ellipseX + ellipse.size.x * 0.5f, ellipseY + ellipse.size.y * 0.5f, ellipse.size.x, ellipse.size.y);
		
		if (ellipse.direction < 0) {
			path = reversePath(path);
		}
		return path;
	}
	
	ofPath createRectanglePath(const RectangleData &rectangle) {
		ofPath path;
		float rectX = rectangle.position.x - rectangle.size.x * 0.5f;
		float rectY = rectangle.position.y - rectangle.size.y * 0.5f;
		
		if (rectangle.roundness > 0) {
			path.rectRounded(rectX, rectY, rectangle.size.x, rectangle.size.y, rectangle.roundness);
		} else {
			path.rectangle(rectX, rectY, rectangle.size.x, rectangle.size.y);
		}
		
		if (rectangle.direction < 0) {
			path = reversePath(path);
		}
		return path;
	}
	
	ofPath createPolygonPath(const PolygonData &polygon) {
		ofPath path;
		
		int numPoints = polygon.points;
		if (numPoints < 3) {
			return path; // Invalid polygon
		}
		
		bool isStar = (polygon.type == 2);
		float outerRadius = polygon.outerRadius;
		float innerRadius = isStar ? polygon.innerRadius : outerRadius;
		
		float angleStep = TWO_PI / numPoints;
		float startAngle = polygon.rotation * DEG_TO_RAD;
		
		bool firstPoint = true;
		for (int i = 0; i < numPoints; i++) {
			float angle = startAngle + i * angleStep;
			
			// Outer point
			float pointX = polygon.position.x + cos(angle) * outerRadius;
			float pointY = polygon.position.y + sin(angle) * outerRadius;
			
			if (firstPoint) {
				path.moveTo(pointX, pointY);
				firstPoint = false;
			} else {
				path.lineTo(pointX, pointY);
			}
			
			// Star inner point
			if (isStar) {
				float innerAngle = angle + angleStep * 0.5f;
				float innerPointX = polygon.position.x + cos(innerAngle) * innerRadius;
				float innerPointY = polygon.position.y + sin(innerAngle) * innerRadius;
				path.lineTo(innerPointX, innerPointY);
			}
		}
		path.close();
		
		if (polygon.direction < 0) {
			path = reversePath(path);
		}
		return path;
	}
	
	ofPath reversePath(const ofPath &path) {
		ofPath reversedPath;
		const auto& outlines = path.getOutline();
		
		for (const auto& outline : outlines) {
			if (outline.size() < 2) continue;
			
			auto vertices = outline.getVertices();
			if (vertices.empty()) continue;
			
			reversedPath.moveTo(vertices.back());
			for (int i = vertices.size() - 2; i >= 0; i--) {
				reversedPath.lineTo(vertices[i]);
			}
			reversedPath.close();
		}
		
		return reversedPath;
	}
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
        
        // 新しいrenderer::Contextシステムを使用
        renderer::Context context;
        for (const auto& shapePtr : data) {
            if (shapePtr) {
                shapePtr->accept(context);
            }
        }
        context.render();
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
