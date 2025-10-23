#pragma once
#include <string>
#include <unordered_map>
#include "ofGraphicsConstants.h"

namespace ofx { namespace ae {

enum class FillRule {
    NON_ZERO,
    EVEN_ODD,
    UNKNOWN
};

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

}}
