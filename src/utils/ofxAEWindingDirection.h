#pragma once
#include <string>
#include <unordered_map>

namespace ofx { namespace ae {

enum class WindingDirection {
    DEFAULT,
    COUNTER_CLOCKWISE,
    CLOCKWISE,
    UNKNOWN
};

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

}}