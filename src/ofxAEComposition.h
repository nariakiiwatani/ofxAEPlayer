#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"

namespace ofx { namespace ae {

// Forward declaration
class Layer;

// PlaybackMode enum for flexible loop control
enum class PlaybackMode {
	ONCE,      // Play once and stop
	LOOP,      // Continuous loop (default for backward compatibility)
	PING_PONG, // Play forward then backward repeatedly
	LOOP_COUNT // Loop a specific number of times
};

class Composition : public ofBaseDraws, public ofBaseUpdates
{
public:
	struct Info {
		int duration;
		float fps;
		int width;
		int height;
		int start_frame;
		int end_frame;
		struct LayerInfo {
			std::string name, unique_name, filepath;
		};
		std::vector<LayerInfo> layers;
		std::vector<MarkerData> markers;
		
		Info() : duration(0), fps(30.0f), width(0), height(0),
						start_frame(0), end_frame(0) {}
	};

	bool load(const std::filesystem::path &filepath);
	bool setup(const ofJson &json);
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
	
	const Info& getInfo() const;
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
	
	// 新しいループ制御機能
	void setPlaybackMode(PlaybackMode mode, int count = 0);
	PlaybackMode getPlaybackMode() const { return playback_mode_; }
	void setLoopCallback(std::function<void(int)> callback);
	float getExactCurrentTime() const;
	int getLoopCount() const { return loop_count_; }
	
private:
	Info info_;
	std::vector<std::shared_ptr<Layer>> layers_;
	std::map<std::string, std::weak_ptr<Layer>> name_layers_map_;
	std::map<std::string, std::weak_ptr<Layer>> unique_name_layers_map_;
	int current_frame_;
	bool is_playing_;
	float start_time_;
	
	// 新しいループ制御のメンバ変数
	PlaybackMode playback_mode_;
	int max_loop_count_;
	int loop_count_;
	bool reverse_direction_;
	std::function<void(int)> loop_callback_;
	float exact_time_;
	
	bool parseJson(const ofJson &json);
	bool loadLayers(const std::filesystem::path &base_path);
	void updateCurrentFrame();
};

}}
