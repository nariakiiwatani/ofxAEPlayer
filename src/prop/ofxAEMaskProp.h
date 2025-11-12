#pragma once

#include <vector>

#include "ofGraphicsBaseTypes.h"
#include "ofPath.h"

#include "../data/Enums.h"
#include "../data/PathData.h"
#include "../data/MaskData.h"
#include "ofxAETimeProperty.h"
#include "ofxAEShapeProp.h"

namespace ofx { namespace ae {

class Visitor;

class MaskModeProp : public TimeProperty<MaskMode> {
public:
	MaskModeProp() : TimeProperty<MaskMode>() {}

	MaskMode parse(const ofJson &json) const override {
		if(json.is_string()) {
			std::string str = json.get<std::string>();
			return maskModeFromString(str);
		}
		else {
			ofLogWarning("BlendModeProp") << "Expected string blend mode value, using NORMAL";
			return MaskMode::ADD;
		}
	}
};


class MaskAtomProp : public TimePropertyGroup
{
public:
	MaskAtomProp();
	
	void accept(Visitor &visitor) override;
};

class MaskProp : public TimePropertyArray
{
public:
	MaskProp();
	
	void setup(const ofJson &base, const ofJson &keyframes) override;
	void accept(Visitor &visitor) override;
private:
	void setupMaskAtom(const ofJson &atomBase, const ofJson &atomKeyframes);
};

}} // namespace ofx::ae
