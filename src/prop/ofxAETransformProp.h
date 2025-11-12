#pragma once

#include "ofxAETimeProperty.h"
#include "../data/TransformData.h"

namespace ofx { namespace ae {

class TransformProp : public TimePropertyGroup
{
public:
	TransformProp() {
		registerProperty<VecTimeProp<3>>("/anchor");
		registerProperty<VecTimeProp<3>>("/position");
		registerProperty<PercentVecTimeProp<3>>("/scale");
		registerProperty<FloatTimeProp>("/rotateZ");
		registerProperty<PercentTimeProp>("/opacity");

		registerExtractor<TransformData>([this](TransformData &t) -> bool {
			bool success = true;

			if(!getProperty<VecTimeProp<3>>("/anchor")->tryExtract(t.anchor)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract anchor, using default";
				t.anchor = glm::vec3(0.0f, 0.0f, 0.0f);
				success = false;
			}

			if(!getProperty<VecTimeProp<3>>("/position")->tryExtract(t.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract position, using default";
				t.position = glm::vec3(0.0f, 0.0f, 0.0f);
				success = false;
			}

			if(!getProperty<PercentVecTimeProp<3>>("/scale")->tryExtract(t.scale)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract scale, using default";
				t.scale = glm::vec3(1.0f, 1.0f, 1.0f);
				success = false;
			}

			if(!getProperty<FloatTimeProp>("/rotateZ")->tryExtract(t.rotateZ)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rotateZ, using default";
				t.rotateZ = 0.0f;
				success = false;
			}

			if(!getProperty<PercentTimeProp>("/opacity")->tryExtract(t.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract opacity, using default";
				t.opacity = 1.0f;
				success = false;
			}

			return success;
		});
	}
};

}}
