#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include "ofxAEBlendMode.h"
#include "ofxAEFillRule.h"
#include "ofxAEWindingDirection.h"

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
		InterpolationType in_type=HOLD;
		InterpolationType out_type=HOLD;
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
		Data(){}
		Data(const T &t):value(t){}
		T value;
		InterpolationData interpolation;
		SpatialTangents spatial_tangents;
	};
};

namespace interpolation {

inline float cubicBezier(float t, float p1, float p2) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    return 3 * uu * t * p1 + 3 * u * tt * p2 + ttt;
}

inline float solveBezierForX(float x, float p1x, float p2x, float tolerance = 0.001f) {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    
    float t = x;
    for (int i = 0; i < 10; ++i) {
        float currentX = cubicBezier(t, p1x, p2x);
        if (std::abs(currentX - x) < tolerance) break;
        
        float derivative = 3 * (1 - t) * (1 - t) * p1x + 6 * (1 - t) * t * (p2x - p1x) + 3 * t * t * (1 - p2x);
        if (std::abs(derivative) < tolerance) break;
        
        t -= (currentX - x) / derivative;
        t = std::max(0.0f, std::min(1.0f, t));
    }
    return t;
}

template<typename T>
T linear(const T& value_a, const T& value_b, float ratio) {
    return value_a + (value_b - value_a) * ratio;
}

template<typename T>
T hold(const T& value_a, const T& value_b, float ratio) {
    return ratio >= 1.0f ? value_b : value_a;
}

template<typename T>
T bezier(const T& value_a, const T& value_b,
		 const Keyframe::TemporalEase& ease_out_a,
		 const Keyframe::TemporalEase& ease_in_b,
         float ratio) {
    if (ratio <= 0.0f) return value_a;
    if (ratio >= 1.0f) return value_b;
    
    float p1x = ease_out_a.influence / 100.0f;
    float p1y = ease_out_a.speed / 100.0f * p1x;
    float p2x = 1.0f - (ease_in_b.influence / 100.0f);
    float p2y = 1.0f - (ease_in_b.speed / 100.0f * (1.0f - p2x));
    
    float t = solveBezierForX(ratio, p1x, p2x);
    
    float bezier_ratio = cubicBezier(t, p1y, p2y);
    
    return linear(value_a, value_b, bezier_ratio);
}

template<typename T>
T ease_in(const T& value_a, const T& value_b,
		  const Keyframe::TemporalEase& ease_params,
          float ratio) {
    if (ratio <= 0.0f) return value_a;
    if (ratio >= 1.0f) return value_b;
    
    float influence = ease_params.influence / 100.0f;
    float speed = ease_params.speed / 100.0f;
    
    float p1x = influence * 0.42f; // Standard ease-in control point adjusted by influence
    float p1y = 0.0f;
    float p2x = 1.0f;
    float p2y = 1.0f;
    
    float t = solveBezierForX(ratio, p1x, p2x);
    float eased_ratio = cubicBezier(t, p1y, p2y * speed);
    
    return linear(value_a, value_b, eased_ratio);
}

template<typename T>
T ease_out(const T& value_a, const T& value_b,
		   const Keyframe::TemporalEase& ease_params,
           float ratio) {
    if (ratio <= 0.0f) return value_a;
    if (ratio >= 1.0f) return value_b;
    
    float influence = ease_params.influence / 100.0f;
    float speed = ease_params.speed / 100.0f;
    
    float p1x = 0.0f;
    float p1y = 0.0f;
    float p2x = 1.0f - (influence * 0.58f); // Standard ease-out control point adjusted by influence
    float p2y = 1.0f;
    
    float t = solveBezierForX(ratio, p1x, p2x);
    float eased_ratio = cubicBezier(t, p1y * speed, p2y);
    
    return linear(value_a, value_b, eased_ratio);
}

template<typename T>
T ease_in_out(const T& value_a, const T& value_b,
			  const Keyframe::TemporalEase& ease_in,
			  const Keyframe::TemporalEase& ease_out,
              float ratio) {
    if (ratio <= 0.0f) return value_a;
    if (ratio >= 1.0f) return value_b;
    
    float influence_in = ease_in.influence / 100.0f;
    float speed_in = ease_in.speed / 100.0f;
    float influence_out = ease_out.influence / 100.0f;
    float speed_out = ease_out.speed / 100.0f;
    
    float p1x = influence_in * 0.42f;
    float p1y = 0.0f;
    float p2x = 1.0f - (influence_out * 0.58f);
    float p2y = 1.0f;
    
    float t = solveBezierForX(ratio, p1x, p2x);
    float eased_ratio = cubicBezier(t, p1y * speed_in, p2y * speed_out);
    
    return linear(value_a, value_b, eased_ratio);
}

template<typename T>
T calculate(const Keyframe::Data<T>& keyframe_a,
			const Keyframe::Data<T>& keyframe_b,
           float ratio) {
	Keyframe::InterpolationType interp_type = keyframe_a.interpolation.out_type;
    
    switch (interp_type) {
		case Keyframe::LINEAR:
            return linear(keyframe_a.value, keyframe_b.value, ratio);
            
		case Keyframe::HOLD:
            return hold(keyframe_a.value, keyframe_b.value, ratio);
            
		case Keyframe::BEZIER:
            return bezier(keyframe_a.value, keyframe_b.value,
                         keyframe_a.interpolation.out_ease,
                         keyframe_b.interpolation.in_ease, ratio);
            
		case Keyframe::EASE_IN:
            return ease_in(keyframe_a.value, keyframe_b.value,
                          keyframe_a.interpolation.out_ease, ratio);
            
		case Keyframe::EASE_OUT:
            return ease_out(keyframe_a.value, keyframe_b.value,
                           keyframe_b.interpolation.in_ease, ratio);
            
		case Keyframe::EASE_IN_OUT:
            return ease_in_out(keyframe_a.value, keyframe_b.value,
                              keyframe_b.interpolation.in_ease,
                              keyframe_a.interpolation.out_ease, ratio);
            
        default:
            return linear(keyframe_a.value, keyframe_b.value, ratio);
    }
}

template<>
inline BlendMode calculate(const Keyframe::Data<BlendMode>& keyframe_a,
			const Keyframe::Data<BlendMode>& keyframe_b,
			float ratio) {
	return ratio < 0.5f ? keyframe_a.value : keyframe_b.value;
}
template<>
inline FillRule calculate(const Keyframe::Data<FillRule>& keyframe_a,
			const Keyframe::Data<FillRule>& keyframe_b,
			float ratio) {
	return ratio < 0.5f ? keyframe_a.value : keyframe_b.value;
}
template<>
inline WindingDirection calculate(const Keyframe::Data<WindingDirection>& keyframe_a,
			const Keyframe::Data<WindingDirection>& keyframe_b,
			float ratio) {
	return ratio < 0.5f ? keyframe_a.value : keyframe_b.value;
}

} // namespace interpolation

namespace util {

template<typename T>
struct KeyframePair {
    const Keyframe::Data<T>* keyframe_a = nullptr;
    const Keyframe::Data<T>* keyframe_b = nullptr;
    float ratio = 0.0f;
    int frame_a = 0;
    int frame_b = 0;
};

template<typename T>
KeyframePair<T> findKeyframePair(const std::map<int, Keyframe::Data<T>>& keyframes, int frame) {
    KeyframePair<T> result;
    
    if (keyframes.empty()) {
        return result;
    }
    
    if (keyframes.size() == 1) {
        auto it = keyframes.begin();
        result.keyframe_a = &it->second;
        result.keyframe_b = &it->second;
        result.frame_a = it->first;
        result.frame_b = it->first;
        result.ratio = 0.0f;
        return result;
    }
    
    auto upper = keyframes.upper_bound(frame);
    
    if (upper == keyframes.begin()) {
        auto first = keyframes.begin();
        result.keyframe_a = &first->second;
        result.keyframe_b = &first->second;
        result.frame_a = first->first;
        result.frame_b = first->first;
		result.ratio = 0.0f;
        return result;
    }
    
    if (upper == keyframes.end()) {
        auto last = std::prev(keyframes.end());
        result.keyframe_a = &last->second;
        result.keyframe_b = &last->second;
        result.frame_a = last->first;
        result.frame_b = last->first;
		result.ratio = 0.0f;
        return result;
    }
    
    auto keyframe_b_it = upper;
    auto keyframe_a_it = std::prev(upper);
    
    result.keyframe_a = &keyframe_a_it->second;
    result.keyframe_b = &keyframe_b_it->second;
    result.frame_a = keyframe_a_it->first;
    result.frame_b = keyframe_b_it->first;
    
    if (result.frame_b != result.frame_a) {
        result.ratio = static_cast<float>(frame - result.frame_a) /
                      static_cast<float>(result.frame_b - result.frame_a);
    } else {
        result.ratio = 0.0f;
    }
    
    return result;
}

template<typename T>
T interpolateKeyframe(const Keyframe::Data<T>& keyframe_a,
                     const Keyframe::Data<T>& keyframe_b,
                     float ratio) {
    return interpolation::calculate(keyframe_a, keyframe_b, ratio);
}

} // namespace util

}} // namespace ofx::ae

template<typename T> using ofxAEKeyframe = ofx::ae::Keyframe::Data<T>;
