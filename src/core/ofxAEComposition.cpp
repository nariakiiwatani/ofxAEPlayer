#include <fstream>

#include "ofLog.h"
#include "ofUtils.h"

#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAEVisitor.h"
#include "JsonFuncs.h"

namespace ofx { namespace ae {

bool Composition::load(const std::filesystem::path &filepath)
{
	return setup(ofLoadJson(filepath), ofFilePath::getEnclosingDirectory(filepath));
}

bool Composition::setup(const ofJson &json, const std::filesystem::path &base_dir)
{
	json::extract(json, "fps", info_.fps);
	json::extract(json, "frameCount", info_.frame_count);
	json::extract(json, "startFrame", info_.start_frame);
	json::extract(json, "endFrame", info_.end_frame);
	if(info_.end_frame == 0.0f) {
		info_.end_frame = info_.frame_count;
	}
	
	json::extract(json, "width", info_.width);
	json::extract(json, "height", info_.height);

	info_.layers.clear();
	if(json.contains("layers") && json["layers"].is_array()) {
		for(const auto &layer : json["layers"]) {
#define EXTRACT_LAYER2(k,n) json::extract(layer, #k, info.n)
#define EXTRACT_LAYER(n) EXTRACT_LAYER2(n, n)
			Info::LayerInfo info;
			EXTRACT_LAYER(name);
			EXTRACT_LAYER2(uniqueName, unique_name);
			EXTRACT_LAYER2(file, filepath);
			EXTRACT_LAYER(parent);
			EXTRACT_LAYER2(startFrame, offset);
			EXTRACT_LAYER(visible);
#undef EXTRACT_LAYER2
#undef EXTRACT_LAYER
			if(layer.contains("trackMatte")) {
				ofJson track_matte = layer["trackMatte"];
				Info::LayerInfo::TrackMatte matte;
				matte.layer = track_matte["layer"];
				matte.type = trackMatteTypeFromString(track_matte["type"]);
				info.track_matte = matte;
			}
			info_.layers.push_back(info);
		}
	}

	if(json.contains("markers") && json["markers"].is_array()) {
		Marker::parseMarkers(json["markers"], info_.markers);
	}

	layers_.clear();
	name_layers_map_.clear();
	unique_name_layers_map_.clear();

	for(auto info : info_.layers) {
		std::filesystem::path layer_file = base_dir / info.filepath;

		if(!std::filesystem::exists(layer_file)) {
			ofLogWarning("ofxAEComposition") << "Layer file not found: " << layer_file;
			continue;
		}
		auto layer = std::make_shared<Layer>();
		if(layer->load(layer_file)) {
			layers_.push_back(layer);
			name_layers_map_.insert({info.name, layer});
			unique_name_layers_map_.insert({info.unique_name, layer});
			
			Frame offset_frame = info.offset;
			layer_offsets_.insert({layer, offset_frame});
			layer->setVisible(info.visible);
			
			layer->setFps(info_.fps);
		}
		else {
			ofLogError("ofxAEComposition") << "Failed to load layer: " << layer_file;
		}
	}

	for(auto info : info_.layers) {
		auto layer = unique_name_layers_map_[info.unique_name].lock();
		if(!layer) continue;
		if(info.parent != "") {
			if(auto l = unique_name_layers_map_[info.parent].lock()) {
				layer->setParent(l);
			}
		}
		if(info.track_matte) {
			if(auto l = unique_name_layers_map_[info.track_matte->layer].lock()) {
				layer->setTrackMatte(l, info.track_matte->type);
				l->setUseAsTrackMatte(true);
			}
		}
	}

	current_frame_ = -1.0f;
	setFrame(0.0f);
	return !layers_.empty();
}

bool Composition::setFrame(Frame frame)
{
	if(util::isNearFrame(current_frame_, frame)) {
		return false;
	}
	
	bool ret = false;
	auto getOffset = [this](std::shared_ptr<Layer> layer) -> Frame {
		auto found = layer_offsets_.find(layer);
		return (found != end(layer_offsets_)) ? found->second : 0.0f;
	};
	
	for(auto& layer : layers_) {
		Frame offset = getOffset(layer);
		ret |= layer->setFrame(frame - offset);
	}
	
	current_frame_ = frame;
	return ret;
}

bool Composition::setTime(double time)
{
	return setFrame(util::timeToFrame(time, info_.fps));
}

double Composition::getTime() const
{
	return util::frameToTime(current_frame_, info_.fps);
}

double Composition::getDuration() const
{
	return util::frameToTime(info_.frame_count, info_.fps);
}
void Composition::update()
{
	for(auto& layer : layers_) {
		layer->update();
	}
}

void Composition::draw(float x, float y, float w, float h) const
{
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale(w / info_.width, h / info_.height);
	
	for(auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
		if(!(*it)->isVisible() || (*it)->isAdjustmentLayer()) {
			continue;
		}
		(*it)->draw();
	}
	
	ofPopMatrix();
}

float Composition::getHeight() const
{
	return static_cast<float>(info_.height);
}

float Composition::getWidth() const
{
	return static_cast<float>(info_.width);
}

const Composition::Info& Composition::getInfo() const
{
	return info_;
}

std::shared_ptr<Layer> Composition::getLayer(const std::string &name) const
{
	for(auto&& [n,l] : name_layers_map_) {
		if(n == name) return l.lock();
	}
	return nullptr;
}

std::vector<std::shared_ptr<Layer>> Composition::getLayers() const
{
	return layers_;
}

void Composition::accept(Visitor &visitor)
{
	visitor.visit(*this);
}
}}
