#pragma once

#include <vector>

#include "ofGraphicsBaseTypes.h"
#include "ofPath.h"

#include "../core/ofxAEMask.h"
#include "../data/ofxAEPath.h"
#include "ofxAEProperty.h"
#include "ofxAEShapeProp.h"

namespace ofx { namespace ae {

class Visitor;

class MaskModeProp : public Property<MaskMode> {
public:
	MaskModeProp() : Property<MaskMode>() {}

	MaskMode parse(const ofJson &json) const override {
		if(json.is_string()) {
			std::string str = json.get<std::string>();
			return maskModeFromString(str);
		} else {
			ofLogWarning("BlendModeProp") << "Expected string blend mode value, using NORMAL";
			return MaskMode::ADD;
		}
	}
};

struct MaskAtomData {
    PathData shape;
    glm::vec2 feather{0.f, 0.f};
    float opacity = 1.f;
    float offset = 0.f;
    bool inverted = false;
    MaskMode mode = static_cast<MaskMode>(0);
    
    void accept(Visitor &visitor) const;
};


class MaskAtomProp : public PropertyGroup
{
public:
    MaskAtomProp();
    
    void accept(Visitor &visitor) override;
};

class MaskProp : public PropertyArray
{
public:
    MaskProp();
    
    void setup(const ofJson &base, const ofJson &keyframes) override;
    void accept(Visitor &visitor) override;
private:
    void setupMaskAtom(const ofJson &atomBase, const ofJson &atomKeyframes);
};

}} // namespace ofx::ae
