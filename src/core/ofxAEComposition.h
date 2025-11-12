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
		double duration;
		float fps;
		int width;
		int height;
		struct LayerInfo {
			std::string name, unique_name, filepath, parent;
			int offset;
			bool visible;
			struct TrackMatte {
				std::string layer;
				TrackMatteType type;
			};
			std::optional<TrackMatte> track_matte;
		};
		std::vector<LayerInfo> layers;
		std::vector<MarkerData> markers;

		Info() : duration(0.0), fps(30.0f), width(0), height(0) {}
	};

	bool load(const std::filesystem::path &filepath);
	bool setup(const ofJson &json, const std::filesystem::path &base_dir);
	
	bool setTime(double time);
	double getTime() const { return current_time_; }
	float getTimeFloat() const { return static_cast<float>(current_time_); }
	double getDuration() const { return info_.duration; }
	double getFps() const { return info_.fps; }
	int convertTimeToFrame(float time) const { return time*info_.fps; }
	double convertFrameToTime(int frame) const { return frame/info_.fps; }
	int getStartFrame() const { return 0; }
	int getEndFrame() const { return static_cast<int>(info_.duration * info_.fps); }
	int getDurationInFrames() const { return getEndFrame() - getStartFrame(); }

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
	std::map<std::weak_ptr<Layer>, int, std::owner_less<std::weak_ptr<Layer>>> layer_offsets_;

	double current_time_;
};

}}
