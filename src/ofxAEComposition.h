#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"

namespace ofx { namespace ae {

// Forward declaration
class Layer;

class Composition : public ofBaseDraws, public ofBaseUpdates {
public:
	struct CompositionInfo {
		int duration;
		float fps;
		int width;
		int height;
		int start_frame;
		int end_frame;
		std::string footage_directory;
		std::vector<std::string> layer_names;
		std::vector<MarkerData> markers;
		
		CompositionInfo() : duration(0), fps(30.0f), width(0), height(0),
						start_frame(0), end_frame(0), footage_directory("footages") {}
	};
	
	// 既存メソッド
	bool setup(const ofJson &json);
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
	
	// 追加実装
	bool load(const std::string &comp_path);
	const CompositionInfo& getInfo() const;
	std::shared_ptr<Layer> getLayer(const std::string &name) const;
	std::vector<std::shared_ptr<Layer>> getLayers() const;
	void play();
	void pause();
	void seekToFrame(int frame);
	void seekToMarker(const std::string &marker_comment);
	
	// タイムライン制御
	int getCurrentFrame() const { return current_frame_; }
	bool isPlaying() const { return is_playing_; }
	float getCurrentTime() const;
	void setCurrentFrame(int frame);
	
private:
	CompositionInfo composition_info_;
	std::vector<std::shared_ptr<Layer>> layers_;
	std::filesystem::path base_path_;
	int current_frame_;
	bool is_playing_;
	float start_time_;
	
	bool parseCompositionJson(const ofJson &json);
	bool loadLayers();
	void updateCurrentFrame();
};

}}
