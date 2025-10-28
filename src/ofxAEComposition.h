#pragma once

#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"
#include "ofxAETrackMatte.h"

namespace ofx { namespace ae {

class Visitor;
class Layer;

class Composition : public ofBaseDraws, public ofBaseUpdates
{
public:
	void accept(Visitor& visitor);
	struct Info {
		int duration;
		float fps;
		int width;
		int height;
		int start_frame;
		int end_frame;
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

		Info() : duration(0), fps(30.0f), width(0), height(0),
		start_frame(0), end_frame(0) {}
	};

	bool load(const std::filesystem::path &filepath);
	bool setup(const ofJson &json, const std::filesystem::path &base_dir);
	bool setFrame(int frame);
	void update() override;
	using ofBaseDraws::draw;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;

	float getCurrentTime() const { return current_frame_/info_.fps; }
	int getCurrentFrame() const { return current_frame_; }

	const Info& getInfo() const;
	std::shared_ptr<Layer> getLayer(const std::string &name) const;
	std::vector<std::shared_ptr<Layer>> getLayers() const;

private:
	Info info_;
	std::vector<std::shared_ptr<Layer>> layers_;
	std::map<std::string, std::weak_ptr<Layer>> name_layers_map_;
	std::map<std::string, std::weak_ptr<Layer>> unique_name_layers_map_;
	std::map<std::weak_ptr<Layer>, int, std::owner_less<std::weak_ptr<Layer>>> layer_offsets_;

	int current_frame_;
};

}}
