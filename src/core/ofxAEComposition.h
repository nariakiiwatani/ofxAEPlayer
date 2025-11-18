#pragma once

#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "../data/MarkerData.h"
#include "../utils/ofxAETrackMatte.h"
#include "../utils/ofxAETimeUtils.h"

namespace ofx { namespace ae {

class Visitor;
class Layer;

class Composition : public ofBaseDraws, public ofBaseUpdates
{
public:
	void accept(Visitor &visitor);
	struct Info {
		FrameCount frame_count;
		float fps;
		int width;
		int height;
		Frame start_frame;
		Frame end_frame;
		
		double getDuration() const { return util::frameToTime(frame_count, fps); }
		double getStartTime() const { return util::frameToTime(start_frame, fps); }
		double getEndTime() const { return util::frameToTime(end_frame, fps); }
		
		struct LayerInfo {
			std::string name, unique_name, filepath, parent;
			float offset;
			bool visible;
			struct TrackMatte {
				std::string layer;
				TrackMatteType type;
			};
			std::optional<TrackMatte> track_matte;
		};
		std::vector<LayerInfo> layers;
		std::vector<MarkerData> markers;

		Info() : frame_count(0.0f), fps(30.0f), width(0), height(0), start_frame(0.0f), end_frame(0.0f) {}
	};

	bool load(const std::filesystem::path &filepath);
	bool setup(const ofJson &json, const std::filesystem::path &base_dir);
	
	bool setFrame(Frame frame);
	Frame getFrame() const { return current_frame_; }
	FrameCount getFrameCount() const { return info_.frame_count; }
	float getFps() const { return info_.fps; }
	
	bool setTime(double time);
	double getTime() const;
	double getDuration() const;
	Frame convertTimeToFrame(double time) const { return util::timeToFrame(time, info_.fps); }
	double convertFrameToTime(int frame) const { return util::frameToTime(frame, info_.fps); }

	void update() override;
	using ofBaseDraws::draw;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;

	const Info& getInfo() const;
	std::shared_ptr<Layer> getLayer(const std::string &name) const;
	std::vector<std::shared_ptr<Layer>> getLayers() const;

private:
	Info info_;
	std::vector<std::shared_ptr<Layer>> layers_;
	std::map<std::string, std::weak_ptr<Layer>> name_layers_map_;
	std::map<std::string, std::weak_ptr<Layer>> unique_name_layers_map_;
	std::map<std::weak_ptr<Layer>, Frame, std::owner_less<std::weak_ptr<Layer>>> layer_offsets_;

	Frame current_frame_;
};

}}
