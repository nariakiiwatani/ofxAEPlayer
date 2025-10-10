#include "ofxAEShapeSource.h"
#include "ofGraphics.h"
#include "ofLog.h"

namespace ofx { namespace ae {

// ========================================================================
// Constructor and Destructor
// ========================================================================

ShapeSource::ShapeSource() 
    : boundsNeedUpdate_(true)
    , lastUpdateTime_(-1.0f) {
    rootGroup_ = std::make_unique<ShapeGroup>();
}

ShapeSource::~ShapeSource() {
    cleanup();
}

// ========================================================================
// LayerSource Interface Implementation
// ========================================================================

bool ShapeSource::setup(const ofJson& json) {
    try {
        // Initialize root group if not exists
        if (!rootGroup_) {
            rootGroup_ = std::make_unique<ShapeGroup>();
        }
        
        // Load shape data from different possible JSON structures
        bool loadSuccess = false;
        
        // Try direct shapes array (simple format)
        if (json.contains("shapes") && json["shapes"].is_array()) {
            ofJson shapeStructure;
            shapeStructure["properties"] = json["shapes"];
            loadSuccess = loadShapeData(shapeStructure);
        }
        // Try properties array (After Effects format)
        else if (json.contains("properties") && json["properties"].is_array()) {
            loadSuccess = loadShapeData(json);
        }
        // Try nested layer structure
        else if (json.contains("layer") && json["layer"].contains("properties")) {
            loadSuccess = loadShapeData(json["layer"]);
        }
        // Try loading as direct shape group
        else {
            loadSuccess = rootGroup_->loadFromJson(json);
        }
        
        if (!loadSuccess) {
            ofLogWarning("ShapeSource") << "Failed to load shape data, creating empty root group";
            rootGroup_ = std::make_unique<ShapeGroup>();
        }
        
        // Invalidate bounds for recalculation
        invalidateBounds();
        
        ofLogVerbose("ShapeSource") << "Shape source setup completed successfully";
        return true;
        
    } catch (const std::exception& e) {
        ofLogError("ShapeSource") << "Exception during setup: " << e.what();
        return false;
    }
}

void ShapeSource::update(float currentTime) {
    // Check if update is needed
    if (lastUpdateTime_ == currentTime) {
        return;
    }
    
    lastUpdateTime_ = currentTime;
    
    // For now, shapes are static, but this can be extended for:
    // - Animated shape properties
    // - Keyframe interpolation
    // - Dynamic shape modifications
    
    // Mark bounds for update if needed
    // In the future, this could be more intelligent based on what changed
    invalidateBounds();
}

void ShapeSource::draw(const RenderContext& context) const {
    if (!rootGroup_ || !isVisible()) {
        return;
    }
    
    // Setup rendering context
    ofPushMatrix();
    ofPushStyle();
    
    try {
        // Apply context transformation
        ofMultMatrix(context.transform);
        
        // Apply context opacity
        ofColor currentColor = ofGetStyle().color;
        currentColor.a = static_cast<int>(currentColor.a * context.opacity);
        ofSetColor(currentColor);
        
        // Apply blend mode (basic implementation)
        // TODO: Implement full After Effects blend mode compatibility
        switch (context.blendMode) {
            case BlendMode::NORMAL:
                ofEnableAlphaBlending();
                break;
            case BlendMode::ADD:
                ofEnableBlendMode(OF_BLENDMODE_ADD);
                break;
            case BlendMode::MULTIPLY:
                ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
                break;
            case BlendMode::SCREEN:
                ofEnableBlendMode(OF_BLENDMODE_SCREEN);
                break;
            case BlendMode::SUBTRACT:
                ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
                break;
            default:
                ofEnableAlphaBlending();
                break;
        }
        
        // Render all shapes
        renderShapes();
        
    } catch (const std::exception& e) {
        ofLogError("ShapeSource") << "Exception during draw: " << e.what();
    }
    
    // Restore state
    ofPopStyle();
    ofPopMatrix();
}

float ShapeSource::getWidth() const {
    const ofRectangle& bounds = getBounds();
    return bounds.width;
}

float ShapeSource::getHeight() const {
    const ofRectangle& bounds = getBounds();
    return bounds.height;
}

ofRectangle ShapeSource::getBounds() const {
    if (boundsNeedUpdate_) {
        calculateBounds();
    }
    return cachedBounds_;
}

std::string ShapeSource::getDebugInfo() const {
    std::stringstream info;
    info << "ShapeSource[";
    
    if (rootGroup_) {
        info << "items:" << rootGroup_->getItemCount();
        
        const ofRectangle& bounds = getBounds();
        info << ", bounds:(" << bounds.x << "," << bounds.y 
             << "," << bounds.width << "," << bounds.height << ")";
    } else {
        info << "no_root_group";
    }
    
    info << "]";
    return info.str();
}

// ========================================================================
// Shape-Specific Methods (migrated from ShapeLayer)
// ========================================================================

void ShapeSource::setRootGroup(std::unique_ptr<ShapeGroup> rootGroup) {
    rootGroup_ = std::move(rootGroup);
    invalidateBounds();
}

bool ShapeSource::loadShapeData(const ofJson& shapeData) {
    try {
        if (!rootGroup_) {
            rootGroup_ = std::make_unique<ShapeGroup>();
        }
        
        bool success = rootGroup_->loadFromJson(shapeData);
        if (success) {
            invalidateBounds();
        }
        
        return success;
        
    } catch (const std::exception& e) {
        ofLogError("ShapeSource") << "Failed to load shape data: " << e.what();
        return false;
    }
}

void ShapeSource::renderShapes() const {
    if (rootGroup_) {
        rootGroup_->render();
    }
}

// ========================================================================
// Helper Methods (migrated from ShapeLayer)
// ========================================================================

std::unique_ptr<ShapeGeometry> ShapeSource::createGeometryFromJson(const ofJson& data) {
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
        ofLogWarning("ShapeSource") << "Unknown geometry type: " << type;
        return nullptr;
    }
    
    if (geometry && geometry->loadFromJson(data)) {
        return geometry;
    }
    
    return nullptr;
}

std::unique_ptr<ShapeItem> ShapeSource::createItemFromJson(const ofJson& data) {
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

bool ShapeSource::parseShapeProperties(const ofJson& data, ShapeProperties& properties) {
    return properties.loadFromJson(data);
}

void ShapeSource::invalidateBounds() const {
    boundsNeedUpdate_ = true;
}

void ShapeSource::calculateBounds() const {
    if (!rootGroup_) {
        cachedBounds_ = ofRectangle();
        boundsNeedUpdate_ = false;
        return;
    }
    
    cachedBounds_ = rootGroup_->getBounds();
    boundsNeedUpdate_ = false;
    
    ofLogVerbose("ShapeSource") << "Calculated bounds: " 
                                << cachedBounds_.x << "," << cachedBounds_.y 
                                << " " << cachedBounds_.width << "x" << cachedBounds_.height;
}

// ========================================================================
// Essential Shape Class Implementations
// ========================================================================

// ShapePath implementation
ofRectangle ShapePath::getBounds() const {
    if (vertices.empty()) return ofRectangle();
    
    glm::vec2 min = vertices[0].position;
    glm::vec2 max = vertices[0].position;
    
    for (const auto& vertex : vertices) {
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
    }
    
    return ofRectangle(min.x, min.y, max.x - min.x, max.y - min.y);
}

void ShapePath::generatePath(ofPath& path) const {
    path.clear();
    if (vertices.empty()) return;
    
    const PathVertex& firstVertex = vertices[0];
    path.moveTo(firstVertex.position.x, firstVertex.position.y);
    
    for (size_t i = 0; i < vertices.size(); i++) {
        size_t nextIndex = (i + 1) % vertices.size();
        if (!closed && nextIndex == 0) break;
        
        const PathVertex& currentVertex = vertices[i];
        const PathVertex& nextVertex = vertices[nextIndex];
        
        glm::vec2 cp1 = currentVertex.position + currentVertex.outTangent;
        glm::vec2 cp2 = nextVertex.position + nextVertex.inTangent;
        
        path.bezierTo(cp1.x, cp1.y, cp2.x, cp2.y, nextVertex.position.x, nextVertex.position.y);
    }
    
    if (closed) path.close();
}

bool ShapePath::loadFromJson(const ofJson& pathData) {
    try {
        clear();
        if (!pathData.contains("vertices")) return false;
        
        const auto& verticesData = pathData["vertices"];
        for (size_t i = 0; i < verticesData.size(); i++) {
            const auto& vertexData = verticesData[i];
            if (!vertexData.is_array() || vertexData.size() < 2) continue;
            
            PathVertex vertex;
            vertex.position = glm::vec2(vertexData[0], vertexData[1]);
            addVertex(vertex);
        }
        
        if (pathData.contains("closed")) {
            setClosed(pathData["closed"]);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// ShapeProperties implementation
bool ShapeProperties::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("fill")) {
            const auto& fillData = data["fill"];
            fill_.enabled = fillData.value("enabled", true);
            if (fillData.contains("color") && fillData["color"].is_array() && fillData["color"].size() >= 3) {
                const auto& colorArray = fillData["color"];
                fill_.color = ofColor(
                    static_cast<int>(colorArray[0].get<float>() * 255),
                    static_cast<int>(colorArray[1].get<float>() * 255),
                    static_cast<int>(colorArray[2].get<float>() * 255),
                    colorArray.size() > 3 ? static_cast<int>(colorArray[3].get<float>() * 255) : 255
                );
            }
            fill_.opacity = fillData.value("opacity", 1.0f);
        }
        
        if (data.contains("stroke")) {
            const auto& strokeData = data["stroke"];
            stroke_.enabled = strokeData.value("enabled", false);
            stroke_.width = strokeData.value("width", 1.0f);
            stroke_.opacity = strokeData.value("opacity", 1.0f);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// ShapeGeometryItem implementation
bool ShapeGeometryItem::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("type")) {
            std::string type = data["type"];
            
            if (type == "ellipse") {
                geometry_ = std::make_unique<ShapeEllipse>();
            } else if (type == "rectangle") {
                geometry_ = std::make_unique<ShapeRectangle>();
            } else if (type == "path") {
                geometry_ = std::make_unique<ShapePathGeometry>();
            } else {
                return false;
            }
            
            if (!geometry_->loadFromJson(data)) {
                return false;
            }
        }
        
        if (data.contains("properties")) {
            return properties_.loadFromJson(data["properties"]);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// ShapeGroup implementation
const ShapeItem* ShapeGroup::getItem(size_t index) const {
    if (index < items_.size()) {
        return items_[index].get();
    }
    return nullptr;
}

void ShapeGroup::render() const {
    if (!visible_ || opacity_ <= 0.0f) return;
    
    ofPushStyle();
    ofColor currentColor = ofGetStyle().color;
    currentColor.a = static_cast<int>(currentColor.a * opacity_);
    ofSetColor(currentColor);
    
    for (const auto& item : items_) {
        if (item->getType() == ShapeItem::GEOMETRY) {
            const ShapeGeometryItem* geometryItem = static_cast<const ShapeGeometryItem*>(item.get());
            if (geometryItem->getGeometry()) {
                ofPath path;
                geometryItem->getGeometry()->generatePath(path);
                
                const ShapeProperties& properties = geometryItem->getProperties();
                if (properties.getFill().enabled) {
                    path.setFilled(true);
                    path.setFillColor(properties.getFill().color);
                } else {
                    path.setFilled(false);
                }
                
                if (properties.getStroke().enabled) {
                    path.setStrokeWidth(properties.getStroke().width);
                    path.setStrokeColor(properties.getStroke().color);
                }
                
                path.draw();
            }
        } else if (item->getType() == ShapeItem::GROUP) {
            const ShapeGroup* group = static_cast<const ShapeGroup*>(item.get());
            group->render();
        }
    }
    
    ofPopStyle();
}

ofRectangle ShapeGroup::getBounds() const {
    if (items_.empty()) return ofRectangle();
    
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
        
        if (data.contains("name")) {
            name_ = data["name"];
        }
        
        if (data.contains("visible")) {
            visible_ = data["visible"];
        }
        
        if (data.contains("opacity")) {
            opacity_ = data["opacity"];
        }
        
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
        return false;
    }
}

// Shape geometry implementations
void ShapeEllipse::generatePath(ofPath& path) const {
    path.clear();
    float radiusX = size_.x * 0.5f;
    float radiusY = size_.y * 0.5f;
    path.ellipse(position_.x, position_.y, radiusX, radiusY);
}

ofRectangle ShapeEllipse::getBounds() const {
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    return ofRectangle(position_.x - halfWidth, position_.y - halfHeight, size_.x, size_.y);
}

bool ShapeEllipse::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("size") && data["size"].is_array() && data["size"].size() >= 2) {
            size_ = glm::vec2(data["size"][0], data["size"][1]);
        }
        if (data.contains("position") && data["position"].is_array() && data["position"].size() >= 2) {
            position_ = glm::vec2(data["position"][0], data["position"][1]);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::unique_ptr<ShapeGeometry> ShapeEllipse::clone() const {
    auto copy = std::make_unique<ShapeEllipse>();
    copy->size_ = size_;
    copy->position_ = position_;
    return std::move(copy);
}

void ShapeRectangle::generatePath(ofPath& path) const {
    path.clear();
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    path.rectangle(position_.x - halfWidth, position_.y - halfHeight, size_.x, size_.y);
}

ofRectangle ShapeRectangle::getBounds() const {
    float halfWidth = size_.x * 0.5f;
    float halfHeight = size_.y * 0.5f;
    return ofRectangle(position_.x - halfWidth, position_.y - halfHeight, size_.x, size_.y);
}

bool ShapeRectangle::loadFromJson(const ofJson& data) {
    try {
        if (data.contains("size") && data["size"].is_array() && data["size"].size() >= 2) {
            size_ = glm::vec2(data["size"][0], data["size"][1]);
        }
        if (data.contains("position") && data["position"].is_array() && data["position"].size() >= 2) {
            position_ = glm::vec2(data["position"][0], data["position"][1]);
        }
        if (data.contains("roundness")) {
            roundness_ = data["roundness"];
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::unique_ptr<ShapeGeometry> ShapeRectangle::clone() const {
    auto copy = std::make_unique<ShapeRectangle>();
    copy->size_ = size_;
    copy->position_ = position_;
    copy->roundness_ = roundness_;
    return std::move(copy);
}

void ShapePathGeometry::generatePath(ofPath& path) const {
    path_.generatePath(path);
}

ofRectangle ShapePathGeometry::getBounds() const {
    return path_.getBounds();
}

bool ShapePathGeometry::loadFromJson(const ofJson& data) {
    return path_.loadFromJson(data);
}

std::unique_ptr<ShapeGeometry> ShapePathGeometry::clone() const {
    auto copy = std::make_unique<ShapePathGeometry>();
    copy->path_ = path_;
    return std::move(copy);
}

}} // namespace ofx::ae
