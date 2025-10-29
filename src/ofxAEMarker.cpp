#include <algorithm>

#include "ofLog.h"

#include "ofxAEMarker.h"

namespace ofx { namespace ae {

bool Marker::parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result) {
	result.clear();
	
	if(!marker_data.is_array()) {
		ofLogError("ofxAEMarker") << "Marker data is not an array";
		return false;
	}
	
	for (const auto &marker : marker_data) {
		MarkerData data;
		
		if(!marker.contains("frame") || !marker["frame"].is_number()) {
			ofLogError("ofxAEMarker") << "Marker missing or invalid frame number";
			continue;
		}
		data.frame = marker["frame"].get<int>();

		if(marker.contains("comment") && marker["comment"].is_string()) {
			data.comment = marker["comment"].get<std::string>();
		}
		
		if(marker.contains("length") && marker["length"].is_number()) {
			data.length = marker["length"].get<int>();
		}
		
		result.push_back(data);
	}
	
	std::sort(result.begin(), result.end(), [](const MarkerData &a, const MarkerData &b) {
		return a.frame < b.frame;
	});
	
	return !result.empty();
}

std::vector<MarkerData> Marker::getMarkersInRange(const std::vector<MarkerData> &markers, int start_frame, int end_frame) {
	std::vector<MarkerData> result;
	
	for(const auto &marker : markers) {
		int marker_end = marker.frame + marker.length;
		if((marker.frame >= start_frame && marker.frame <= end_frame) ||
			(marker.frame <= start_frame && marker_end >= start_frame)) {
			result.push_back(marker);
		}
	}
	
	return result;
}

const MarkerData* Marker::findMarkerByComment(const std::vector<MarkerData> &markers, const std::string &comment) {
	for(const auto &marker : markers) {
		if(marker.comment == comment) {
			return &marker;
		}
	}
	return nullptr;
}

const MarkerData* Marker::findMarkerByFrame(const std::vector<MarkerData> &markers, int frame) {
	for(const auto &marker : markers) {
		if(frame >= marker.frame && frame <= marker.frame + marker.length) {
			return &marker;
		}
	}
	return nullptr;
}

std::vector<const MarkerData*> Marker::findMarkersWithDuration(const std::vector<MarkerData> &markers) {
	std::vector<const MarkerData*> result;
	
	for(const auto &marker : markers) {
		if(marker.length > 0) {
			result.push_back(&marker);
		}
	}
	
	return result;
}

}}
