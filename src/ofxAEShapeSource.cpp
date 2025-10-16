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

	return true;
}

void ShapeSource::update() {
}

bool ShapeSource::setFrame(int frame) {
    return shape_props_.setFrame(frame);
}

void ShapeSource::draw(float x, float y, float w, float h) const {
    RenderContext::push();
	ShapeData data;
	shape_props_.extract(data);
	for(auto it = data.rbegin(); it != data.rend(); ++it) {
		// TODO: rnder function

	}
    RenderContext::pop();
}

float ShapeSource::getWidth() const {
    return 0;
}

float ShapeSource::getHeight() const {
    return 0;
}

void ShapeSource::renderShapeItem(const ShapeItem& item, float x, float y, float w, float h) const {
    std::visit([this, x, y, w, h](const auto& shape) {
        using T = std::decay_t<decltype(shape)>;
        
        if constexpr (std::is_same_v<T, FillData>) {
            current_fill_ = shape;
            has_fill_active_ = true;
        } else if constexpr (std::is_same_v<T, StrokeData>) {
            current_stroke_ = shape;
            has_stroke_active_ = true;
        } else if constexpr (std::is_same_v<T, EllipseData>) {
            // Apply current fill/stroke state and render
            if (has_fill_active_) {
                applyFill(current_fill_);
                ofFill();
                renderEllipse(shape, x, y, w, h);
            }
            if (has_stroke_active_) {
                applyStroke(current_stroke_);
                ofNoFill();
                renderEllipse(shape, x, y, w, h);
            }
        } else if constexpr (std::is_same_v<T, RectangleData>) {
            // Apply current fill/stroke state and render
            if (has_fill_active_) {
                applyFill(current_fill_);
                ofFill();
                renderRectangle(shape, x, y, w, h);
            }
            if (has_stroke_active_) {
                applyStroke(current_stroke_);
                ofNoFill();
                renderRectangle(shape, x, y, w, h);
            }
        } else if constexpr (std::is_same_v<T, PolygonData>) {
            // Apply current fill/stroke state and render
            if (has_fill_active_) {
                applyFill(current_fill_);
                ofFill();
                renderPolygon(shape, x, y, w, h);
            }
            if (has_stroke_active_) {
                applyStroke(current_stroke_);
                ofNoFill();
                renderPolygon(shape, x, y, w, h);
            }
        } else if constexpr (std::is_same_v<T, GroupData>) {
            renderGroup(shape, x, y, w, h);
        }
    }, item);
}

void ShapeSource::renderEllipse(const EllipseData& ellipse, float x, float y, float w, float h) const {
    // Calculate position and size
    // After Effects position is from shape center, openFrameworks draws from top-left
    float ellipseX = x + ellipse.position.x - ellipse.size.x * 0.5f;
    float ellipseY = y + ellipse.position.y - ellipse.size.y * 0.5f;
    
    // Scale size to fit within the given bounds if needed
    float scaleX = (w > 0) ? w / ellipse.size.x : 1.0f;
    float scaleY = (h > 0) ? h / ellipse.size.y : 1.0f;
    
    ofDrawEllipse(ellipseX, ellipseY, ellipse.size.x * scaleX, ellipse.size.y * scaleY);
}

void ShapeSource::renderRectangle(const RectangleData& rectangle, float x, float y, float w, float h) const {
    // Calculate position and size
    // After Effects position is from shape center, openFrameworks draws from top-left
    float rectX = x + rectangle.position.x - rectangle.size.x * 0.5f;
    float rectY = y + rectangle.position.y - rectangle.size.y * 0.5f;
    
    // Scale size to fit within the given bounds if needed
    float scaleX = (w > 0) ? w / rectangle.size.x : 1.0f;
    float scaleY = (h > 0) ? h / rectangle.size.y : 1.0f;
    
    if (rectangle.roundness > 0) {
        // Use rounded rectangle
        ofDrawRectRounded(rectX, rectY, rectangle.size.x * scaleX, rectangle.size.y * scaleY, rectangle.roundness);
    } else {
        // Use regular rectangle
        ofDrawRectangle(rectX, rectY, rectangle.size.x * scaleX, rectangle.size.y * scaleY);
    }
}

void ShapeSource::applyFill(const FillData& fill) const {
    // Set fill color with opacity
    ofFloatColor fillColor(fill.color.r, fill.color.g, fill.color.b, fill.opacity);
    RenderContext::mulColorRGBA(fillColor);
}

void ShapeSource::applyStroke(const StrokeData& stroke) const {
    // Set stroke color with opacity
    ofFloatColor strokeColor(stroke.color.r, stroke.color.g, stroke.color.b, stroke.opacity);
    RenderContext::mulColorRGBA(strokeColor);
    
    // Set stroke width
    ofSetLineWidth(stroke.width);
    
    // Apply line cap style
    switch (stroke.lineCap) {
        case 1: // Round cap
            // Note: openFrameworks doesn't have direct line cap control
            // This would need to be implemented at the path level if needed
            break;
        case 2: // Square cap
            break;
        default: // Butt cap (default)
            break;
    }
    
    // Apply line join style
    switch (stroke.lineJoin) {
        case 1: // Round join
            break;
        case 2: // Bevel join
            break;
        default: // Miter join (default)
            break;
    }
    
    // Note: miterLimit, blendMode, and compositeOrder would be handled
    // at the rendering context level if needed for advanced rendering
}

void ShapeSource::renderPolygon(const PolygonData& polygon, float x, float y, float w, float h) const {
    // Calculate position
    float polyX = x + polygon.position.x;
    float polyY = y + polygon.position.y;
    
    // Create polygon points
    int numPoints = polygon.points;
    if (numPoints < 3) return; // Need at least 3 points for a polygon
    
    ofPath path;
    path.setStrokeWidth(0); // Will be handled by stroke rendering
    
    // For stars (type 2), we need inner and outer radius
    bool isStar = (polygon.type == 2);
    float outerRadius = polygon.outerRadius;
    float innerRadius = isStar ? polygon.innerRadius : outerRadius;
    
    // Calculate points
    float angleStep = TWO_PI / numPoints;
    float startAngle = polygon.rotation * DEG_TO_RAD;
    
    for (int i = 0; i < numPoints; i++) {
        float angle = startAngle + i * angleStep;
        float radius = outerRadius;
        
        float pointX = polyX + cos(angle) * radius;
        float pointY = polyY + sin(angle) * radius;
        
        if (i == 0) {
            path.moveTo(pointX, pointY);
        } else {
            path.lineTo(pointX, pointY);
        }
        
        // For stars, add inner point
        if (isStar && i < numPoints) {
            float innerAngle = angle + angleStep * 0.5f;
            float innerPointX = polyX + cos(innerAngle) * innerRadius;
            float innerPointY = polyY + sin(innerAngle) * innerRadius;
            path.lineTo(innerPointX, innerPointY);
        }
    }
    
    path.close();
    
    // Apply direction (clockwise/counterclockwise)
    if (polygon.direction < 0) {
        // For counterclockwise, we could reverse the path or handle it differently
        // For now, we'll keep the same rendering
    }
    
    path.draw();
}

void ShapeSource::renderGroup(const GroupData& group, float x, float y, float w, float h) const {
    // Save current rendering state
    FillData prevFill = current_fill_;
    StrokeData prevStroke = current_stroke_;
    bool prevHasFill = has_fill_active_;
    bool prevHasStroke = has_stroke_active_;
    
    // Reset rendering state for the group
    has_fill_active_ = false;
    has_stroke_active_ = false;
    
    // Render nested shapes in the group
    for (auto it = group.shapes.rbegin(); it != group.shapes.rend(); ++it) {
        renderShapeItem(*it, x, y, w, h);
    }
    
    // Restore previous rendering state
    current_fill_ = prevFill;
    current_stroke_ = prevStroke;
    has_fill_active_ = prevHasFill;
    has_stroke_active_ = prevHasStroke;
}

glm::vec2 ShapeSource::getShapeSize() const {
    // Find the first shape item to determine size
    for (const auto& item : current_shape_data_.items) {
        auto size = std::visit([](const auto& shape) -> glm::vec2 {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, EllipseData>) {
                return shape.size;
            } else if constexpr (std::is_same_v<T, RectangleData>) {
                return shape.size;
            } else if constexpr (std::is_same_v<T, PolygonData>) {
                // For polygons, use outer radius * 2 as approximate size
                return glm::vec2(shape.outerRadius * 2, shape.outerRadius * 2);
            } else {
                return glm::vec2(0, 0);
            }
        }, item);
        
        if (size.x > 0 || size.y > 0) {
            return size;
        }
    }
    return glm::vec2(0, 0);
}

ofRectangle ShapeSource::getShapeBounds() const {
    glm::vec2 size = getShapeSize();
    glm::vec2 position(0, 0);
    
    // Find the first shape item to determine position
    for (const auto& item : current_shape_data_.items) {
        auto pos = std::visit([](const auto& shape) -> glm::vec2 {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, EllipseData>) {
                return shape.position;
            } else if constexpr (std::is_same_v<T, RectangleData>) {
                return shape.position;
            } else if constexpr (std::is_same_v<T, PolygonData>) {
                return shape.position;
            } else {
                return glm::vec2(0, 0);
            }
        }, item);
        
        if (pos.x != 0 || pos.y != 0) {
            position = pos;
            break;
        }
    }
    
    // Calculate bounds (position is center, so offset by half size)
    return ofRectangle(
        position.x - size.x * 0.5f,
        position.y - size.y * 0.5f,
        size.x,
        size.y
    );
}

}} // namespace ofx::ae
