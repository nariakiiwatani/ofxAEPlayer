#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEProperty.h"
#include "ofxAERenderContext.h"
#include <variant>
#include <memory>

namespace ofx { namespace ae {

// Shape data structures
struct EllipseData {
    glm::vec2 size;
    glm::vec2 position;
    int direction = 1; // 1 = clockwise, -1 = counterclockwise
};

struct RectangleData {
    glm::vec2 size;
    glm::vec2 position;
    float roundness;
    int direction = 1; // 1 = clockwise, -1 = counterclockwise
};

struct PolygonData {
    int direction = 1;
    int type = 1; // 1 = polygon, 2 = star
    int points = 5;
    glm::vec2 position;
    float rotation = 0;
    float innerRadius = 50;
    float outerRadius = 100;
    float innerRoundness = 0;
    float outerRoundness = 0;
};

struct GroupData {
    int blendMode = 1;
    std::vector<struct ShapeItem> shapes; // Forward declaration handled below
};

struct FillData {
    glm::vec3 color;
    float opacity;
    int rule = 1; // Fill rule
    int blendMode = 1;
    int compositeOrder = 1;
};

struct StrokeData {
    glm::vec3 color;
    float opacity;
    float width;
    int lineCap = 1;
    int lineJoin = 1;
    float miterLimit = 4;
    int blendMode = 1;
    int compositeOrder = 1;
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
    PolygonData polygon;
    GroupData group;
    FillData fill;
    StrokeData stroke;
    
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
        registerProperty<IntProp>("/direction");
    }
    void extract(EllipseData &e) const override {
        getProperty<VecProp<2>>("/size")->get(e.size);
        getProperty<VecProp<2>>("/position")->get(e.position);
        getProperty<IntProp>("/direction")->get(e.direction);
    }
};

class RectangleProp : public PropertyGroup_<RectangleData>
{
public:
    RectangleProp() {
        registerProperty<VecProp<2>>("/size");
        registerProperty<VecProp<2>>("/position");
        registerProperty<FloatProp>("/roundness");
        registerProperty<IntProp>("/direction");
    }
    void extract(RectangleData &r) const override {
        getProperty<VecProp<2>>("/size")->get(r.size);
        getProperty<VecProp<2>>("/position")->get(r.position);
        getProperty<FloatProp>("/roundness")->get(r.roundness);
        getProperty<IntProp>("/direction")->get(r.direction);
    }
};

class PolygonProp : public PropertyGroup_<PolygonData>
{
public:
    PolygonProp() {
        registerProperty<IntProp>("/direction");
        registerProperty<IntProp>("/type");
        registerProperty<IntProp>("/points");
        registerProperty<VecProp<2>>("/position");
        registerProperty<FloatProp>("/rotation");
        registerProperty<FloatProp>("/innerRadius");
        registerProperty<FloatProp>("/outerRadius");
        registerProperty<FloatProp>("/innerRoundness");
        registerProperty<FloatProp>("/outerRoundness");
    }
    void extract(PolygonData &p) const override {
        getProperty<IntProp>("/direction")->get(p.direction);
        getProperty<IntProp>("/type")->get(p.type);
        getProperty<IntProp>("/points")->get(p.points);
        getProperty<VecProp<2>>("/position")->get(p.position);
        getProperty<FloatProp>("/rotation")->get(p.rotation);
        getProperty<FloatProp>("/innerRadius")->get(p.innerRadius);
        getProperty<FloatProp>("/outerRadius")->get(p.outerRadius);
        getProperty<FloatProp>("/innerRoundness")->get(p.innerRoundness);
        getProperty<FloatProp>("/outerRoundness")->get(p.outerRoundness);
    }
};

class GroupProp : public PropertyGroup_<GroupData>
{
public:
    GroupProp() {
        registerProperty<IntProp>("/blendMode");
        // Note: nested shapes will be handled differently via the shape array
    }
    void extract(GroupData &g) const override {
        getProperty<IntProp>("/blendMode")->get(g.blendMode);
        // Note: nested shapes are handled in ShapeProp
    }
};

class FillProp : public PropertyGroup_<FillData>
{
public:
    FillProp() {
        registerProperty<VecProp<3>>("/color");
        registerProperty<PercentProp>("/opacity");
        registerProperty<IntProp>("/rule");
        registerProperty<IntProp>("/blendMode");
        registerProperty<IntProp>("/compositeOrder");
    }
    void extract(FillData &f) const override {
        getProperty<VecProp<3>>("/color")->get(f.color);
        getProperty<PercentProp>("/opacity")->get(f.opacity);
        getProperty<IntProp>("/rule")->get(f.rule);
        getProperty<IntProp>("/blendMode")->get(f.blendMode);
        getProperty<IntProp>("/compositeOrder")->get(f.compositeOrder);
    }
};

class StrokeProp : public PropertyGroup_<StrokeData>
{
public:
    StrokeProp() {
        registerProperty<VecProp<3>>("/color");
        registerProperty<PercentProp>("/opacity");
        registerProperty<FloatProp>("/width");
        registerProperty<IntProp>("/lineCap");
        registerProperty<IntProp>("/lineJoin");
        registerProperty<FloatProp>("/miterLimit");
        registerProperty<IntProp>("/blendMode");
        registerProperty<IntProp>("/compositeOrder");
    }
    void extract(StrokeData &s) const override {
        getProperty<VecProp<3>>("/color")->get(s.color);
        getProperty<PercentProp>("/opacity")->get(s.opacity);
        getProperty<FloatProp>("/width")->get(s.width);
        getProperty<IntProp>("/lineCap")->get(s.lineCap);
        getProperty<IntProp>("/lineJoin")->get(s.lineJoin);
        getProperty<FloatProp>("/miterLimit")->get(s.miterLimit);
        getProperty<IntProp>("/blendMode")->get(s.blendMode);
        getProperty<IntProp>("/compositeOrder")->get(s.compositeOrder);
    }
};

// Base class for shape property groups
class ShapePropertyBase {
public:
    virtual ~ShapePropertyBase() = default;
    virtual bool hasAnimation() const = 0;
    virtual bool setFrame(int frame) = 0;
    virtual ShapeItemType getType() const = 0;
};

// Template wrapper for typed property groups
template<typename PropType, ShapeItemType TypeEnum>
class ShapePropertyWrapper : public ShapePropertyBase {
public:
    std::unique_ptr<PropType> prop;
    
    ShapePropertyWrapper() : prop(std::make_unique<PropType>()) {}
    
    bool hasAnimation() const override {
        return prop->hasAnimation();
    }
    
    bool setFrame(int frame) override {
        return prop->setFrame(frame);
    }
    
    ShapeItemType getType() const override {
        return TypeEnum;
    }
    
    PropType* operator->() { return prop.get(); }
    const PropType* operator->() const { return prop.get(); }
    PropType& operator*() { return *prop; }
    const PropType& operator*() const { return *prop; }
};

// Convenience aliases for typed wrappers
using EllipsePropertyWrapper = ShapePropertyWrapper<EllipseProp, SHAPE_ELLIPSE>;
using RectanglePropertyWrapper = ShapePropertyWrapper<RectangleProp, SHAPE_RECTANGLE>;
using PolygonPropertyWrapper = ShapePropertyWrapper<PolygonProp, SHAPE_POLYGON>;
using GroupPropertyWrapper = ShapePropertyWrapper<GroupProp, SHAPE_GROUP>;
using FillPropertyWrapper = ShapePropertyWrapper<FillProp, SHAPE_FILL>;
using StrokePropertyWrapper = ShapePropertyWrapper<StrokeProp, SHAPE_STROKE>;

// Variant-based ShapeItem with properties
struct ShapeItemWithProps {
    using PropertyVariant = std::variant<
        std::monostate,  // For SHAPE_UNKNOWN
        EllipsePropertyWrapper,
        RectanglePropertyWrapper,
        PolygonPropertyWrapper,
        GroupPropertyWrapper,
        FillPropertyWrapper,
        StrokePropertyWrapper
    >;
    
    PropertyVariant property;
    
    ShapeItemWithProps() : property(std::monostate{}) {}
    
    // Get the shape type
    ShapeItemType getType() const {
        return std::visit([](const auto& prop) -> ShapeItemType {
            using T = std::decay_t<decltype(prop)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return SHAPE_UNKNOWN;
            } else {
                return prop.getType();
            }
        }, property);
    }
    
    // Extract current values to ShapeItem
    void extractTo(ShapeItem& item) const {
        item.type = getType();
        
        std::visit([&item](const auto& prop) {
            using T = std::decay_t<decltype(prop)>;
            if constexpr (std::is_same_v<T, EllipsePropertyWrapper>) {
                prop->extract(item.ellipse);
            } else if constexpr (std::is_same_v<T, RectanglePropertyWrapper>) {
                prop->extract(item.rectangle);
            } else if constexpr (std::is_same_v<T, PolygonPropertyWrapper>) {
                prop->extract(item.polygon);
            } else if constexpr (std::is_same_v<T, GroupPropertyWrapper>) {
                prop->extract(item.group);
            } else if constexpr (std::is_same_v<T, FillPropertyWrapper>) {
                prop->extract(item.fill);
            } else if constexpr (std::is_same_v<T, StrokePropertyWrapper>) {
                prop->extract(item.stroke);
            }
            // std::monostate case - do nothing
        }, property);
    }
    
    bool hasAnimation() const {
        return std::visit([](const auto& prop) -> bool {
            using T = std::decay_t<decltype(prop)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return false;
            } else {
                return prop.hasAnimation();
            }
        }, property);
    }
    
    bool setFrame(int frame) {
        return std::visit([frame](auto& prop) -> bool {
            using T = std::decay_t<decltype(prop)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return false;
            } else {
                return prop.setFrame(frame);
            }
        }, property);
    }
    
    // Type-safe accessors
    template<typename T>
    T* get() {
        return std::get_if<T>(&property);
    }
    
    template<typename T>
    const T* get() const {
        return std::get_if<T>(&property);
    }
    
    // Check if holding specific type
    template<typename T>
    bool holds() const {
        return std::holds_alternative<T>(property);
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
        ShapeItemType type = detectItemType(json);
 auto data = json.begin().value();
 auto keyframe = keyframes.is_null() || keyframes.empty() ? ofJson{} : keyframes.begin().value();

        // Debug logging for JSON structure
        ofLogNotice("ShapeProp") << "Setting up element type: " << type;
        ofLogNotice("ShapeProp") << "JSON data: " << data.dump();
        ofLogNotice("ShapeProp") << "Keyframes: " << keyframe.dump();

        switch (type) {
            case SHAPE_ELLIPSE: {
                auto wrapper = EllipsePropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            case SHAPE_RECTANGLE: {
                auto wrapper = RectanglePropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            case SHAPE_POLYGON: {
                auto wrapper = PolygonPropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            case SHAPE_GROUP: {
                auto wrapper = GroupPropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            case SHAPE_FILL: {
                auto wrapper = FillPropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            case SHAPE_STROKE: {
                auto wrapper = StrokePropertyWrapper{};
                wrapper->setup(data, keyframe);
                element.property = std::move(wrapper);
                break;
            }
                
            default:
                element.property = std::monostate{};
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
		if(!item.is_object() || item.empty()) return SHAPE_UNKNOWN;
        
        std::string type = item.begin().key();
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
    bool setFrame(int frame) override;
    
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
    void renderPolygon(const PolygonData& polygon, float x, float y, float w, float h) const;
    void renderGroup(const GroupData& group, float x, float y, float w, float h) const;
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
