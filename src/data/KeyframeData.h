#pragma once

#include <vector>

namespace ofx { namespace ae {

class Keyframe {
public:
	enum InterpolationType {
		LINEAR,
		BEZIER,
		HOLD
	};

	struct TemporalEase {
		float speed = 0;
		float influence = 0;
	};

	struct InterpolationData {
		InterpolationType in_type = HOLD;
		InterpolationType out_type = HOLD;
		bool roving = false;
		bool continuous = false;
		TemporalEase in_ease;
		TemporalEase out_ease;
	};

	struct SpatialTangents {
		std::vector<float> in_tangent;
		std::vector<float> out_tangent;
	};

	template<typename T>
	struct Data {
		Data(){}
		Data(const T &t):value(t){}
		T value;
		InterpolationData interpolation;
		SpatialTangents spatial_tangents;
	};
};

}} // namespace ofx::ae

template<typename T> using ofxAEKeyframe = ofx::ae::Keyframe::Data<T>;