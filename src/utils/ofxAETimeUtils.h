#pragma once

#include <cmath>
#include <map>
#include "../data/KeyframeData.h"

namespace ofx { namespace ae {

using Frame = float;
using FrameCount = float;

}} // namespace ofx::ae

namespace ofx { namespace ae { namespace util {

constexpr float FRAME_EPSILON = 1e-4f;

inline bool isNearFrame(Frame a, Frame b) {
	return std::abs(a - b) < FRAME_EPSILON;
}

inline bool isNearTime(double a, double b) {
	constexpr double TIME_EPSILON = 1e-6;
	return std::abs(a - b) < TIME_EPSILON;
}

inline Frame timeToFrame(double time, float fps) {
	return static_cast<Frame>(time * fps);
}

inline double frameToTime(Frame frame, float fps) {
	return static_cast<double>(frame) / fps;
}

template<typename T>
struct FrameKeyframePair {
	const Keyframe::Data<T>* keyframe_a = nullptr;
	const Keyframe::Data<T>* keyframe_b = nullptr;
	float ratio = 0.0f;
	Frame frame_a = 0.0f;
	Frame frame_b = 0.0f;
};

template<typename T>
struct TimeKeyframePair {
	const Keyframe::Data<T>* keyframe_a = nullptr;
	const Keyframe::Data<T>* keyframe_b = nullptr;
	float ratio = 0.0f;
	double time_a = 0.0;
	double time_b = 0.0;
};

template<typename T>
FrameKeyframePair<T> findFrameKeyframePair(const std::map<Frame, Keyframe::Data<T>>& keyframes, Frame frame) {
	FrameKeyframePair<T> result;
	
	if(keyframes.empty()) {
		return result;
	}
	
	if(keyframes.size() == 1) {
		auto it = keyframes.begin();
		result.keyframe_a = &it->second;
		result.keyframe_b = &it->second;
		result.frame_a = it->first;
		result.frame_b = it->first;
		result.ratio = 0.0f;
		return result;
	}
	
	auto upper = keyframes.upper_bound(frame);
	
	if(upper == keyframes.begin()) {
		auto first = keyframes.begin();
		result.keyframe_a = &first->second;
		result.keyframe_b = &first->second;
		result.frame_a = first->first;
		result.frame_b = first->first;
		result.ratio = 0.0f;
		return result;
	}
	
	if(upper == keyframes.end()) {
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
	
	if(!isNearFrame(result.frame_b, result.frame_a)) {
		result.ratio = static_cast<float>((frame - result.frame_a) / (result.frame_b - result.frame_a));
	}
	else {
		result.ratio = 0.0f;
	}
	
	return result;
}

template<typename T>
TimeKeyframePair<T> findTimeKeyframePair(const std::map<double, Keyframe::Data<T>>& keyframes, double time) {
	TimeKeyframePair<T> result;
	
	if(keyframes.empty()) {
		return result;
	}
	
	if(keyframes.size() == 1) {
		auto it = keyframes.begin();
		result.keyframe_a = &it->second;
		result.keyframe_b = &it->second;
		result.time_a = it->first;
		result.time_b = it->first;
		result.ratio = 0.0f;
		return result;
	}
	
	auto upper = keyframes.upper_bound(time);
	
	if(upper == keyframes.begin()) {
		auto first = keyframes.begin();
		result.keyframe_a = &first->second;
		result.keyframe_b = &first->second;
		result.time_a = first->first;
		result.time_b = first->first;
		result.ratio = 0.0f;
		return result;
	}
	
	if(upper == keyframes.end()) {
		auto last = std::prev(keyframes.end());
		result.keyframe_a = &last->second;
		result.keyframe_b = &last->second;
		result.time_a = last->first;
		result.time_b = last->first;
		result.ratio = 0.0f;
		return result;
	}
	
	auto keyframe_b_it = upper;
	auto keyframe_a_it = std::prev(upper);
	
	result.keyframe_a = &keyframe_a_it->second;
	result.keyframe_b = &keyframe_b_it->second;
	result.time_a = keyframe_a_it->first;
	result.time_b = keyframe_b_it->first;
	
	constexpr double TIME_EPSILON = 1e-6;
	if(std::abs(result.time_b - result.time_a) > TIME_EPSILON) {
		result.ratio = static_cast<float>((time - result.time_a) / (result.time_b - result.time_a));
	}
	else {
		result.ratio = 0.0f;
	}
	
	return result;
}

}}} // namespace ofx::ae::util
