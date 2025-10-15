#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEProperty.h"
#include "ofxAERenderContext.h"

namespace ofx { namespace ae {

// Shape data structures
struct EllipseData {
    glm::vec2 size;
    glm::vec2 position;
};

struct RectangleData {
    glm::vec2 size;
    glm::vec2 position;
    float roundness;
};

struct FillData {
    glm::vec3 color;
    float opacity;
};

struct StrokeData {
    glm::vec3 color;
    float opacity;
    float width;
    // Note: lineCap, lineJoin, miterLimit can be added later
};

// Shape item types for array-based structure
enum ShapeItemType {
    SHAPE_ELLIPSE,
    SHAPE_RECTANGLE,
    SHAPE_POLYGON,
    SHAPE_GROUP,
    SHAPE_FILL,
    SHAPE_STROKE,
    SHAPE_UNKNOWN
};

struct ShapeItem {
    ShapeItemType type;
    EllipseData ellipse;
    RectangleData rectangle;
    FillData fill;
    StrokeData stroke;
    // Note: polygon and group data can be added later
    
    ShapeItem() : type(SHAPE_UNKNOWN) {}
};

struct ShapeData {
    std::vector<ShapeItem> items;
    
    // Utility methods to find items by type
    const ShapeItem* findItem(ShapeItemType type) const {
        for (const auto& item : items) {
            if (item.type == type) return &item;
        }
        return nullptr;
    }
    
    bool hasItem(ShapeItemType type) const {
        return findItem(type) != nullptr;
    }
};

// Property classes following the same pattern as TransformProp
class EllipseProp : public PropertyGroup_<EllipseData>
{
public:
    EllipseProp() {
        registerProperty<VecProp<2>>("/size");
        registerProperty<VecProp<2>>("/position");
    }
    void extract(EllipseData &e) const override {
        getProperty<VecProp<2>>("/size")->get(e.size);
        getProperty<VecProp<2>>("/position")->get(e.position);
    }
};

class RectangleProp : public PropertyGroup_<RectangleData>
{
public:
    RectangleProp() {
        registerProperty<VecProp<2>>("/size");
        registerProperty<VecProp<2>>("/position");
        registerProperty<FloatProp>("/roundness");
    }
    void extract(RectangleData &r) const override {
        getProperty<VecProp<2>>("/size")->get(r.size);
        getProperty<VecProp<2>>("/position")->get(r.position);
        getProperty<FloatProp>("/roundness")->get(r.roundness);
    }
};

class FillProp : public PropertyGroup_<FillData>
{
public:
    FillProp() {
        registerProperty<VecProp<3>>("/color");
        registerProperty<PercentProp>("/opacity");
    }
    void extract(FillData &f) const override {
        getProperty<VecProp<3>>("/color")->get(f.color);
        getProperty<PercentProp>("/opacity")->get(f.opacity);
    }
};

class StrokeProp : public PropertyGroup_<StrokeData>
{
public:
    StrokeProp() {
        registerProperty<VecProp<3>>("/color");
        registerProperty<PercentProp>("/opacity");
        registerProperty<FloatProp>("/width");
    }
    void extract(StrokeData &s) const override {
        getProperty<VecProp<3>>("/color")->get(s.color);
        getProperty<PercentProp>("/opacity")->get(s.opacity);
        getProperty<FloatProp>("/width")->get(s.width);
    }
};

// ShapeItem with all properties embedded
struct ShapeItemWithProps {
    ShapeItemType type;
    std::unique_ptr<EllipseProp> ellipse_prop;
    std::unique_ptr<RectangleProp> rectangle_prop;
    std::unique_ptr<FillProp> fill_prop;
    std::unique_ptr<StrokeProp> stroke_prop;
    
    ShapeItemWithProps() : type(SHAPE_UNKNOWN) {}
    
    // Extract current values to ShapeItem
    void extractTo(ShapeItem& item) const {
        item.type = type;
        if (ellipse_prop) ellipse_prop->extract(item.ellipse);
        if (rectangle_prop) rectangle_prop->extract(item.rectangle);
        if (fill_prop) fill_prop->extract(item.fill);
        if (stroke_prop) stroke_prop->extract(item.stroke);
    }
    
    bool hasAnimation() const {
        if (ellipse_prop && ellipse_prop->hasAnimation()) return true;
        if (rectangle_prop && rectangle_prop->hasAnimation()) return true;
        if (fill_prop && fill_prop->hasAnimation()) return true;
        if (stroke_prop && stroke_prop->hasAnimation()) return true;
        return false;
    }
    
    bool setFrame(int frame) {
        bool changed = false;
        if (ellipse_prop) changed |= ellipse_prop->setFrame(frame);
        if (rectangle_prop) changed |= rectangle_prop->setFrame(frame);
        if (fill_prop) changed |= fill_prop->setFrame(frame);
        if (stroke_prop) changed |= stroke_prop->setFrame(frame);
        return changed;
    }
};

class ShapeProp : public PropertyArray<ShapeItemWithProps>
{
public:
    void extractShapeData(ShapeData &s) const {
        s.items.clear();
        for (const auto& item_with_props : getItems()) {
            ShapeItem item;
            item_with_props.extractTo(item);
            s.items.push_back(item);
        }
    }
    
protected:
    bool setupElement(ShapeItemWithProps &element, const ofJson &json, const ofJson &keyframes) override {
        element.type = detectItemType(json);
        
        switch (element.type) {
            case SHAPE_ELLIPSE:
                element.ellipse_prop = std::make_unique<EllipseProp>();
                element.ellipse_prop->setup(json, keyframes);
                break;
                
            case SHAPE_RECTANGLE:
                element.rectangle_prop = std::make_unique<RectangleProp>();
                element.rectangle_prop->setup(json, keyframes);
                break;
                
            case SHAPE_FILL:
                element.fill_prop = std::make_unique<FillProp>();
                element.fill_prop->setup(json, keyframes);
                break;
                
            case SHAPE_STROKE:
                element.stroke_prop = std::make_unique<StrokeProp>();
                element.stroke_prop->setup(json, keyframes);
                break;
                
            default:
                return false; // Skip unknown types
        }
        
        return true;
    }
    
    bool hasElementAnimation(const ShapeItemWithProps &element) const override {
        return element.hasAnimation();
    }
    
    bool setElementFrame(ShapeItemWithProps &element, int frame) override {
        return element.setFrame(frame);
    }
    
private:
    ShapeItemType detectItemType(const ofJson& item) const {
        if (!item.contains("type")) return SHAPE_UNKNOWN;
        
        std::string type = item["type"].get<std::string>();
        if (type == "ellipse") return SHAPE_ELLIPSE;
        if (type == "rectangle") return SHAPE_RECTANGLE;
        if (type == "fill") return SHAPE_FILL;
        if (type == "stroke") return SHAPE_STROKE;
        if (type == "polygon") return SHAPE_POLYGON;
        if (type == "group") return SHAPE_GROUP;
        
        return SHAPE_UNKNOWN;
    }
};

class ShapeSource : public LayerSource
{
public:
    ShapeSource();
    virtual ~ShapeSource() = default;
    
    // LayerSource interface implementation
    bool setup(const ofJson &json) override;
    void update() override;
    void draw(float x, float y, float w, float h) const override;
    void setFrame(int frame) override;
    
    SourceType getSourceType() const override { return SHAPE; }
    float getWidth() const override;
    float getHeight() const override;
    std::string getDebugInfo() const override { return "ShapeSource"; }

private:
    ShapeProp shape_props_;
    ShapeData current_shape_data_;
    
    // Internal rendering methods
    void renderShape(float x, float y, float w, float h) const;
    void renderShapeItem(const ShapeItem& item, float x, float y, float w, float h) const;
    void renderEllipse(const EllipseData& ellipse, float x, float y, float w, float h) const;
    void renderRectangle(const RectangleData& rectangle, float x, float y, float w, float h) const;
    void applyFill(const FillData& fill) const;
    void applyStroke(const StrokeData& stroke) const;
    
    // Utility methods
    glm::vec2 getShapeSize() const;
    ofRectangle getShapeBounds() const;
    
    // Current rendering state
    mutable FillData current_fill_;
    mutable StrokeData current_stroke_;
    mutable bool has_fill_active_ = false;
    mutable bool has_stroke_active_ = false;
};

}} // namespace ofx::ae
