#include "ofxAEShapeSource.h"
#include "ofLog.h"
#include "ofMath.h"

namespace ofx { namespace ae {

namespace renderer {
struct Context : public ShapeVisitor {
	Context(float opacity=1):opacity(opacity) {
		push();
	}

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
		float alpha = opacity;
		auto command = [=](const ofPath &path) {
			auto p = path;
			ofFloatColor c = fill.color;
			c.a = fill.opacity*alpha;
			switch(fill.rule) {
				case 1: p.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
				case 2: p.setPolyWindingMode(OF_POLY_WINDING_ODD); break;
			}
			p.setFillColor(c);
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
		float alpha = opacity;
		auto command = [=](const ofPath &path) {
			auto p = path;
			ofFloatColor c = stroke.color;
			c.a = stroke.opacity*alpha;
			p.setStrokeColor(c);
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
	float opacity;

	void appendPath(const ofPath &p) { work.top().path.append(p); }
	void push() { work.push(Work()); }
	void pop() { auto w = work.top(); work.pop(); work.top().append(w); }

	ofPath& getPath() { return work.top().path; }
	void appendCommand(std::function<void(const ofPath&)> fn) { auto p = getPath(); appendCommand([p,fn]{fn(p);}); }
	void prependCommand(std::function<void(const ofPath&)> fn) { auto p = getPath(); prependCommand([p,fn]{fn(p);}); }
	void appendCommand(std::function<void()> fn) { work.top().command.push_back(fn); }
	void prependCommand(std::function<void()> fn) { work.top().command.push_front(fn); }

	static float signedArea(const ofPolyline& pl){
		const auto& v = pl.getVertices();
		if(v.size() < 3) return 0.f;
		double a = 0.0;
		for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
			a += (double)v[j].x * v[i].y - (double)v[i].x * v[j].y;
		}
		return (float)(0.5 * a); // >0 ならCCW
	}

	static ofPath enforceWinding(const ofPath& src, int direction){
		int desiredSign = 1;
		switch(direction){
		  case 2: desiredSign = 1; break;
		  case 3: desiredSign = -1; break;
		}
		ofPath out;
		out.setMode(src.getMode());
		out.setCurveResolution(src.getCurveResolution());
		for(const auto& poly : src.getOutline()){
			auto v = poly.getVertices();
			if(v.size() < 3) continue;
			bool ccw = signedArea(poly) > 0;
			if((ccw ? +1 : -1) != desiredSign){
				std::reverse(v.begin(), v.end());
			}
			out.moveTo(v[0]);
			for(size_t i=1;i<v.size();++i) out.lineTo(v[i]);
			out.close();
		}
		return out;
	}
	ofPath createEllipsePath(const EllipseData &e){
		ofPath path;
		const float cx=e.position.x, cy=e.position.y;
		path.ellipse(cx, cy, e.size.x, e.size.y);
		return enforceWinding(path, e.direction);
	}

	ofPath createRectanglePath(const RectangleData &r){
		ofPath path;
		float x=r.position.x - r.size.x*0.5f;
		float y=r.position.y - r.size.y*0.5f;
		if(r.roundness>0) path.rectRounded(x,y,r.size.x,r.size.y,r.roundness);
		else              path.rectangle(x,y,r.size.x,r.size.y);
		return enforceWinding(path, r.direction);
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

			float pointX = polygon.position.x + cos(angle) * outerRadius;
			float pointY = polygon.position.y + sin(angle) * outerRadius;
			
			if (firstPoint) {
				path.moveTo(pointX, pointY);
				firstPoint = false;
			} else {
				path.lineTo(pointX, pointY);
			}
			
			if (isStar) {
				float innerAngle = angle + angleStep * 0.5f;
				float innerPointX = polygon.position.x + cos(innerAngle) * innerRadius;
				float innerPointY = polygon.position.y + sin(innerAngle) * innerRadius;
				path.lineTo(innerPointX, innerPointY);
			}
		}
		path.close();
		
		path = enforceWinding(path, polygon.direction);
		return path;
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
        
		renderer::Context context(RenderContext::getCurrentStyle().color.a);
        for (const auto& shapePtr : data.data) {
            if (shapePtr) {
                shapePtr->accept(context);
            }
        }
        context.render();
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

}} // namespace ofx::ae
