#pragma once

#include "ofxAEProperty.h"

namespace ofx { namespace ae {

struct TransformData {
	glm::vec3 anchor;
	glm::vec3 position;
	glm::vec3 scale;
	float opacity;
};
class TransformProp : public PropertyGroup_<TransformData>
{
public:
	TransformProp() {
		registerProperty<VecProp<3>>("/anchor");
		registerProperty<VecProp<3>>("/position");
		registerProperty<PercentVecProp<3>>("/scale");
		registerProperty<PercentProp>("/opacity");
	}
	void extract(TransformData &t) const override {
		getProperty<VecProp<3>>("/anchor")->get(t.anchor);
		getProperty<VecProp<3>>("/position")->get(t.position);
		getProperty<PercentVecProp<3>>("/scale")->get(t.scale);
		getProperty<PercentProp>("/opacity")->get(t.opacity);
	}
};

}}
