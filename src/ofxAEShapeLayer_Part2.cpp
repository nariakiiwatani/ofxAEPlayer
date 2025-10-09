#include "ofxAEShapeLayer.h"
#include "ofGraphics.h"
#include "ofLog.h"

namespace ofx {
namespace ae {

// ===== SHAPE GEOMETRY ITEM IMPLEMENTATION =====

ShapeGeometryItem::ShapeGeometryItem() : ShapeItem(GEOMETRY) {}

bool ShapeGeometryItem::loadFromJson(const ofJson& data) {
    try {
        // Load geometry based on type
        if (data.contains("type")) {
            std::string type = data["type"];
            
            if (type == "ellipse") {
                geometry_ = std::make_unique<ShapeEllipse>();
            } else if (type == "rectangle") {
                geometry_ = std::make_unique<ShapeRectangle>();
            } else if (type == "path") {
                geometry_ = std::make_unique<ShapePathGeometry>();
            } else {
                ofLogWarning("ShapeGeometryItem") << "Unknown geometry type: " << type;
                return false;
            }
            
            if (!geometry_->loadFromJson(data)) {
                return false;
            }
        }
        
        // Load properties
        if (data.contains("properties")) {
            if (!properties_.loadFromJson(data["properties"])) {
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapeGeometryItem") << "Failed to load from JSON: " << e.what();
        return false;
    }
}

ofJson ShapeGeometryItem::toJson() const {
    ofJson json;
    
    if (geometry_) {
        json = geometry_->toJson();
    }
    
    json["properties"] = properties_.toJson();
    
    return json;
}

// ===== SHAPE GROUP IMPLEMENTATION =====

ShapeGroup::ShapeGroup() : ShapeItem(GROUP), visible_(true), opacity_(1.0f) {}

void ShapeGroup::addItem(std::unique_ptr<ShapeItem> item) {
    if (item) {
        items_.push_back(std::move(item));
    }
}

void ShapeGroup::removeItem(size_t index) {
    if (index < items_.size()) {
        items_.erase(items_.begin() + index);
    }
}

void ShapeGroup::clear() {
    items_.clear();
}

const ShapeItem* ShapeGroup::getItem(size_t index) const {
    if (index < items_.size()) {
        return items_[index].get();
    }
    return nullptr;
}

ShapeItem* ShapeGroup::getItem(size_t index) {
    if (index < items_.size()) {
        return items_[index].get();
    }
    return nullptr;
}

void ShapeGroup::render() const {
    if (!visible_ || opacity_ <= 0.0f) {
        return;
    }
    
    // Apply group transform
    ofPushMatrix();
    pushMatrix();
    
    // Apply group opacity
    ofPushStyle();
    
    // Get current alpha and multiply with group opacity
    ofColor currentColor = ofGetStyle().color;
    float currentAlpha = currentColor.a / 255.0f;
    float finalAlpha = currentAlpha * opacity_;
    
    // Render all items
    for (const auto& item : items_) {
        if (item->getType() == ShapeItem::GEOMETRY) {
            const ShapeGeometryItem* geometryItem = static_cast<const ShapeGeometryItem*>(item.get());
            renderGeometryItem(geometryItem);
        } else if (item->getType() == ShapeItem::GROUP) {
            const ShapeGroup* group = static_cast<const ShapeGroup*>(item.get());
            renderGroup(group);
        }
    }
    
    ofPopStyle();
    popMatrix();
    ofPopMatrix();
}

void ShapeGroup::renderGeometryItem(const ShapeGeometryItem* item) const {
    if (!item || !item->getGeometry()) return;
    
    const ShapeGeometry* geometry = item->getGeometry();
    const ShapeProperties& properties = item->getProperties();
    
    // Generate path from geometry
    ofPath path;
    geometry->generatePath(path);
    
    // Apply fill
    if (properties.getFill().enabled) {
        const ShapeFill& fill = properties.getFill();
        ofColor fillColor = fill.color;
        fillColor.a = static_cast<int>(fillColor.a * fill.opacity);
        
        path.setFillColor(fillColor);
        path.setFilled(true);
    } else {
        path.setFilled(false);
    }
    
    // Apply stroke
    if (properties.getStroke().enabled) {
        const ShapeStroke& stroke = properties.getStroke();
        ofColor strokeColor = stroke.color;
        strokeColor.a = static_cast<int>(strokeColor.a * stroke.opacity);
        
        path.setStrokeColor(strokeColor);
        path.setStrokeWidth(stroke.width);

        // Set line cap and join
        switch (stroke.lineCap) {
            case ShapeStroke::BUTT:
                // ofPath doesn't have direct line cap control, handled in rendering
                break;
            case ShapeStroke::ROUND:
                // Will be handled in custom rendering if needed
                break;
            case ShapeStroke::SQUARE:
                // Will be handled in custom rendering if needed
                break;
        }
    } else {
		path.setStrokeWidth(0);
    }
    
    // Draw the path
    path.draw();
}

void ShapeGroup::renderGroup(const ShapeGroup* group) const {
    if (group) {
        group->render();
    }
}

ofRectangle ShapeGroup::getBounds() const {
    if (items_.empty()) {
        return ofRectangle();
    }
    
    ofRectangle bounds;
    bool boundsInitialized = false;
    
    for (const auto& item : items_) {
        ofRectangle itemBounds;
        
        if (item->getType() == ShapeItem::GEOMETRY) {
            const ShapeGeometryItem* geometryItem = static_cast<const ShapeGeometryItem*>(item.get());
            if (geometryItem->getGeometry()) {
                itemBounds = geometryItem->getGeometry()->getBounds();
            }
        } else if (item->getType() == ShapeItem::GROUP) {
            const ShapeGroup* group = static_cast<const ShapeGroup*>(item.get());
            itemBounds = group->getBounds();
        }
        
        if (itemBounds.width > 0 && itemBounds.height > 0) {
            if (!boundsInitialized) {
                bounds = itemBounds;
                boundsInitialized = true;
            } else {
                bounds = bounds.getUnion(itemBounds);
            }
        }
    }
    
    return bounds;
}

bool ShapeGroup::loadFromJson(const ofJson& data) {
    try {
        clear();
        
        // Load basic properties
        if (data.contains("name")) {
            name_ = data["name"];
        }
        
        if (data.contains("visible")) {
            visible_ = data["visible"];
        }
        
        if (data.contains("opacity")) {
            opacity_ = data["opacity"];
        }
        
        // Load transform data
        if (data.contains("transform")) {
            const auto& transformData = data["transform"];
            
            if (transformData.contains("position") && transformData["position"].is_array() && transformData["position"].size() >= 2) {
                setTranslation(transformData["position"][0], transformData["position"][1], 0);
            }
            
            if (transformData.contains("scale") && transformData["scale"].is_array() && transformData["scale"].size() >= 2) {
                setScale(transformData["scale"][0], transformData["scale"][1], 1);
            }
            
            if (transformData.contains("rotation")) {
                setRotationZ(transformData["rotation"]);
            }
            
            if (transformData.contains("anchor") && transformData["anchor"].is_array() && transformData["anchor"].size() >= 2) {
                setAnchorPoint(transformData["anchor"][0], transformData["anchor"][1], 0);
            }
        }
        
        // Load child items
        if (data.contains("properties") && data["properties"].is_array()) {
            for (const auto& itemData : data["properties"]) {
                if (itemData.contains("type")) {
                    std::string type = itemData["type"];
                    
                    if (type == "group") {
                        auto childGroup = std::make_unique<ShapeGroup>();
                        if (childGroup->loadFromJson(itemData)) {
                            addItem(std::move(childGroup));
                        }
                    } else {
                        // Geometry item
                        auto geometryItem = std::make_unique<ShapeGeometryItem>();
                        if (geometryItem->loadFromJson(itemData)) {
                            addItem(std::move(geometryItem));
                        }
                    }
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("ShapeGroup") << "Failed to load from JSON: " << e.what();
        return false;
    }
}

ofJson ShapeGroup::toJson() const {
    ofJson json;
    
    json["type"] = "group";
    json["name"] = name_;
    json["visible"] = visible_;
    json["opacity"] = opacity_;
    
    // Export transform
    ofJson transformJson;
    transformJson["position"] = {getTranslation().x, getTranslation().y};
    transformJson["scale"] = {getScale().x, getScale().y};
    transformJson["rotation"] = getRotation().z;
    transformJson["anchor"] = {getAnchorPoint().x, getAnchorPoint().y};
    json["transform"] = transformJson;
    
    // Export child items
    ofJson itemsJson = ofJson::array();
    for (const auto& item : items_) {
        itemsJson.push_back(item->toJson());
    }
    json["properties"] = itemsJson;
    
    return json;
}

// ===== SHAPE LAYER IMPLEMENTATION =====

ShapeLayer::ShapeLayer() {
    rootGroup_ = std::make_unique<ShapeGroup>();
}

bool ShapeLayer::setup(const ofJson &json) {
    // First call parent setup for basic layer properties
    if (!Layer::setup(json)) {
        return false;
    }
    
    // Parse shape-specific data
    if (json.contains("properties") && json["properties"].is_array()) {
        // After Effects shape layer properties are in the "properties" array
        return loadShapeData(json);
    } else if (json.contains("layer") && json["layer"].contains("properties")) {
        // Alternative structure with nested layer data
        return loadShapeData(json["layer"]);
    }
    
    // If no shape properties found, create empty root group
    if (!rootGroup_) {
        rootGroup_ = std::make_unique<ShapeGroup>();
    }
    
    ofLogVerbose("ShapeLayer") << "Shape layer setup completed successfully";
    return true;
}

void ShapeLayer::update() {
    // Call parent update for transform and keyframe processing
    Layer::update();
    
    // Update any shape-specific properties
    // This can be extended for shape animations
}

void ShapeLayer::draw(float x, float y, float w, float h) const {
    // Setup drawing context
    ofPushMatrix();
    ofTranslate(x, y);
    
    // Apply layer transform
    pushMatrix();
    
    // Render shapes
    renderShapes();
    
    // Restore state
    popMatrix();
    ofPopMatrix();
}

void ShapeLayer::renderShapes() const {
    if (rootGroup_ && isVisible()) {
        // Apply layer opacity
        ofPushStyle();
        
        ofColor currentColor = ofGetStyle().color;
        float layerOpacity = getOpacity();
        currentColor.a = static_cast<int>(currentColor.a * layerOpacity);
        ofSetColor(currentColor);
        
        // Render the root group
        rootGroup_->render();
        
        ofPopStyle();
    }
}

bool ShapeLayer::loadShapeData(const ofJson& shapeData) {
    try {
        if (!rootGroup_) {
            rootGroup_ = std::make_unique<ShapeGroup>();
        }
        
        return rootGroup_->loadFromJson(shapeData);
    } catch (const std::exception& e) {
        ofLogError("ShapeLayer") << "Failed to load shape data: " << e.what();
        return false;
    }
}

std::unique_ptr<ShapeGeometry> ShapeLayer::createGeometryFromJson(const ofJson& data) {
    if (!data.contains("type")) {
        return nullptr;
    }
    
    std::string type = data["type"];
    std::unique_ptr<ShapeGeometry> geometry;
    
    if (type == "ellipse") {
        geometry = std::make_unique<ShapeEllipse>();
    } else if (type == "rectangle") {
        geometry = std::make_unique<ShapeRectangle>();
    } else if (type == "path") {
        geometry = std::make_unique<ShapePathGeometry>();
    } else {
        ofLogWarning("ShapeLayer") << "Unknown geometry type: " << type;
        return nullptr;
    }
    
    if (geometry && geometry->loadFromJson(data)) {
        return geometry;
    }
    
    return nullptr;
}

std::unique_ptr<ShapeItem> ShapeLayer::createItemFromJson(const ofJson& data) {
    if (!data.contains("type")) {
        return nullptr;
    }
    
    std::string type = data["type"];
    
    if (type == "group") {
        auto group = std::make_unique<ShapeGroup>();
        if (group->loadFromJson(data)) {
            return std::move(group);
        }
    } else {
        // Geometry item
        auto geometryItem = std::make_unique<ShapeGeometryItem>();
        if (geometryItem->loadFromJson(data)) {
            return std::move(geometryItem);
        }
    }
    
    return nullptr;
}

bool ShapeLayer::parseShapeProperties(const ofJson& data, ShapeProperties& properties) {
    return properties.loadFromJson(data);
}

} // namespace ae
} // namespace ofx
