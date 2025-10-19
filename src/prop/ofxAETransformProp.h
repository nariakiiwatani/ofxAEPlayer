#pragma once

#include "ofxAEProperty.h"
#include "ofMatrix4x4.h"

namespace ofx { namespace ae {

struct TransformData {
	glm::vec3 anchor{0,0,0};
	glm::vec3 position{0,0,0};
	glm::vec3 scale{1,1,1};
	float rotateZ{0};
	float skew{0};
	float skewAxis{0};
	float opacity{1};

	ofMatrix4x4 toOf() const {
		return compose(position, {0,0,rotateZ}, scale, skew, skewAxis, anchor);
	}

private:
	static ofMatrix4x4 newSkewMatrix(float skewDeg, float axisDeg) {
		float t = tan(ofDegToRad(skewDeg));
		float phi = ofDegToRad(axisDeg);
		float ax = cos(phi), ay = sin(phi);
		float nx = -sin(phi), ny = cos(phi);

		ofMatrix4x4 K;
		K.makeIdentityMatrix();
		K(0,0) += t * ax * nx;
		K(0,1) += t * ax * ny;
		K(1,0) += t * ay * nx;
		K(1,1) += t * ay * ny;
		return K;
	}

	static ofMatrix4x4 compose(const glm::vec3& pos,
						const glm::vec3& rotXYZdeg,
						const glm::vec3& scale,
						float skewDeg, float axisDeg,
						const glm::vec3& anchor) {
		auto T_anchor = ofMatrix4x4::newTranslationMatrix(-anchor);
		auto S = ofMatrix4x4::newScaleMatrix(scale);
		auto K = newSkewMatrix(skewDeg, axisDeg);
		auto R = ofMatrix4x4::newRotationMatrix(rotXYZdeg.z, {0,0,1},
										  rotXYZdeg.y, {0,1,0},
										  rotXYZdeg.x, {1,0,0});
		auto T = ofMatrix4x4::newTranslationMatrix(pos);
		return T_anchor * S * K * R * T;
	}
};
class TransformProp : public PropertyGroup
{
public:
	TransformProp() {
		registerProperty<VecProp<3>>("/anchor");
		registerProperty<VecProp<3>>("/position");
		registerProperty<PercentVecProp<3>>("/scale");
		registerProperty<FloatProp>("/rotateZ");
		registerProperty<FloatProp>("/skew");
		registerProperty<FloatProp>("/skewAxis");
		registerProperty<PercentProp>("/opacity");

		registerExtractor<TransformData>([this](TransformData& t) -> bool {
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

			if (!getProperty<FloatProp>("/skew")->tryExtract(t.skew)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract skew, using default";
				t.skew = 0.0f;
				success = false;
			}

			if (!getProperty<FloatProp>("/skewAxis")->tryExtract(t.skewAxis)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract skewAxis, using default";
				t.skewAxis = 0.0f;
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
