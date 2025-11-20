#pragma once

#include <string>
#include <vector>
#include <optional>

#include "ofMain.h"
#include "../utils/ofxAETimeUtils.h"

namespace ofx { namespace ae {

struct MarkerData {
	std::string name;
	std::string comment;
	Frame frame;
	FrameCount duration_frames;
	
	MarkerData() : frame(0.0f), duration_frames(0.0f) {}
	
	double getTime(float fps) const { return util::frameToTime(frame, fps); }
	double getDuration(float fps) const { return util::frameToTime(duration_frames, fps); }
	
	Frame getFrame() const { return frame; }
	FrameCount getDurationFrames() const { return duration_frames; }
	
	bool containsFrame(Frame f) const {
		return f >= frame && f < (frame + duration_frames);
	}
	
	bool containsTime(double time, float fps) const {
		return containsFrame(util::timeToFrame(time, fps));
	}
};

class Marker {
public:
	static bool parseMarkers(const ofJson &marker_data, std::vector<MarkerData> &result);
	
	static std::vector<MarkerData> getMarkersInFrameRange(const std::vector<MarkerData> &markers, Frame start_frame, Frame end_frame);
	static const MarkerData *findMarkerByComment(const std::vector<MarkerData> &markers, const std::string &comment);
	static const MarkerData *findMarkerByName(const std::vector<MarkerData> &markers, const std::string &name);
	static const MarkerData *findMarkerAtFrame(const std::vector<MarkerData> &markers, Frame frame);
	static std::vector<const MarkerData *> findMarkersWithDuration(const std::vector<MarkerData> &markers);
	
	static std::vector<MarkerData> getMarkersInRange(const std::vector<MarkerData> &markers, double start_time, double end_time, float fps);
	static const MarkerData *findMarkerByTime(const std::vector<MarkerData> &markers, double time, float fps);
};

}}
