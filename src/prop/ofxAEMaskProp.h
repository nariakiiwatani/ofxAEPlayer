#pragma once

#include "ofxAEProperty.h"
#include "ofPath.h"
#include "ofGraphicsBaseTypes.h"
#include <vector>

namespace ofx { namespace ae {

class Visitor;

struct MaskShapeData {
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> inTangents;
    std::vector<glm::vec2> outTangents;
    bool closed = false;

    MaskShapeData operator+(const MaskShapeData& other) const;
    MaskShapeData operator-(const MaskShapeData& other) const;
    MaskShapeData operator*(float ratio) const;
    
    bool isEmpty() const { return vertices.empty(); }
    size_t getVertexCount() const { return vertices.size(); }
    
    ofPath toOfPath() const;
    
    static MaskShapeData lerp(const MaskShapeData& a, const MaskShapeData& b, float t);
};

struct MaskAtomData {
    MaskShapeData shape;
    glm::vec2 feather{0.f, 0.f}; // [inner, outer]
    float opacity = 1.f;
    float offset = 0.f;
    
    void accept(Visitor& visitor) const;
};

class PathShapeProp : public Property<MaskShapeData>
{
public:
    PathShapeProp() : Property<MaskShapeData>() {}
    
    MaskShapeData parse(const ofJson &json) const override;
};

class MaskAtomProp : public PropertyGroup
{
public:
    MaskAtomProp();
    
    void accept(Visitor& visitor) override;
};

class MaskProp : public PropertyArray
{
public:
    MaskProp();
    
    void setup(const ofJson &base, const ofJson &keyframes) override;
    void accept(Visitor& visitor) override;
private:
    void setupMaskAtom(const ofJson &atomBase, const ofJson &atomKeyframes);
};

}} // namespace ofx::ae
