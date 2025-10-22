#pragma once
#include <string>
#include <unordered_map>

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
extern void applyBlendMode(BlendMode mode);
}}
