#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>

namespace ofx { namespace ae {

bool Composition::setup(const ofJson &json) {
	return parseJson(json);
}

void Composition::update() {
	if (is_playing_) {
		updateCurrentFrame();
	}
	
	// レイヤーの更新
	float currentTime = getCurrentTime();
	for (auto &layer : layers_) {
		if (layer) {
			layer->update();
		}
	}
}

void Composition::draw(float x, float y, float w, float h) const {
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale(w / info_.width, h / info_.height);
	
	for (const auto &layer : layers_) {
		if (layer) {
			layer->draw();
		}
	}
	
	ofPopMatrix();
}

float Composition::getHeight() const {
	return static_cast<float>(info_.height);
}

float Composition::getWidth() const {
	return static_cast<float>(info_.width);
}

bool Composition::load(const std::filesystem::path &filepath)
{
	ofJson json = ofLoadJson(filepath);

	if (!parseJson(json)) {
		return false;
	}

	std::filesystem::path base_path = ofFilePath::getEnclosingDirectory(filepath);
	return loadLayers(base_path);
}

bool Composition::loadLayers(const std::filesystem::path &base_path)
{
	layers_.clear();

	for(auto info : info_.layers) {
		std::filesystem::path layer_file = base_path / info.filepath;

		if (!std::filesystem::exists(layer_file)) {
			ofLogWarning("ofxAEComposition") << "Layer file not found: " << layer_file;
			continue;
		}
		auto layer = std::make_shared<Layer>();
		if(layer->load(layer_file)) {
			layers_.push_back(layer);
		} else {
			ofLogError("ofxAEComposition") << "Failed to load layer: " << layer_file;
		}
	}

	return !layers_.empty();
}

const Composition::Info& Composition::getInfo() const {
	return info_;
}

std::shared_ptr<Layer> Composition::getLayer(const std::string &name) const {
	for (const auto &layer : layers_) {
		if (layer && layer->getName() == name) {
			return layer;
		}
	}
	return nullptr;
}

std::vector<std::shared_ptr<Layer>> Composition::getLayers() const {
	return layers_;
}

void Composition::play() {
	if (!is_playing_) {
		is_playing_ = true;
		start_time_ = ofGetElapsedTimef() - (current_frame_ / info_.fps);
	}
}

void Composition::pause() {
	is_playing_ = false;
}

void Composition::seekToFrame(int frame) {
	current_frame_ = std::max(info_.start_frame, 
		std::min(frame, info_.end_frame));
	
	if (is_playing_) {
		start_time_ = ofGetElapsedTimef() - (current_frame_ / info_.fps);
	}
}

void Composition::seekToMarker(const std::string &marker_comment) {
	const MarkerData* marker = Marker::findMarkerByComment(info_.markers, marker_comment);
	if (marker) {
		seekToFrame(marker->frame);
	} else {
		ofLogWarning("ofxAEComposition") << "Marker not found: " << marker_comment;
	}
}

float Composition::getCurrentTime() const {
	return current_frame_ / info_.fps;
}

void Composition::setCurrentFrame(int frame) {
	seekToFrame(frame);
}

bool Composition::parseJson(const ofJson &json) {
	// 基本情報の解析
	if (json.contains("duration") && json["duration"].is_number()) {
		info_.duration = json["duration"].get<int>();
	}
	
	if (json.contains("fps") && json["fps"].is_number()) {
		info_.fps = json["fps"].get<float>();
	}
	
	if (json.contains("width") && json["width"].is_number()) {
		info_.width = json["width"].get<int>();
	}
	
	if (json.contains("height") && json["height"].is_number()) {
		info_.height = json["height"].get<int>();
	}
	
	if (json.contains("startFrame") && json["startFrame"].is_number()) {
		info_.start_frame = json["startFrame"].get<int>();
	}
	
	if (json.contains("endFrame") && json["endFrame"].is_number()) {
		info_.end_frame = json["endFrame"].get<int>();
	}
	
	// レイヤー名リストの解析
	if (json.contains("layers") && json["layers"].is_array()) {
		info_.layers.clear();
		for (const auto &layer : json["layers"]) {
			info_.layers.emplace_back(layer["name"], layer["uniqueName"], layer["file"]);
		}
	}
	
	// マーカーの解析
	if (json.contains("markers")) {
		if (!Marker::parseMarkers(json["markers"], info_.markers)) {
			ofLogWarning("ofxAEComposition") << "Failed to parse markers";
		}
	}
	
	// 初期化
	current_frame_ = info_.start_frame;
	is_playing_ = false;
	start_time_ = 0.0f;
	
	// 新しいループ制御の初期化（後方互換性のためデフォルトはLOOP）
	playback_mode_ = PlaybackMode::LOOP;
	max_loop_count_ = 0;
	loop_count_ = 0;
	reverse_direction_ = false;
	loop_callback_ = nullptr;
	exact_time_ = 0.0f;
	
	return true;
}

void Composition::updateCurrentFrame() {
	if (info_.fps <= 0.0f) return;
	
	float elapsed_time = ofGetElapsedTimef() - start_time_;
	exact_time_ = elapsed_time;
	
	int frame_range = info_.end_frame - info_.start_frame + 1;
	int new_frame = info_.start_frame + static_cast<int>(elapsed_time * info_.fps);
	
	// 再生モード別の処理
	switch (playback_mode_) {
		case PlaybackMode::ONCE:
			if (new_frame > info_.end_frame) {
				new_frame = info_.end_frame;
				is_playing_ = false;
			}
			break;
			
		case PlaybackMode::LOOP:
			if (new_frame > info_.end_frame) {
				new_frame = info_.start_frame;
				start_time_ = ofGetElapsedTimef();
				loop_count_++;
				if (loop_callback_) {
					loop_callback_(loop_count_);
				}
			}
			break;
			
		case PlaybackMode::PING_PONG:
			{
				int cycle_frames = frame_range * 2 - 2; // 往復のフレーム数
				if (cycle_frames <= 0) cycle_frames = 1;
				
				int cycle_frame = static_cast<int>(elapsed_time * info_.fps) % cycle_frames;
				
				if (cycle_frame < frame_range) {
					// 前進
					new_frame = info_.start_frame + cycle_frame;
					if (reverse_direction_) {
						reverse_direction_ = false;
						loop_count_++;
						if (loop_callback_) {
							loop_callback_(loop_count_);
						}
					}
				} else {
					// 後退
					new_frame = info_.end_frame - (cycle_frame - frame_range + 1);
					if (!reverse_direction_) {
						reverse_direction_ = true;
					}
				}
			}
			break;
			
		case PlaybackMode::LOOP_COUNT:
			if (new_frame > info_.end_frame) {
				loop_count_++;
				if (loop_callback_) {
					loop_callback_(loop_count_);
				}
				
				if (max_loop_count_ > 0 && loop_count_ >= max_loop_count_) {
					new_frame = info_.end_frame;
					is_playing_ = false;
				} else {
					new_frame = info_.start_frame;
					start_time_ = ofGetElapsedTimef();
				}
			}
			break;
	}
	
	current_frame_ = new_frame;
}

// 新しいメソッドの実装
void Composition::setPlaybackMode(PlaybackMode mode, int count) {
	playback_mode_ = mode;
	max_loop_count_ = count;
	loop_count_ = 0;
	reverse_direction_ = false;
}

void Composition::setLoopCallback(std::function<void(int)> callback) {
	loop_callback_ = callback;
}

float Composition::getExactCurrentTime() const {
	return exact_time_;
}

}}
