#pragma once

#include "ofMain.h"
#include <vector>
#include <string>

namespace ofx { namespace ae {

// Marker data structure
struct MarkerData {
    int frame;          // Frame number where marker is placed
    std::string comment; // Marker comment/name
    int length;         // Duration of marker (0 for point markers)
    
    MarkerData() : frame(0), length(0) {}
};

// Marker utility functions
class Marker {
public:
    // Parse markers from JSON data
    static bool parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result);
    
    // Get markers within frame range
    static std::vector<MarkerData> getMarkersInRange(const std::vector<MarkerData> &markers, 
                                                     int start_frame, int end_frame);
    
    // Find marker by comment/name
    static const MarkerData* findMarkerByComment(const std::vector<MarkerData> &markers, 
                                                const std::string &comment);
    
    // Find marker at specific frame
    static const MarkerData* findMarkerByFrame(const std::vector<MarkerData> &markers, int frame);
    
    // Get markers with duration (area markers)
    static std::vector<const MarkerData*> findMarkersWithDuration(const std::vector<MarkerData> &markers);
};

}}