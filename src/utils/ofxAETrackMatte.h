#pragma once
#include <string>
#include <unordered_map>

class ofShader;

namespace ofx { namespace ae {

enum class TrackMatteType {
	NO_TRACK_MATTE,
	ALPHA,
	ALPHA_INVERTED,
	LUMA,
	LUMA_INVERTED,
	UNKNOWN
};

inline TrackMatteType trackMatteTypeFromString(const std::string& str) {
	static const std::unordered_map<std::string, TrackMatteType> map = {
		{"NO_TRACK_MATTE", TrackMatteType::NO_TRACK_MATTE},
		{"ALPHA", TrackMatteType::ALPHA},
		{"ALPHA_INVERTED", TrackMatteType::ALPHA_INVERTED},
		{"LUMA", TrackMatteType::LUMA},
		{"LUMA_INVERTED", TrackMatteType::LUMA_INVERTED}
	};
	auto it = map.find(str);
	return (it != map.end()) ? it->second : TrackMatteType::UNKNOWN;
}

inline std::string toString(TrackMatteType type) {
	switch (type) {
		case TrackMatteType::NO_TRACK_MATTE: return "NO_TRACK_MATTE";
		case TrackMatteType::ALPHA: return "ALPHA";
		case TrackMatteType::ALPHA_INVERTED: return "ALPHA_INVERTED";
		case TrackMatteType::LUMA: return "LUMA";
		case TrackMatteType::LUMA_INVERTED: return "LUMA_INVERTED";
		default: return "UNKNOWN";
	}
}
extern std::unique_ptr<ofShader> createShaderForTrackMatteType(TrackMatteType type);
}}
