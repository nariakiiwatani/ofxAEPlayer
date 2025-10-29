#pragma once
#include "../data/Enums.h"

class ofShader;

namespace ofx { namespace ae {
	extern std::unique_ptr<ofShader> createShaderForTrackMatteType(TrackMatteType type);
}}
