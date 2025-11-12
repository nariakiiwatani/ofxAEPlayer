#include <algorithm>

#include "ofLog.h"

#include "MarkerData.h"

namespace ofx { namespace ae {

bool Marker::parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result)
{
	result.clear();
	
	if(!marker_data.is_array()) {
		ofLogError("ofxAEMarker") << "Marker data is not an array";
		return false;
	}
	
	for (const auto &marker : marker_data) {
		MarkerData data;
		
		if(!marker.contains("time") || !marker["time"].is_number()) {
			ofLogError("ofxAEMarker") << "Marker missing or invalid time value";
			continue;
		}
		data.time = marker["time"].get<double>();
	
		if(marker.contains("comment") && marker["comment"].is_string()) {
			data.comment = marker["comment"].get<std::string>();
		}
		
		if(marker.contains("duration") && marker["duration"].is_number()) {
			data.duration = marker["duration"].get<double>();
		}
		
		result.push_back(data);
	}
	
	std::sort(result.begin(), result.end(), [](const MarkerData &a, const MarkerData &b) {
		return a.time < b.time;
	});
	
	return !result.empty();
}

std::vector<MarkerData> Marker::getMarkersInRange(const std::vector<MarkerData> &markers, double start_time, double end_time)
{
	std::vector<MarkerData> result;
	
	for(const auto &marker : markers) {
		double marker_end = marker.time + marker.duration;
		if((marker.time >= start_time && marker.time <= end_time) ||
			(marker.time <= start_time && marker_end >= start_time)) {
			result.push_back(marker);
		}
	}
	
	return result;
}

const MarkerData* Marker::findMarkerByComment(const std::vector<MarkerData> &markers, const std::string &comment)
{
	for(const auto &marker : markers) {
		if(marker.comment == comment) {
			return &marker;
		}
	}
	return nullptr;
}

const MarkerData* Marker::findMarkerByTime(const std::vector<MarkerData> &markers, double time)
{
	for(const auto &marker : markers) {
		if(time >= marker.time && time <= marker.time + marker.duration) {
			return &marker;
		}
	}
	return nullptr;
}

std::vector<const MarkerData*> Marker::findMarkersWithDuration(const std::vector<MarkerData> &markers)
{
	std::vector<const MarkerData*> result;
	
	for(const auto &marker : markers) {
		if(marker.duration > 0.0) {
			result.push_back(&marker);
		}
	}
	
	return result;
}

}}
