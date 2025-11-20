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

namespace {
	constexpr int ARC_LENGTH_SEGMENTS = 200;
	constexpr int ARC_LENGTH_BINARY_SEARCH_SEGMENTS = 30;
	constexpr int MAX_BINARY_SEARCH_ITERATIONS = 6;
	constexpr float ARC_LENGTH_TOLERANCE = 0.01f;
}

template<typename T>
T linear(const T &value_a, const T &value_b, float ratio) {
	return value_a + (value_b - value_a) * ratio;
}

template<typename T>
T hold(const T &value_a, const T &value_b, float ratio) {
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
		float f  = bez3(t,p1x,p2x) - x;
		float df = 3*(1-t)*(1-t)*p1x
				 + 6*(1-t)*t*(p2x-p1x)
				 + 3*t*t*(1-p2x);
		if(std::fabs(df) < 1e-6f) break;
		t = std::min(std::max(t - f/df, 0.f), 1.f);
	}
	return t;
}


template<typename T>
inline T normalize(const T &a) {
	return glm::normalize(a);
}

template<>
inline bool normalize(const bool &v) {
	return v;
}

template<>
inline float normalize(const float &dv) {
	if(dv == 0.f) return 0.f;
	return dv > 0.f ? 1.f : -1.f;
}

template<>
inline int normalize(const int &dv) {
	if(dv == 0) return 0;
	return dv > 0 ? 1 : -1;
}

template<>
inline ofFloatColor normalize(const ofFloatColor &a) {
	glm::vec4 v = glm::normalize(glm::vec4{a.r,a.g,a.b,a.a});
	return ofFloatColor{v.x,v.y,v.z,v.w};
}

template<typename T>
inline T bezier(const T &value_a, const T &value_b,
	 const Keyframe::TemporalEase &ease_out_a,
	 const Keyframe::TemporalEase &ease_in_b,
	 float dt, float ratio)
{
	if(ratio<=0.f) return value_a;
	if(ratio>=1.f) return value_b;
	if(dt <= 0.f) return value_a;

	const float p1x = ease_out_a.influence;
	const float p2x = 1.f - ease_in_b.influence;

	const T p0y = value_a;
	const T p3y = value_b;

	const T dv  = value_b - value_a;
	const T dir = normalize(dv);

	const T p1y = value_a + dir * (ease_out_a.speed * dt * ease_out_a.influence);
	const T p2y = value_b - dir * (ease_in_b.speed * dt * ease_in_b.influence);

	const float s = solveForX(ratio, p1x, p2x);

	const float u = 1.f - s;
	const T y = p0y*u*u*u
			  + p1y*3*u*u*s
			  + p2y*3*u*s*s
			  + p3y*s*s*s;
	return y;
}

template<typename T>
inline T evaluateBezier(const T &p0, const T &p1, const T &p2, const T &p3, float t) {
	const float u = 1.f - t;
	return p0*u*u*u + p1*3*u*u*t + p2*3*u*t*t + p3*t*t*t;
}

template<typename T>
inline T evaluateBezierDerivative(const T &p0, const T &p1, const T &p2, const T &p3, float t) {
	const float u = 1.f - t;
	return
		3.f*u*u*(p1 - p0) +
		6.f*u*t*(p2 - p1) +
		3.f*t*t*(p3 - p2);
}

template<typename T>
inline float arcLengthUpTo(const T &p0, const T &p1, const T &p2, const T &p3,
						   float t, int segments = ARC_LENGTH_SEGMENTS) {
	if(t <= 0.f) return 0.f;
	if(t >= 1.f) t = 1.f;

	float total_length = 0.f;
	T prev_point = p0;

	for(int i = 1; i <= segments; ++i) {
		const float ti = t * static_cast<float>(i) / static_cast<float>(segments);
		const T current_point = evaluateBezier(p0, p1, p2, p3, ti);
		total_length += glm::length(current_point - prev_point);
		prev_point = current_point;
	}
	return total_length;
}

template<typename T>
inline float calculateArcLength(const T &p0, const T &p1, const T &p2, const T &p3) {
	return arcLengthUpTo(p0, p1, p2, p3, 1.f, ARC_LENGTH_SEGMENTS);
}

template<typename T>
inline float findParameterForArcLength(const T &p0, const T &p1, const T &p2, const T &p3,
									   float targetLength, float totalLength) {
	if(targetLength <= 0.f) return 0.f;
	if(targetLength >= totalLength) return 1.f;

	float t = targetLength / totalLength;
	t = std::min(std::max(t, 0.f), 1.f);

	for(int iter = 0; iter < MAX_BINARY_SEARCH_ITERATIONS; ++iter) {
		const float s = arcLengthUpTo(p0, p1, p2, p3, t, ARC_LENGTH_BINARY_SEARCH_SEGMENTS);
		const float diff = s - targetLength;
		if(std::fabs(diff) < ARC_LENGTH_TOLERANCE) {
			break;
		}

		const T d = evaluateBezierDerivative(p0, p1, p2, p3, t);
		const float speed = glm::length(d);
		if(speed < 1e-6f) {
			break;
		}

		t -= diff / speed;
		if(t <= 0.f) { t = 0.f; break; }
		if(t >= 1.f) { t = 1.f; break; }
	}

	return t;
}


template<typename T>
inline T spatialBezierLinearTime(const T &value_a, const T &value_b,
		  const T &out_tangent_a, const T &in_tangent_b,
		  float ratio)
{
	if(ratio<=0.f) return value_a;
	if(ratio>=1.f) return value_b;

	const T p0 = value_a;
	const T p1 = value_a + out_tangent_a;
	const T p2 = value_b + in_tangent_b;
	const T p3 = value_b;

	const float totalArcLength = calculateArcLength(p0, p1, p2, p3);
	const float targetLength  = ratio * totalArcLength;
	const float t = findParameterForArcLength(p0, p1, p2, p3, targetLength, totalArcLength);
	return evaluateBezier(p0, p1, p2, p3, t);
}

template<typename T>
inline T bezierWithSpatialTangents(const T &value_a, const T &value_b,
		   const T &out_tangent_a, const T &in_tangent_b,
		   const Keyframe::TemporalEase &ease_out_a,
		   const Keyframe::TemporalEase &ease_in_b,
		   float dt, float ratio)
{
	if(ratio<=0.f) return value_a;
	if(ratio>=1.f) return value_b;
	if(dt <= 0.f) return value_a;

	const float p1x = ease_out_a.influence;
	const float p2x = 1.f - ease_in_b.influence;

	const float s = solveForX(ratio, p1x, p2x);

	const T p0 = value_a;
	const T p1 = value_a + out_tangent_a;
	const T p2 = value_b + in_tangent_b;
	const T p3 = value_b;

	const float u = 1.f - s;
	const T y = p0*u*u*u
			  + p1*3*u*u*s
			  + p2*3*u*s*s
			  + p3*s*s*s;
	return y;
}

inline float temporalEase01(const Keyframe::TemporalEase &ease_out_a,
							const Keyframe::TemporalEase &ease_in_b,
							float ratio)
{
	if(ratio <= 0.f) return 0.f;
	if(ratio >= 1.f) return 1.f;

	const float p1x = ease_out_a.influence;
	const float p2x = 1.f - ease_in_b.influence;

	const float s = solveForX(ratio, p1x, p2x);
	return s;
}

template<>
inline PathData bezier<PathData>(const PathData &va, const PathData &vb,
								 const Keyframe::TemporalEase &ease_out_a,
								 const Keyframe::TemporalEase &ease_in_b,
								 float /*dt*/, float ratio)
{
	PathData ret = va;
	if(va.vertices.size()!=vb.vertices.size()
	 || va.inTangents.size()!=vb.inTangents.size()
	 || va.outTangents.size()!=vb.outTangents.size()) {
		return va;
	}

	const float eased = temporalEase01(ease_out_a, ease_in_b, ratio);

	auto lerp = [&](const glm::vec2 &a, const glm::vec2 &b){
		return a + (b - a) * eased;
	};

	ret.vertices.resize(va.vertices.size());
	ret.inTangents.resize(va.inTangents.size());
	ret.outTangents.resize(va.outTangents.size());

	for(size_t i=0;i<ret.vertices.size();++i)
		ret.vertices[i] = lerp(va.vertices[i], vb.vertices[i]);
	for(size_t i=0;i<ret.inTangents.size();++i)
		ret.inTangents[i] = lerp(va.inTangents[i], vb.inTangents[i]);
	for(size_t i=0;i<ret.outTangents.size();++i)
		ret.outTangents[i] = lerp(va.outTangents[i], vb.outTangents[i]);

	ret.closed	 = vb.closed;
	ret.direction = vb.direction;
	return ret;
}

template<typename T>
bool hasSpatialTangents(const Keyframe::Data<T> &/*kf*/) {
	return false;
}

template<int N, typename T>
bool hasSpatialTangents(const Keyframe::Data<glm::vec<N,T>> &kf) {
	const auto &st_out = kf.spatial_tangents.out_tangent;
	const auto &st_in  = kf.spatial_tangents.in_tangent;
	return st_out.size() >= 2 && st_in.size() >= 2;
}

template<typename T>
T calculate(const Keyframe::Data<T> &keyframe_a,
			const Keyframe::Data<T> &keyframe_b,
			float dt, float ratio) {
	Keyframe::InterpolationType interp_type = keyframe_a.interpolation.out_type;

	switch(interp_type) {
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

template<int N, typename T>
glm::vec<N,T> calculate(const Keyframe::Data<glm::vec<N,T>> &keyframe_a,
						const Keyframe::Data<glm::vec<N,T>> &keyframe_b,
						float dt, float ratio) {
	if(hasSpatialTangents(keyframe_a) && hasSpatialTangents(keyframe_b)) {
		glm::vec<N,T> out_tangent{};
		glm::vec<N,T> in_tangent{};

		for(int i = 0; i < std::min(N, 2); ++i) {
			if(i < (int)keyframe_a.spatial_tangents.out_tangent.size())
				out_tangent[i] = static_cast<T>(keyframe_a.spatial_tangents.out_tangent[i]);
			if(i < (int)keyframe_b.spatial_tangents.in_tangent.size())
				in_tangent[i] = static_cast<T>(keyframe_b.spatial_tangents.in_tangent[i]);
		}

		Keyframe::InterpolationType interp_type = keyframe_a.interpolation.out_type;

		if(interp_type == Keyframe::LINEAR) {
			return spatialBezierLinearTime(keyframe_a.value, keyframe_b.value,
										   out_tangent, in_tangent, ratio);
		}
		else if(interp_type == Keyframe::BEZIER) {
			return bezierWithSpatialTangents(keyframe_a.value, keyframe_b.value,
											 out_tangent, in_tangent,
											 keyframe_a.interpolation.out_ease,
											 keyframe_b.interpolation.in_ease,
											 dt, ratio);
		}
	}

	Keyframe::InterpolationType interp_type = keyframe_a.interpolation.out_type;

	switch(interp_type) {
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
inline BlendMode calculate(const Keyframe::Data<BlendMode> &keyframe_a,
						   const Keyframe::Data<BlendMode> &keyframe_b,
						   float /*dt*/, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}

template<>
inline FillRule calculate(const Keyframe::Data<FillRule> &keyframe_a,
						  const Keyframe::Data<FillRule> &keyframe_b,
						  float /*dt*/, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}

template<>
inline WindingDirection calculate(const Keyframe::Data<WindingDirection> &keyframe_a,
								  const Keyframe::Data<WindingDirection> &keyframe_b,
								  float /*dt*/, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}

template<>
inline MaskMode calculate(const Keyframe::Data<MaskMode> &keyframe_a,
						  const Keyframe::Data<MaskMode> &keyframe_b,
						  float /*dt*/, float ratio) {
	return ratio < 1.f ? keyframe_a.value : keyframe_b.value;
}

} // namespace interpolation

template<typename T>
T interpolateKeyframe(const Keyframe::Data<T> &keyframe_a,
					  const Keyframe::Data<T> &keyframe_b,
					  float dt, float ratio) {
	return interpolation::calculate(keyframe_a, keyframe_b, dt, ratio);
}

}} // namespace ofx::ae
