#pragma once
#include <string>
#include <unordered_map>
#include "ofGraphicsConstants.h"

class ofShader;

namespace ofx { namespace ae {

enum class BlendMode {
	NORMAL,
	DISSOLVE,
	DANCING_DISSOLVE,
	DARKEN,
	MULTIPLY,
	COLOR_BURN,
	LINEAR_BURN,
	DARKER_COLOR,
	LIGHTEN,
	SCREEN,
	COLOR_DODGE,
	LINEAR_DODGE,
	LIGHTER_COLOR,
	OVERLAY,
	SOFT_LIGHT,
	HARD_LIGHT,
	VIVID_LIGHT,
	LINEAR_LIGHT,
	PIN_LIGHT,
	HARD_MIX,
	DIFFERENCE,
	EXCLUSION,
	SUBTRACT,
	DIVIDE,
	HUE,
	SATURATION,
	COLOR,
	LUMINOSITY,
	ADD,
	CLASSIC_COLOR_DODGE,
	CLASSIC_COLOR_BURN,
	LIGHTEN_COLOR_DODGE,
	LIGHTEN_COLOR_BURN,
	UNKNOWN
};

enum class TrackMatteType {
	NO_TRACK_MATTE,
	ALPHA,
	ALPHA_INVERTED,
	LUMA,
	LUMA_INVERTED,
	UNKNOWN
};

enum class FillRule {
	NON_ZERO,
	EVEN_ODD,
	UNKNOWN
};

enum class WindingDirection {
	DEFAULT,
	COUNTER_CLOCKWISE,
	CLOCKWISE,
	UNKNOWN
};

enum class MaskMode {
	ADD,
	SUBTRACT,
	INTERSECT,
	LIGHTEN,
	DARKEN,
	DIFFERENCE,
	UNKNOWN
};

enum class SourceType {
	SHAPE,
	COMPOSITION,
	SOLID,
	CAMERA,
	LIGHT,
	ADJUSTMENT,
	TEXT,
	NULL_OBJECT,
	STILL,
	VIDEO,
	SEQUENCE,
	UNKNOWN,
	NUM_TYPES
};

inline BlendMode blendModeFromString(const std::string& str) {
	static const std::unordered_map<std::string, BlendMode> map = {
		{"NORMAL", BlendMode::NORMAL},
		{"DISSOLVE", BlendMode::DISSOLVE},
		{"DANCING_DISSOLVE", BlendMode::DANCING_DISSOLVE},
		{"DARKEN", BlendMode::DARKEN},
		{"MULTIPLY", BlendMode::MULTIPLY},
		{"COLOR_BURN", BlendMode::COLOR_BURN},
		{"LINEAR_BURN", BlendMode::LINEAR_BURN},
		{"DARKER_COLOR", BlendMode::DARKER_COLOR},
		{"LIGHTEN", BlendMode::LIGHTEN},
		{"SCREEN", BlendMode::SCREEN},
		{"COLOR_DODGE", BlendMode::COLOR_DODGE},
		{"LINEAR_DODGE", BlendMode::LINEAR_DODGE},
		{"LIGHTER_COLOR", BlendMode::LIGHTER_COLOR},
		{"OVERLAY", BlendMode::OVERLAY},
		{"SOFT_LIGHT", BlendMode::SOFT_LIGHT},
		{"HARD_LIGHT", BlendMode::HARD_LIGHT},
		{"VIVID_LIGHT", BlendMode::VIVID_LIGHT},
		{"LINEAR_LIGHT", BlendMode::LINEAR_LIGHT},
		{"PIN_LIGHT", BlendMode::PIN_LIGHT},
		{"HARD_MIX", BlendMode::HARD_MIX},
		{"DIFFERENCE", BlendMode::DIFFERENCE},
		{"EXCLUSION", BlendMode::EXCLUSION},
		{"SUBTRACT", BlendMode::SUBTRACT},
		{"DIVIDE", BlendMode::DIVIDE},
		{"HUE", BlendMode::HUE},
		{"SATURATION", BlendMode::SATURATION},
		{"COLOR", BlendMode::COLOR},
		{"LUMINOSITY", BlendMode::LUMINOSITY},
		{"ADD", BlendMode::ADD},
		{"CLASSIC_COLOR_DODGE", BlendMode::CLASSIC_COLOR_DODGE},
		{"CLASSIC_COLOR_BURN", BlendMode::CLASSIC_COLOR_BURN},
		{"LIGHTEN_COLOR_DODGE", BlendMode::LIGHTEN_COLOR_DODGE},
		{"LIGHTEN_COLOR_BURN", BlendMode::LIGHTEN_COLOR_BURN}
	};
	auto it = map.find(str);
	if (it != map.end()) return it->second;
	return BlendMode::UNKNOWN;
}

inline std::string toString(BlendMode mode) {
	switch (mode) {
		#define CASE(X) case BlendMode::X: return #X
		CASE(NORMAL);
		CASE(DISSOLVE);
		CASE(DANCING_DISSOLVE);
		CASE(DARKEN);
		CASE(MULTIPLY);
		CASE(COLOR_BURN);
		CASE(LINEAR_BURN);
		CASE(DARKER_COLOR);
		CASE(LIGHTEN);
		CASE(SCREEN);
		CASE(COLOR_DODGE);
		CASE(LINEAR_DODGE);
		CASE(LIGHTER_COLOR);
		CASE(OVERLAY);
		CASE(SOFT_LIGHT);
		CASE(HARD_LIGHT);
		CASE(VIVID_LIGHT);
		CASE(LINEAR_LIGHT);
		CASE(PIN_LIGHT);
		CASE(HARD_MIX);
		CASE(DIFFERENCE);
		CASE(EXCLUSION);
		CASE(SUBTRACT);
		CASE(DIVIDE);
		CASE(HUE);
		CASE(SATURATION);
		CASE(COLOR);
		CASE(LUMINOSITY);
		CASE(ADD);
		CASE(CLASSIC_COLOR_DODGE);
		CASE(CLASSIC_COLOR_BURN);
		CASE(LIGHTEN_COLOR_DODGE);
		CASE(LIGHTEN_COLOR_BURN);
		#undef CASE
		default: return "UNKNOWN";
	}
}

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

inline FillRule fillRuleFromString(const std::string& str) {
	static const std::unordered_map<std::string, FillRule> map = {
		{"NON_ZERO", FillRule::NON_ZERO},
		{"EVEN_ODD", FillRule::EVEN_ODD}
	};
	auto it = map.find(str);
	return (it != map.end()) ? it->second : FillRule::UNKNOWN;
}

inline std::string toString(FillRule rule) {
	switch (rule) {
		case FillRule::NON_ZERO: return "NON_ZERO";
		case FillRule::EVEN_ODD: return "EVEN_ODD";
		default: return "UNKNOWN";
	}
}

inline ofPolyWindingMode toOf(FillRule rule) {
	switch(rule) {
		case FillRule::NON_ZERO: return OF_POLY_WINDING_NONZERO;
		case FillRule::EVEN_ODD: return OF_POLY_WINDING_ODD;
		default: return OF_POLY_WINDING_NONZERO;
	}
}

inline WindingDirection windingDirectionFromString(const std::string& str) {
	static const std::unordered_map<std::string, WindingDirection> map = {
		{"DEFAULT", WindingDirection::DEFAULT},
		{"COUNTER_CLOCKWISE", WindingDirection::COUNTER_CLOCKWISE},
		{"CLOCKWISE", WindingDirection::CLOCKWISE}
	};
	auto it = map.find(str);
	return (it != map.end()) ? it->second : WindingDirection::UNKNOWN;
}

inline std::string toString(WindingDirection direction) {
	switch (direction) {
		case WindingDirection::DEFAULT: return "DEFAULT";
		case WindingDirection::COUNTER_CLOCKWISE: return "COUNTER_CLOCKWISE";
		case WindingDirection::CLOCKWISE: return "CLOCKWISE";
		default: return "UNKNOWN";
	}
}

inline int getDesiredSign(WindingDirection direction) {
	switch (direction) {
		case WindingDirection::DEFAULT:
		case WindingDirection::COUNTER_CLOCKWISE: 
			return 1;
		case WindingDirection::CLOCKWISE: 
			return -1;
		default: 
			return 1;
	}
}

inline MaskMode maskModeFromString(const std::string& str) {
	static const std::unordered_map<std::string, MaskMode> map = {
		{"ADD", MaskMode::ADD},
		{"SUBTRACT", MaskMode::SUBTRACT},
		{"INTERSECT", MaskMode::INTERSECT},
		{"LIGHTEN", MaskMode::LIGHTEN},
		{"DARKEN", MaskMode::DARKEN},
		{"DIFFERENCE", MaskMode::DIFFERENCE}
	};
	auto it = map.find(str);
	return (it != map.end()) ? it->second : MaskMode::UNKNOWN;
}

inline std::string toString(MaskMode mode) {
	switch (mode) {
		case MaskMode::ADD: return "ADD";
		case MaskMode::SUBTRACT: return "SUBTRACT";
		case MaskMode::INTERSECT: return "INTERSECT";
		case MaskMode::LIGHTEN: return "LIGHTEN";
		case MaskMode::DARKEN: return "DARKEN";
		case MaskMode::DIFFERENCE: return "DIFFERENCE";
		default: return "UNKNOWN";
	}
}

inline std::string toString(SourceType type) {
	switch (type) {
		case SourceType::SHAPE: return "shape";
		case SourceType::COMPOSITION: return "composition";
		case SourceType::SOLID: return "solid";
		case SourceType::CAMERA: return "camera";
		case SourceType::LIGHT: return "light";
		case SourceType::ADJUSTMENT: return "adjustment";
		case SourceType::TEXT: return "text";
		case SourceType::NULL_OBJECT: return "null";
		case SourceType::STILL: return "still";
		case SourceType::VIDEO: return "video";
		case SourceType::SEQUENCE: return "sequence";
		default: return "unknown";
	}
}

inline SourceType sourceTypeFromString(const std::string& name) {
	static const std::unordered_map<std::string, SourceType> map = {
		{"shape", SourceType::SHAPE},
		{"composition", SourceType::COMPOSITION},
		{"solid", SourceType::SOLID},
		{"camera", SourceType::CAMERA},
		{"light", SourceType::LIGHT},
		{"adjustment", SourceType::ADJUSTMENT},
		{"text", SourceType::TEXT},
		{"null", SourceType::NULL_OBJECT},
		{"still", SourceType::STILL},
		{"video", SourceType::VIDEO},
		{"sequence", SourceType::SEQUENCE}
	};
	auto it = map.find(name);
	return (it != map.end()) ? it->second : SourceType::UNKNOWN;
}

}}
