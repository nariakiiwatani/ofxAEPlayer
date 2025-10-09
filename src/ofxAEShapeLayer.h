#pragma once

#include "ofMain.h"
#include "ofxAELayer.h"
#include "ofJson.h"
#include <vector>
#include <memory>

namespace ofx {
namespace ae {

// ===== SHAPE PATH SYSTEM =====

// Shape path vertex with bezier support (based on MaskVertex but optimized for shapes)
struct PathVertex {
    glm::vec2 position;
    glm::vec2 inTangent;
    glm::vec2 outTangent;
    
    PathVertex() : position(0, 0), inTangent(0, 0), outTangent(0, 0) {}
    PathVertex(const glm::vec2& pos) : position(pos), inTangent(0, 0), outTangent(0, 0) {}
    PathVertex(const glm::vec2& pos, const glm::vec2& inTan, const glm::vec2& outTan) 
        : position(pos), inTangent(inTan), outTangent(outTan) {}
};

// Shape path definition with After Effects compatibility
class ShapePath {
public:
    ShapePath();
    ~ShapePath();
    
    // Path management
    void addVertex(const PathVertex& vertex);
    void addVertex(const glm::vec2& position);
    void addVertex(const glm::vec2& position, const glm::vec2& inTangent, const glm::vec2& outTangent);
    void clear();
    
    void setClosed(bool closed) { this->closed = closed; }
    bool isClosed() const { return closed; }
    
    const std::vector<PathVertex>& getVertices() const { return vertices; }
    std::vector<PathVertex>& getVertices() { return vertices; }
    
    size_t getVertexCount() const { return vertices.size(); }
    
    // Bezier curve evaluation
    glm::vec2 evaluateAt(float t) const;
    void generatePolyline(ofPolyline& polyline, int resolution = 100) const;
    void generatePath(ofPath& path) const;
    
    // Bounds calculation
    ofRectangle getBounds() const;
    
    // After Effects data loading
    bool loadFromJson(const ofJson& pathData);
    ofJson toJson() const;
    
private:
    std::vector<PathVertex> vertices;
    bool closed;
};

// ===== SHAPE GEOMETRY SYSTEM =====

// Base class for all shape geometries
class ShapeGeometry {
public:
    enum Type {
        ELLIPSE,
        RECTANGLE,
        PATH,
        POLYGON
    };
    
    ShapeGeometry(Type type) : type_(type) {}
    virtual ~ShapeGeometry() = default;
    
    Type getType() const { return type_; }
    
    // Pure virtual interface
    virtual void generatePath(ofPath& path) const = 0;
    virtual ofRectangle getBounds() const = 0;
    virtual bool loadFromJson(const ofJson& data) = 0;
    virtual ofJson toJson() const = 0;
    virtual std::unique_ptr<ShapeGeometry> clone() const = 0;
    
protected:
    Type type_;
};

// Ellipse geometry
class ShapeEllipse : public ShapeGeometry {
public:
    ShapeEllipse();
    ~ShapeEllipse() override = default;
    
    // Properties
    void setSize(const glm::vec2& size) { size_ = size; }
    void setPosition(const glm::vec2& position) { position_ = position; }
    
    const glm::vec2& getSize() const { return size_; }
    const glm::vec2& getPosition() const { return position_; }
    
    // ShapeGeometry interface
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    ofJson toJson() const override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    glm::vec2 size_;
    glm::vec2 position_;
};

// Rectangle geometry
class ShapeRectangle : public ShapeGeometry {
public:
    ShapeRectangle();
    ~ShapeRectangle() override = default;
    
    // Properties
    void setSize(const glm::vec2& size) { size_ = size; }
    void setPosition(const glm::vec2& position) { position_ = position; }
    void setRoundness(float roundness) { roundness_ = roundness; }
    
    const glm::vec2& getSize() const { return size_; }
    const glm::vec2& getPosition() const { return position_; }
    float getRoundness() const { return roundness_; }
    
    // ShapeGeometry interface
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    ofJson toJson() const override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    glm::vec2 size_;
    glm::vec2 position_;
    float roundness_;
};

// Path geometry (custom bezier paths)
class ShapePathGeometry : public ShapeGeometry {
public:
    ShapePathGeometry();
    ~ShapePathGeometry() override = default;
    
    // Path management
    void setPath(const ShapePath& path) { path_ = path; }
    const ShapePath& getPath() const { return path_; }
    ShapePath& getPath() { return path_; }
    
    // ShapeGeometry interface
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    ofJson toJson() const override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    ShapePath path_;
};

// ===== SHAPE PROPERTIES SYSTEM =====

// Fill properties
struct ShapeFill {
    bool enabled;
    ofColor color;
    float opacity;
    
    ShapeFill() : enabled(true), color(255, 255, 255, 255), opacity(1.0f) {}
};

// Stroke properties
struct ShapeStroke {
    bool enabled;
    ofColor color;
    float opacity;
    float width;
    
    enum LineCap {
        BUTT = 0,
        ROUND = 1,
        SQUARE = 2
    };
    
    enum LineJoin {
        MITER = 0,
        ROUND_JOIN = 1,
        BEVEL = 2
    };
    
    LineCap lineCap;
    LineJoin lineJoin;
    float miterLimit;
    
    ShapeStroke() : enabled(false), color(0, 0, 0, 255), opacity(1.0f), width(1.0f),
                   lineCap(BUTT), lineJoin(MITER), miterLimit(4.0f) {}
};

// Complete shape properties collection
class ShapeProperties {
public:
    ShapeProperties();
    ~ShapeProperties() = default;
    
    // Fill management
    void setFill(const ShapeFill& fill) { fill_ = fill; }
    const ShapeFill& getFill() const { return fill_; }
    ShapeFill& getFill() { return fill_; }
    
    // Stroke management
    void setStroke(const ShapeStroke& stroke) { stroke_ = stroke; }
    const ShapeStroke& getStroke() const { return stroke_; }
    ShapeStroke& getStroke() { return stroke_; }
    
    // After Effects data loading
    bool loadFromJson(const ofJson& data);
    ofJson toJson() const;
    
private:
    ShapeFill fill_;
    ShapeStroke stroke_;
};

// ===== SHAPE GROUP SYSTEM =====

// Forward declarations
class ShapeGroup;
class ShapeLayer;

// Shape item - can be either geometry or a group
class ShapeItem {
public:
    enum Type {
        GEOMETRY,
        GROUP
    };
    
    ShapeItem(Type type) : type_(type) {}
    virtual ~ShapeItem() = default;
    
    Type getType() const { return type_; }
    virtual bool loadFromJson(const ofJson& data) = 0;
    virtual ofJson toJson() const = 0;
    
protected:
    Type type_;
};

// Geometry item wrapper
class ShapeGeometryItem : public ShapeItem {
public:
    ShapeGeometryItem();
    ~ShapeGeometryItem() override = default;
    
    void setGeometry(std::unique_ptr<ShapeGeometry> geometry) { geometry_ = std::move(geometry); }
    const ShapeGeometry* getGeometry() const { return geometry_.get(); }
    ShapeGeometry* getGeometry() { return geometry_.get(); }
    
    void setProperties(const ShapeProperties& properties) { properties_ = properties; }
    const ShapeProperties& getProperties() const { return properties_; }
    ShapeProperties& getProperties() { return properties_; }
    
    bool loadFromJson(const ofJson& data) override;
    ofJson toJson() const override;
    
private:
    std::unique_ptr<ShapeGeometry> geometry_;
    ShapeProperties properties_;
};

// Shape group - hierarchical container for shape items
class ShapeGroup : public ShapeItem, public TransformNode {
public:
    ShapeGroup();
    ~ShapeGroup() override = default;
    
    // Item management
    void addItem(std::unique_ptr<ShapeItem> item);
    void removeItem(size_t index);
    void clear();
    
    size_t getItemCount() const { return items_.size(); }
    const ShapeItem* getItem(size_t index) const;
    ShapeItem* getItem(size_t index);
    
    // Properties
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }
    
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
    void setOpacity(float opacity) { opacity_ = ofClamp(opacity, 0.0f, 1.0f); }
    float getOpacity() const { return opacity_; }
    
    // Rendering
    void render() const;
    ofRectangle getBounds() const;
    
    // After Effects data loading
    bool loadFromJson(const ofJson& data) override;
    ofJson toJson() const override;
    
private:
    std::vector<std::unique_ptr<ShapeItem>> items_;
    std::string name_;
    bool visible_;
    float opacity_;
    
    void renderGeometryItem(const ShapeGeometryItem* item) const;
    void renderGroup(const ShapeGroup* group) const;
};

// ===== SHAPE LAYER =====

// Main shape layer class
class ShapeLayer : public Layer {
public:
    ShapeLayer();
    ~ShapeLayer() override = default;
    
    // Layer interface
    bool setup(const ofJson &json) override;
    void update() override;
    void draw(float x, float y, float w, float h) const override;
    
    // Shape management
    void setRootGroup(std::unique_ptr<ShapeGroup> rootGroup) { rootGroup_ = std::move(rootGroup); }
    const ShapeGroup* getRootGroup() const { return rootGroup_.get(); }
    ShapeGroup* getRootGroup() { return rootGroup_.get(); }
    
    // After Effects data loading
    bool loadShapeData(const ofJson& shapeData);
    
    // Rendering
    void renderShapes() const;
    
private:
    std::unique_ptr<ShapeGroup> rootGroup_;
    
    // After Effects data parsing helpers
    std::unique_ptr<ShapeGeometry> createGeometryFromJson(const ofJson& data);
    std::unique_ptr<ShapeItem> createItemFromJson(const ofJson& data);
    bool parseShapeProperties(const ofJson& data, ShapeProperties& properties);
};

} // namespace ae
} // namespace ofx
