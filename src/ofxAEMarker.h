#pragma once

#include "ofMain.h"
#include <vector>
#include <string>

namespace ofx { namespace ae {

struct MarkerData {
    int frame;
    std::string comment;
    int length;
    
    MarkerData() : frame(0), length(0) {}
};

class Marker {
public:
    static bool parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result);
    static std::vector<MarkerData> getMarkersInRange(const std::vector<MarkerData> &markers, int start_frame, int end_frame);
    static const MarkerData* findMarkerByComment(const std::vector<MarkerData> &markers, const std::string &comment);
    static const MarkerData* findMarkerByFrame(const std::vector<MarkerData> &markers, int frame);
    static std::vector<const MarkerData*> findMarkersWithDuration(const std::vector<MarkerData> &markers);
};

}}
