#pragma once

#include <glm/vec2.hpp>
#include "PathData.h"
#include "Enums.h"

namespace ofx { namespace ae {

class Visitor;

struct MaskAtomData {
	PathData shape;
	glm::vec2 feather{0.f, 0.f};
	float opacity = 1.f;
	float offset = 0.f;
	bool inverted = false;
	MaskMode mode = static_cast<MaskMode>(0);
	
	void accept(Visitor &visitor) const;
};

}} // namespace ofx::ae
