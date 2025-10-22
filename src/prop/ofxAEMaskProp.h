#pragma once

#include "ofxAEProperty.h"
#include "ofxAEShapeProp.h"
#include "ofPath.h"
#include "ofGraphicsBaseTypes.h"
#include <vector>

namespace ofx { namespace ae {

class Visitor;

struct MaskAtomData {
    PathData shape;
    glm::vec2 feather{0.f, 0.f}; // [inner, outer]
    float opacity = 1.f;
    float offset = 0.f;
    
    void accept(Visitor& visitor) const;
};

// PathShapeProp removed - using PathDataProp from ofxAEShapeProp.h instead

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
