#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAELayerFactory.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>

namespace ofx { namespace ae {

bool Composition::setup(const ofJson &json) {
	return parseCompositionJson(json);
}

void Composition::update() {
	if (is_playing_) {
		updateCurrentFrame();
	}
	
	// レイヤーの更新
	for (auto &layer : layers_) {
		if (layer) {
			layer->setCurrentFrame(current_frame_);
			layer->update();
		}
	}
}

void Composition::draw(float x, float y, float w, float h) const {
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale(w / composition_info_.width, h / composition_info_.height);
	
	// レイヤーを描画順序で描画
	for (const auto &layer : layers_) {
		if (layer) {
			layer->draw(0, 0, composition_info_.width, composition_info_.height);
		}
	}
	
	ofPopMatrix();
}

float Composition::getHeight() const {
	return static_cast<float>(composition_info_.height);
}

float Composition::getWidth() const {
	return static_cast<float>(composition_info_.width);
}

bool Composition::load(const std::string &comp_path) {
	base_path_ = std::filesystem::path(comp_path).parent_path();
	
	std::ifstream file(comp_path);
	if (!file.is_open()) {
		ofLogError("ofxAEComposition") << "Cannot open file: " << comp_path;
		return false;
	}
	
	ofJson json;
	try {
		file >> json;
	} catch (const std::exception &e) {
		ofLogError("ofxAEComposition") << "JSON parse error: " << e.what();
		return false;
	}
	
	if (!parseCompositionJson(json)) {
		return false;
	}
	
	return loadLayers();
}

const Composition::CompositionInfo& Composition::getInfo() const {
	return composition_info_;
}

std::shared_ptr<Layer> Composition::getLayer(const std::string &name) const {
	for (const auto &layer : layers_) {
		if (layer && layer->getInfo().name == name) {
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
		start_time_ = ofGetElapsedTimef() - (current_frame_ / composition_info_.fps);
	}
}

void Composition::pause() {
	is_playing_ = false;
}

void Composition::seekToFrame(int frame) {
	current_frame_ = std::max(composition_info_.start_frame, 
		std::min(frame, composition_info_.end_frame));
	
	if (is_playing_) {
		start_time_ = ofGetElapsedTimef() - (current_frame_ / composition_info_.fps);
	}
}

void Composition::seekToMarker(const std::string &marker_comment) {
	const MarkerData* marker = Marker::findMarkerByComment(composition_info_.markers, marker_comment);
	if (marker) {
		seekToFrame(marker->frame);
	} else {
		ofLogWarning("ofxAEComposition") << "Marker not found: " << marker_comment;
	}
}

float Composition::getCurrentTime() const {
	return current_frame_ / composition_info_.fps;
}

void Composition::setCurrentFrame(int frame) {
	seekToFrame(frame);
}

bool Composition::parseCompositionJson(const ofJson &json) {
	// 基本情報の解析
	if (json.contains("duration") && json["duration"].is_number()) {
		composition_info_.duration = json["duration"].get<int>();
	}
	
	if (json.contains("fps") && json["fps"].is_number()) {
		composition_info_.fps = json["fps"].get<float>();
	}
	
	if (json.contains("width") && json["width"].is_number()) {
		composition_info_.width = json["width"].get<int>();
	}
	
	if (json.contains("height") && json["height"].is_number()) {
		composition_info_.height = json["height"].get<int>();
	}
	
	if (json.contains("startFrame") && json["startFrame"].is_number()) {
		composition_info_.start_frame = json["startFrame"].get<int>();
	}
	
	if (json.contains("endFrame") && json["endFrame"].is_number()) {
		composition_info_.end_frame = json["endFrame"].get<int>();
	}
	
	if (json.contains("footageDirectory") && json["footageDirectory"].is_string()) {
		composition_info_.footage_directory = json["footageDirectory"].get<std::string>();
	}
	
	// レイヤー名リストの解析
	if (json.contains("layers") && json["layers"].is_array()) {
		composition_info_.layer_names.clear();
		for (const auto &layer_name : json["layers"]) {
			if (layer_name.is_string()) {
				composition_info_.layer_names.push_back(layer_name.get<std::string>());
			}
		}
	}
	
	// マーカーの解析
	if (json.contains("markers")) {
		if (!Marker::parseMarkers(json["markers"], composition_info_.markers)) {
			ofLogWarning("ofxAEComposition") << "Failed to parse markers";
		}
	}
	
	// 初期化
	current_frame_ = composition_info_.start_frame;
	is_playing_ = false;
	start_time_ = 0.0f;
	
	return true;
}

bool Composition::loadLayers() {
	layers_.clear();
	
	std::filesystem::path layers_dir = base_path_ / "layers";
	if (!std::filesystem::exists(layers_dir)) {
		ofLogError("ofxAEComposition") << "Layers directory not found: " << layers_dir;
		return false;
	}
	
	for (const std::string &layer_name : composition_info_.layer_names) {
		std::filesystem::path layer_file = layers_dir / (layer_name + ".json");
		
		if (!std::filesystem::exists(layer_file)) {
			ofLogWarning("ofxAEComposition") << "Layer file not found: " << layer_file;
			continue;
		}
		
		auto layer = LayerFactory::createFromJson(layer_file.string());
		if (layer) {
			layers_.push_back(layer);
		} else {
			ofLogError("ofxAEComposition") << "Failed to load layer: " << layer_file;
		}
	}
	
	return !layers_.empty();
}

void Composition::updateCurrentFrame() {
	if (composition_info_.fps <= 0.0f) return;
	
	float elapsed_time = ofGetElapsedTimef() - start_time_;
	int new_frame = composition_info_.start_frame + static_cast<int>(elapsed_time * composition_info_.fps);
	
	if (new_frame > composition_info_.end_frame) {
		// ループ再生（必要に応じて停止に変更可能）
		new_frame = composition_info_.start_frame;
		start_time_ = ofGetElapsedTimef();
	}
	
	current_frame_ = new_frame;
}

}}