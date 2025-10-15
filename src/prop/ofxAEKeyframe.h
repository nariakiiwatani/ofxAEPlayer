#pragma once

#include <vector>

namespace ofx { namespace ae {

class Keyframe {
public:
	enum InterpolationType {
		LINEAR,
		BEZIER,
		HOLD,
		EASE_IN,
		EASE_OUT,
		EASE_IN_OUT,
		CUBIC,
		HERMITE,
		CATMULL_ROM
	};

	struct TemporalEase {
		float speed=0;
		float influence=0;
	};

	struct InterpolationData {
		InterpolationType in_type=LINEAR;
		InterpolationType out_type=LINEAR;
		bool roving=false;
		bool continuous=false;
		TemporalEase in_ease;
		TemporalEase out_ease;
	};

	struct SpatialTangents {
		std::vector<float> in_tangent;
		std::vector<float> out_tangent;
	};

	template<typename T>
	struct Data {
		T value;
		InterpolationData interpolation;
		SpatialTangents spatial_tangents;
	};
};

}}

template<typename T> using ofxAEKeyframe = ofx::ae::Keyframe::Data<T>;
