#pragma once

#include "ofMain.h"
#include "ofxAELayerSource.h"
#include "ofJson.h"
#include <memory>
#include <vector>
#include <string>

namespace ofx { namespace ae {

// ========================================================================
// Forward Declarations and Essential Shape Classes
// ========================================================================

// Shape path vertex with bezier support
struct PathVertex {
    glm::vec2 position;
    glm::vec2 inTangent;
    glm::vec2 outTangent;
    
    PathVertex() : position(0, 0), inTangent(0, 0), outTangent(0, 0) {}
    PathVertex(const glm::vec2& pos) : position(pos), inTangent(0, 0), outTangent(0, 0) {}
    PathVertex(const glm::vec2& pos, const glm::vec2& inTan, const glm::vec2& outTan)
        : position(pos), inTangent(inTan), outTangent(outTan) {}
};

// Shape path definition
class ShapePath {
public:
    ShapePath() : closed(false) {}
    
    void addVertex(const PathVertex& vertex) { vertices.push_back(vertex); }
    void clear() { vertices.clear(); }
    void setClosed(bool closed) { this->closed = closed; }
    bool isClosed() const { return closed; }
    
    const std::vector<PathVertex>& getVertices() const { return vertices; }
    size_t getVertexCount() const { return vertices.size(); }
    
    ofRectangle getBounds() const;
    void generatePath(ofPath& path) const;
    bool loadFromJson(const ofJson& pathData);
    
private:
    std::vector<PathVertex> vertices;
    bool closed;
};

// Base class for all shape geometries
class ShapeGeometry {
public:
    enum Type { ELLIPSE, RECTANGLE, PATH, POLYGON };
    
    ShapeGeometry(Type type) : type_(type) {}
    virtual ~ShapeGeometry() = default;
    
    Type getType() const { return type_; }
    virtual void generatePath(ofPath& path) const = 0;
    virtual ofRectangle getBounds() const = 0;
    virtual bool loadFromJson(const ofJson& data) = 0;
    virtual std::unique_ptr<ShapeGeometry> clone() const = 0;
    
protected:
    Type type_;
};

// Shape properties (fill and stroke)
struct ShapeFill {
    bool enabled;
    ofColor color;
    float opacity;
    
    ShapeFill() : enabled(true), color(255, 255, 255, 255), opacity(1.0f) {}
};

struct ShapeStroke {
    enum LineCap { BUTT = 0, ROUND = 1, SQUARE = 2 };
    enum LineJoin { MITER = 0, ROUND_JOIN = 1, BEVEL = 2 };
    
    bool enabled;
    ofColor color;
    float opacity;
    float width;
    LineCap lineCap;
    LineJoin lineJoin;
    float miterLimit;
    
    ShapeStroke() : enabled(false), color(0, 0, 0, 255), opacity(1.0f), width(1.0f),
                   lineCap(BUTT), lineJoin(MITER), miterLimit(4.0f) {}
};

class ShapeProperties {
public:
    void setFill(const ShapeFill& fill) { fill_ = fill; }
    const ShapeFill& getFill() const { return fill_; }
    
    void setStroke(const ShapeStroke& stroke) { stroke_ = stroke; }
    const ShapeStroke& getStroke() const { return stroke_; }
    
    bool loadFromJson(const ofJson& data);
    
private:
    ShapeFill fill_;
    ShapeStroke stroke_;
};

// Base class for shape items
class ShapeItem {
public:
    enum Type { GEOMETRY, GROUP };
    
    ShapeItem(Type type) : type_(type) {}
    virtual ~ShapeItem() = default;
    
    Type getType() const { return type_; }
    virtual bool loadFromJson(const ofJson& data) = 0;
    
protected:
    Type type_;
};

// Geometry item wrapper
class ShapeGeometryItem : public ShapeItem {
public:
    ShapeGeometryItem() : ShapeItem(GEOMETRY) {}
    
    void setGeometry(std::unique_ptr<ShapeGeometry> geometry) { geometry_ = std::move(geometry); }
    const ShapeGeometry* getGeometry() const { return geometry_.get(); }
    
    void setProperties(const ShapeProperties& properties) { properties_ = properties; }
    const ShapeProperties& getProperties() const { return properties_; }
    
    bool loadFromJson(const ofJson& data) override;
    
private:
    std::unique_ptr<ShapeGeometry> geometry_;
    ShapeProperties properties_;
};

// Shape group container
class ShapeGroup : public ShapeItem {
public:
    ShapeGroup() : ShapeItem(GROUP), visible_(true), opacity_(1.0f) {}
    
    void addItem(std::unique_ptr<ShapeItem> item) { if (item) items_.push_back(std::move(item)); }
    void clear() { items_.clear(); }
    size_t getItemCount() const { return items_.size(); }
    const ShapeItem* getItem(size_t index) const;
    
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    void setOpacity(float opacity) { opacity_ = ofClamp(opacity, 0.0f, 1.0f); }
    float getOpacity() const { return opacity_; }
    
    void render() const;
    ofRectangle getBounds() const;
    bool loadFromJson(const ofJson& data) override;
    
private:
    std::vector<std::unique_ptr<ShapeItem>> items_;
    std::string name_;
    bool visible_;
    float opacity_;
};

// Simple shape geometry implementations
class ShapeEllipse : public ShapeGeometry {
public:
    ShapeEllipse() : ShapeGeometry(ELLIPSE), size_(100, 100), position_(0, 0) {}
    
    void setSize(const glm::vec2& size) { size_ = size; }
    void setPosition(const glm::vec2& position) { position_ = position; }
    
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    glm::vec2 size_;
    glm::vec2 position_;
};

class ShapeRectangle : public ShapeGeometry {
public:
    ShapeRectangle() : ShapeGeometry(RECTANGLE), size_(100, 100), position_(0, 0), roundness_(0) {}
    
    void setSize(const glm::vec2& size) { size_ = size; }
    void setPosition(const glm::vec2& position) { position_ = position; }
    void setRoundness(float roundness) { roundness_ = roundness; }
    
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    glm::vec2 size_;
    glm::vec2 position_;
    float roundness_;
};

class ShapePathGeometry : public ShapeGeometry {
public:
    ShapePathGeometry() : ShapeGeometry(PATH) {}
    
    void setPath(const ShapePath& path) { path_ = path; }
    const ShapePath& getPath() const { return path_; }
    
    void generatePath(ofPath& path) const override;
    ofRectangle getBounds() const override;
    bool loadFromJson(const ofJson& data) override;
    std::unique_ptr<ShapeGeometry> clone() const override;
    
private:
    ShapePath path_;
};

/**
 * ShapeSource - LayerSource implementation for vector shapes and paths
 *
 * This class migrates the existing ShapeLayer functionality to the new 
 * Source-based architecture, enabling vector shapes to be used as layer sources
 * with full After Effects compatibility.
 * 
 * Features:
 * - Full compatibility with existing Shape classes (ShapeGroup, ShapeGeometry, etc.)
 * - After Effects JSON format support
 * - Efficient bounds calculation with caching
 * - Hierarchical shape group rendering
 * - Vector graphics optimization
 */
class ShapeSource : public LayerSource {
public:
    ShapeSource();
    ~ShapeSource() override;
    
    // ========================================================================
    // LayerSource Interface Implementation
    // ========================================================================
    
    /**
     * Initialize the shape source from JSON data exported from After Effects.
     * Parses shape data structure and creates the shape hierarchy.
     * 
     * Expected JSON format:
     * {
     *   "sourceType": "shape",
     *   "shapes": [
     *     {
     *       "type": "ellipse",
     *       "size": [100, 100],
     *       "position": [50, 50],
     *       "fill": { "enabled": true, "color": [1.0, 0.5, 0.25, 1.0] },
     *       "stroke": { "enabled": false }
     *     }
     *   ]
     * }
     * 
     * @param json JSON object containing shape configuration
     * @return true if setup was successful, false otherwise
     */
    bool setup(const ofJson& json) override;
    
    /**
     * Update the shape source for the current frame/time.
     * Handles time-dependent updates and invalidates bounds cache if needed.
     * 
     * @param currentTime Current composition time in seconds
     */
    void update(float currentTime) override;
    
    /**
     * Render the shape source within the provided context.
     * Applies transformations, opacity, and blend modes before rendering shapes.
     * 
     * @param context Rendering context with transform, opacity, blend mode, etc.
     */
    void draw(const RenderContext& context) const override;
    
    /**
     * Get the source type - always returns SHAPE
     * @return SourceType::SHAPE
     */
    SourceType getSourceType() const override { return SHAPE; }
    
    /**
     * Get the width of the shape bounds
     * @return Width in pixels
     */
    float getWidth() const override;
    
    /**
     * Get the height of the shape bounds
     * @return Height in pixels
     */
    float getHeight() const override;
    
    /**
     * Get the bounding rectangle of all shapes
     * Uses cached bounds for performance, recalculates when needed
     * @return Rectangle representing shape bounds
     */
    ofRectangle getBounds() const override;
    
    // ========================================================================
    // Rendering Control Overrides
    // ========================================================================
    
    /**
     * Shapes can be cached since they're vector-based and deterministic
     * @return true to enable caching optimizations
     */
    bool canCache() const override { return true; }
    
    /**
     * Get debug information about this shape source
     * @return String containing shape count and bounds info
     */
    std::string getDebugInfo() const override;
    
    // ========================================================================
    // Shape-Specific Methods (migrated from ShapeLayer)
    // ========================================================================
    
    /**
     * Set the root shape group for this source
     * @param rootGroup Unique pointer to the root ShapeGroup
     */
    void setRootGroup(std::unique_ptr<ShapeGroup> rootGroup);
    
    /**
     * Get the root shape group (const version)
     * @return Const pointer to root group, or nullptr if not set
     */
    const ShapeGroup* getRootGroup() const { return rootGroup_.get(); }
    
    /**
     * Get the root shape group (mutable version)
     * @return Pointer to root group, or nullptr if not set
     */
    ShapeGroup* getRootGroup() { return rootGroup_.get(); }
    
    /**
     * Load shape data from JSON structure
     * Compatible with existing ShapeLayer JSON format
     * 
     * @param shapeData JSON object containing shape definitions
     * @return true if loading was successful
     */
    bool loadShapeData(const ofJson& shapeData);
    
    /**
     * Render all shapes using the current OpenGL state
     * This is the core rendering method used by draw()
     */
    void renderShapes() const;

private:
    // ========================================================================
    // Core Data Members (migrated from ShapeLayer)
    // ========================================================================
    
    /// Root shape group containing all shape hierarchy
    std::unique_ptr<ShapeGroup> rootGroup_;
    
    /// Cached bounds rectangle for performance optimization
    mutable ofRectangle cachedBounds_;
    
    /// Flag indicating if bounds need recalculation
    mutable bool boundsNeedUpdate_;
    
    /// Last update time for change detection
    float lastUpdateTime_;
    
    // ========================================================================
    // Helper Methods (migrated from ShapeLayer)
    // ========================================================================
    
    /**
     * Create geometry object from JSON data
     * Supports ellipse, rectangle, and path geometries
     * 
     * @param data JSON object with geometry definition
     * @return Unique pointer to created geometry, or nullptr on failure
     */
    std::unique_ptr<ShapeGeometry> createGeometryFromJson(const ofJson& data);
    
    /**
     * Create shape item (geometry or group) from JSON data
     * Determines type and creates appropriate ShapeItem subclass
     * 
     * @param data JSON object with item definition
     * @return Unique pointer to created item, or nullptr on failure
     */
    std::unique_ptr<ShapeItem> createItemFromJson(const ofJson& data);
    
    /**
     * Parse shape properties (fill, stroke) from JSON data
     * Updates the provided properties object with parsed values
     * 
     * @param data JSON object containing properties
     * @param properties Properties object to update
     * @return true if parsing was successful
     */
    bool parseShapeProperties(const ofJson& data, ShapeProperties& properties);
    
    /**
     * Invalidate the cached bounds and mark for recalculation
     * Called when shape structure or properties change
     */
    void invalidateBounds() const;
    
    /**
     * Calculate bounds from the current shape hierarchy
     * Updates cachedBounds_ and resets boundsNeedUpdate_ flag
     */
    void calculateBounds() const;
};

}} // namespace ofx::ae
