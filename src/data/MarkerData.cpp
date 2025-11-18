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
	
	for(const auto &marker : marker_data) {
		MarkerData data;
		
		// v3.0 JSON: frame-based only
		if(!marker.contains("frame") || !marker["frame"].is_number()) {
			ofLogError("ofxAEMarker") << "Marker missing or invalid frame value (v3.0 JSON required)";
			continue;
		}
		data.frame = marker["frame"].get<float>();
		
		// Name field
		if(marker.contains("name") && marker["name"].is_string()) {
			data.name = marker["name"].get<std::string>();
		}
		
		// Comment field
		if(marker.contains("comment") && marker["comment"].is_string()) {
			data.comment = marker["comment"].get<std::string>();
		}
		
		// Duration in frames
		if(marker.contains("durationFrames") && marker["durationFrames"].is_number()) {
			data.duration_frames = marker["durationFrames"].get<float>();
		}
		
		result.push_back(data);
	}
	
	// Sort by frame position
	std::sort(result.begin(), result.end(), [](const MarkerData &a, const MarkerData &b) {
		return a.frame < b.frame;
	});
	
	return !result.empty();
}

// Frame-based range query (primary)
std::vector<MarkerData> Marker::getMarkersInFrameRange(const std::vector<MarkerData> &markers, Frame start_frame, Frame end_frame)
{
	std::vector<MarkerData> result;
	
	for(const auto &marker : markers) {
		Frame marker_end = marker.frame + marker.duration_frames;
		if((marker.frame >= start_frame && marker.frame <= end_frame) ||
			(marker.frame <= start_frame && marker_end >= start_frame)) {
			result.push_back(marker);
		}
	}
	
	return result;
}

// Legacy time-based range query (compatibility wrapper)
std::vector<MarkerData> Marker::getMarkersInRange(const std::vector<MarkerData> &markers, double start_time, double end_time, float fps)
{
	Frame start_frame = util::timeToFrame(start_time, fps);
	Frame end_frame = util::timeToFrame(end_time, fps);
	return getMarkersInFrameRange(markers, start_frame, end_frame);
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

const MarkerData* Marker::findMarkerByName(const std::vector<MarkerData> &markers, const std::string &name)
{
	for(const auto &marker : markers) {
		if(marker.name == name) {
			return &marker;
		}
	}
	return nullptr;
}

// Frame-based marker lookup (primary)
const MarkerData* Marker::findMarkerAtFrame(const std::vector<MarkerData> &markers, Frame frame)
{
	for(const auto &marker : markers) {
		if(marker.containsFrame(frame)) {
			return &marker;
		}
	}
	return nullptr;
}

// Legacy time-based marker lookup (compatibility wrapper)
const MarkerData* Marker::findMarkerByTime(const std::vector<MarkerData> &markers, double time, float fps)
{
	Frame frame = util::timeToFrame(time, fps);
	return findMarkerAtFrame(markers, frame);
}

std::vector<const MarkerData*> Marker::findMarkersWithDuration(const std::vector<MarkerData> &markers)
{
	std::vector<const MarkerData*> result;
	
	for(const auto &marker : markers) {
		if(marker.duration_frames > 0.0f) {
			result.push_back(&marker);
		}
	}
	
	return result;
}

}}
