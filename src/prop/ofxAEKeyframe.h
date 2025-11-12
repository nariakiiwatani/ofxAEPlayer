#pragma once

#include <cmath>
#include <algorithm>
#include <map>
#include "data/PathData.h"
#include "data/Enums.h"
#include "utils/ofxAEBlendMode.h"
#include "data/KeyframeData.h"

namespace ofx { namespace ae {

namespace interpolation {

template<typename T>
T linear(const T& value_a, const T& value_b, float ratio) {
	return value_a + (value_b - value_a) * ratio;
}

template<typename T>
T hold(const T& value_a, const T& value_b, float ratio) {
	return ratio >= 1.0f ? value_b : value_a;
}

inline float bez3(float t, float p1, float p2){
	float u=1.f-t, tt=t*t, uu=u*u;
	return 3*uu*t*p1 + 3*u*tt*p2 + tt*t;
}
inline float solveForX(float x, float p1x, float p2x){
	x = std::min(std::max(x,0.f),1.f);
	float t = x;
	for(int i=0;i<8;++i){
		float f = bez3(t,p1x,p2x) - x;
		float df = 3*(1-t)*(1-t)*p1x + 6*(1-t)*t*(p2x-p1x) + 3*t*t*(1-p2x);
		if (std::fabs(df) < 1e-6f) break;
		t = std::min(std::max(t - f/df, 0.f), 1.f);
	}
	return t;
}

template<typename T>
inline T normalize(const T &a) {
	return glm::normalize(a);
}

template<>
inline float normalize(const float&) { return 1; }
template<>
inline int normalize(const int&) { return 1; }

template<>
inline ofFloatColor normalize(const ofFloatColor &a) {
	glm::vec4 v = glm::normalize(glm::vec4{a.r,a.g,a.b,a.a});
	return ofFloatColor{v.x,v.y,v.z,v.w};
}

template<typename T>
inline T bezier(const T& value_a, const T& value_b,
		 const Keyframe::TemporalEase& ease_out_a,
		 const Keyframe::TemporalEase& ease_in_b,
		 float dt, float ratio)
{
	if (ratio<=0.f) return value_a;
	if (ratio>=1.f) return value_b;

	const float p1x = ease_out_a.influence;
	const float p2x = 1.f - ease_in_b.influence;

	const T norm = normalize(value_b - value_a);
	const T p1y = value_a + norm*(ease_out_a.speed * dt * ease_out_a.influence);
	const T p2y = value_b - norm*(ease_in_b.speed * dt * ease_in_b.influence);

	const float t = solveForX(ratio, p1x, p2x);

	const float u = 1.f - t;
	const T y = value_a*u*u*u + p1y*3*u*u*t + p2y*3*u*t*t + value_b*t*t*t;
	return y;
}

template<>
inline PathData bezier<PathData>(const PathData& va, const PathData& vb,
								 const Keyframe::TemporalEase& ease_out_a,
								 const Keyframe::TemporalEase& ease_in_b,
								 float dt, float ratio)
{
	PathData ret = va;
	if (va.vertices.size()!=vb.vertices.size()
	 || va.inTangents.size()!=vb.inTangents.size()
	 || va.outTangents.size()!=vb.outTangents.size()){
		return va;
	}

	const float eased = bezier(0.f, 1.f, ease_out_a, ease_in_b, dt, ratio);

	auto lerp = [&](const glm::vec2& a, const glm::vec2& b){
		return a + (b - a) * eased;
	};

	ret.vertices.resize(va.vertices.size());
	ret.inTangents.resize(va.inTangents.size());
	ret.outTangents.resize(va.outTangents.size());

	for (size_t i=0;i<ret.vertices.size();++i)
		ret.vertices[i] = lerp(va.vertices[i], vb.vertices[i]);
	for (size_t i=0;i<ret.inTangents.size();++i)
		ret.inTangents[i] = lerp(va.inTangents[i], vb.inTangents[i]);
	for (size_t i=0;i<ret.outTangents.size();++i)
		ret.outTangents[i] = lerp(va.outTangents[i], vb.outTangents[i]);

	ret.closed	= vb.closed;
	ret.direction = vb.direction;
	return ret;
}
template<typename T>
T calculate(const Keyframe::Data<T>& keyframe_a,
			const Keyframe::Data<T>& keyframe_b,
			float dt, float ratio) {
	Keyframe::InterpolationType interp_type = keyframe_a.interpolation.out_type;
	
	switch (interp_type) {
		case Keyframe::LINEAR:
			return linear(keyframe_a.value, keyframe_b.value, ratio);
			
		case Keyframe::HOLD:
			return hold(keyframe_a.value, keyframe_b.value, ratio);
			
		case Keyframe::BEZIER:
			return bezier(keyframe_a.value, keyframe_b.value,
						 keyframe_a.interpolation.out_ease,
						 keyframe_b.interpolation.in_ease, dt, ratio);

		default:
			return linear(keyframe_a.value, keyframe_b.value, ratio);
	}
}

template<>
inline BlendMode calculate(const Keyframe::Data<BlendMode>& keyframe_a,
			const Keyframe::Data<BlendMode>& keyframe_b,
			float dt, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}
template<>
inline FillRule calculate(const Keyframe::Data<FillRule>& keyframe_a,
			const Keyframe::Data<FillRule>& keyframe_b,
						  float dt, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}
template<>
inline WindingDirection calculate(const Keyframe::Data<WindingDirection>& keyframe_a,
			const Keyframe::Data<WindingDirection>& keyframe_b,
								  float dt, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}
template<>
inline MaskMode calculate(const Keyframe::Data<MaskMode>& keyframe_a,
			const Keyframe::Data<MaskMode>& keyframe_b,
						  float dt, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}

} // namespace interpolation

template<typename T>
T interpolateKeyframe(const Keyframe::Data<T>& keyframe_a,
					 const Keyframe::Data<T>& keyframe_b,
					 float dt, float ratio) {
	return interpolation::calculate(keyframe_a, keyframe_b, dt, ratio);
}

}} // namespace ofx::ae
