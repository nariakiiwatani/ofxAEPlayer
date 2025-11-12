#pragma once

#include <string>
#include <vector>

#include "ofMain.h"

namespace ofx { namespace ae {

struct MarkerData {
	double time;
	std::string comment;
	double duration;
	
	MarkerData() : time(0.0), duration(0.0) {}
};

class Marker {
public:
	static bool parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result);
	static std::vector<MarkerData> getMarkersInRange(const std::vector<MarkerData> &markers, double start_time, double end_time);
	static const MarkerData *findMarkerByComment(const std::vector<MarkerData> &markers, const std::string &comment);
	static const MarkerData *findMarkerByTime(const std::vector<MarkerData> &markers, double time);
	static std::vector<const MarkerData *> findMarkersWithDuration(const std::vector<MarkerData> &markers);
};

}}
