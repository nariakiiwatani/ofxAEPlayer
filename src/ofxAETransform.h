#pragma once

#include "ofMatrix4x4.h"

namespace ofx { namespace ae {

struct TransformData {
	glm::vec3 anchor{0,0,0};
	glm::vec3 position{0,0,0};
	glm::vec3 scale{1,1,1};
	float rotateZ{0};
	float opacity{1};

	ofMatrix4x4 toOf() const {
		return compose(position, {0,0,rotateZ}, scale, anchor);
	}

private:

	static ofMatrix4x4 compose(const glm::vec3& pos,
						const glm::vec3& rotXYZdeg,
						const glm::vec3& scale,
						const glm::vec3& anchor) {
		auto T_anchor = ofMatrix4x4::newTranslationMatrix(-anchor);
		auto S = ofMatrix4x4::newScaleMatrix(scale);
		auto R = ofMatrix4x4::newRotationMatrix(rotXYZdeg.z, {0,0,1},
										  rotXYZdeg.y, {0,1,0},
										  rotXYZdeg.x, {1,0,0});
		auto T = ofMatrix4x4::newTranslationMatrix(pos);
		return T_anchor * S * R * T;
	}
};

}}

