#pragma once

#include <cmath>
#include <map>
#include "../data/KeyframeData.h"

namespace ofx { namespace ae { namespace util {

constexpr double TIME_EPSILON = 1e-6;  // 1 microsecond

inline bool isNearTime(double a, double b) {
	return std::abs(a - b) < TIME_EPSILON;
}

struct FrameTimeConverter {
	double fps;
	
	double frameToTime(float frame) const {
		return static_cast<double>(frame) / fps;
	}
	
	float timeToFrame(double time) const {
		return static_cast<float>(time * fps);
	}
	
	int timeToFrameInt(double time) const {
		return static_cast<int>(std::round(time * fps));
	}
};

template<typename T>
std::map<double, Keyframe::Data<T>> convertKeyframesToTime(
	const std::map<int, Keyframe::Data<T>>& frame_keyframes,
	double fps)
{
	std::map<double, Keyframe::Data<T>> time_keyframes;
	for(const auto& [frame, kf] : frame_keyframes) {
		double time = static_cast<double>(frame) / fps;
		time_keyframes[time] = kf;
	}
	return time_keyframes;
}

template<typename T>
std::map<int, Keyframe::Data<T>> convertKeyframesToFrame(
	const std::map<double, Keyframe::Data<T>>& time_keyframes,
	double fps)
{
	std::map<int, Keyframe::Data<T>> frame_keyframes;
	for(const auto& [time, kf] : time_keyframes) {
		int frame = static_cast<int>(std::round(time * fps));
		frame_keyframes[frame] = kf;
	}
	return frame_keyframes;
}

template<typename T>
struct TimeKeyframePair {
	const Keyframe::Data<T>* keyframe_a = nullptr;
	const Keyframe::Data<T>* keyframe_b = nullptr;
	float ratio = 0.0f;
	double time_a = 0.0;
	double time_b = 0.0;
};

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
	
	if(!isNearTime(result.time_b, result.time_a)) {
		result.ratio = static_cast<float>((time - result.time_a) / (result.time_b - result.time_a));
	}
	else {
		result.ratio = 0.0f;
	}
	
	return result;
}

}}} // namespace ofx::ae::util