#pragma once

#include "ofxAEProperty.h"
#include "../data/TransformData.h"

namespace ofx { namespace ae {
class TransformProp : public PropertyGroup
{
public:
	TransformProp() {
		registerProperty<VecProp<3>>("/anchor");
		registerProperty<VecProp<3>>("/position");
		registerProperty<PercentVecProp<3>>("/scale");
		registerProperty<FloatProp>("/rotateZ");
		registerProperty<PercentProp>("/opacity");

		registerExtractor<TransformData>([this](TransformData &t) -> bool {
			bool success = true;

			if (!getProperty<VecProp<3>>("/anchor")->tryExtract(t.anchor)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract anchor, using default";
				t.anchor = glm::vec3(0.0f, 0.0f, 0.0f);
				success = false;
			}

			if (!getProperty<VecProp<3>>("/position")->tryExtract(t.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract position, using default";
				t.position = glm::vec3(0.0f, 0.0f, 0.0f);
				success = false;
			}

			if (!getProperty<PercentVecProp<3>>("/scale")->tryExtract(t.scale)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract scale, using default";
				t.scale = glm::vec3(1.0f, 1.0f, 1.0f);
				success = false;
			}

			if (!getProperty<FloatProp>("/rotateZ")->tryExtract(t.rotateZ)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rotateZ, using default";
				t.rotateZ = 0.0f;
				success = false;
			}

			if (!getProperty<PercentProp>("/opacity")->tryExtract(t.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract opacity, using default";
				t.opacity = 1.0f;
				success = false;
			}

			return success;
		});
	}
};

}}
