#include "ofxAEShapeSource.h"
#include "ofLog.h"

namespace ofx { namespace ae {

// ShapeSource implementation
ShapeSource::ShapeSource() {
    current_shape_data_ = {};
    has_fill_active_ = false;
    has_stroke_active_ = false;
}

bool ShapeSource::setup(const ofJson &json)
{
	if (!json.contains("shape")) {
		ofLogWarning("ShapeSource") << "No shape data found in JSON";
		return false;
	}

	const auto& shapeJson = json["shape"];

	// Setup keyframes if they exist
	ofJson keyframes = {};
	if (json.contains("keyframes") && json["keyframes"].contains("shape")) {
		keyframes = json["keyframes"]["shape"];
	}

	// Initialize shape properties with PropertyArray-based structure
	shape_props_.setup(shapeJson, keyframes);

	// Extract initial shape data
	shape_props_.extractShapeData(current_shape_data_);

	ofLogNotice("ShapeSource") << "Setup complete with " << current_shape_data_.items.size() << " shape items";
	return true;
}

void ShapeSource::update() {
    // Extract current shape data from properties using PropertyArray pattern
    shape_props_.extractShapeData(current_shape_data_);
}

bool ShapeSource::setFrame(int frame) {
    return shape_props_.setFrame(frame);
}

void ShapeSource::draw(float x, float y, float w, float h) const {
    RenderContext::push();
    renderShape(x, y, w, h);
    RenderContext::pop();
}

float ShapeSource::getWidth() const {
    glm::vec2 size = getShapeSize();
    return size.x;
}

float ShapeSource::getHeight() const {
    glm::vec2 size = getShapeSize();
    return size.y;
}

void ShapeSource::renderShape(float x, float y, float w, float h) const {
    // Reset rendering state
    has_fill_active_ = false;
    has_stroke_active_ = false;
    
    // Process shape items in order
	for(auto it = current_shape_data_.items.rbegin(); it != current_shape_data_.items.rend(); ++it) {
        renderShapeItem(*it, x, y, w, h);
    }
}

void ShapeSource::renderShapeItem(const ShapeItem& item, float x, float y, float w, float h) const {
    switch (item.type) {
        case SHAPE_FILL:
            current_fill_ = item.fill;
            has_fill_active_ = true;
            break;
            
        case SHAPE_STROKE:
            current_stroke_ = item.stroke;
            has_stroke_active_ = true;
            break;
            
        case SHAPE_ELLIPSE:
            // Apply current fill/stroke state and render
            if (has_fill_active_) {
                applyFill(current_fill_);
                ofFill();
                renderEllipse(item.ellipse, x, y, w, h);
            }
            if (has_stroke_active_) {
                applyStroke(current_stroke_);
                ofNoFill();
                renderEllipse(item.ellipse, x, y, w, h);
            }
            break;
            
        case SHAPE_RECTANGLE:
            // Apply current fill/stroke state and render
            if (has_fill_active_) {
                applyFill(current_fill_);
                ofFill();
                renderRectangle(item.rectangle, x, y, w, h);
            }
            if (has_stroke_active_) {
                applyStroke(current_stroke_);
                ofNoFill();
                renderRectangle(item.rectangle, x, y, w, h);
            }
            break;
            
        default:
            // Handle other types (polygon, group) in future
            break;
    }
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
}

glm::vec2 ShapeSource::getShapeSize() const {
    // Find the first shape item to determine size
    for (const auto& item : current_shape_data_.items) {
        switch (item.type) {
            case SHAPE_ELLIPSE:
                return item.ellipse.size;
            case SHAPE_RECTANGLE:
                return item.rectangle.size;
            default:
                break;
        }
    }
    return glm::vec2(0, 0);
}

ofRectangle ShapeSource::getShapeBounds() const {
    glm::vec2 size = getShapeSize();
    glm::vec2 position(0, 0);
    
    // Find the first shape item to determine position
    for (const auto& item : current_shape_data_.items) {
        switch (item.type) {
            case SHAPE_ELLIPSE:
                position = item.ellipse.position;
                goto found_position;
            case SHAPE_RECTANGLE:
                position = item.rectangle.position;
                goto found_position;
            default:
                break;
        }
    }
    
    found_position:
    // Calculate bounds (position is center, so offset by half size)
    return ofRectangle(
        position.x - size.x * 0.5f,
        position.y - size.y * 0.5f,
        size.x,
        size.y
    );
}

}} // namespace ofx::ae
