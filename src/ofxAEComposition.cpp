#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include "JsonFuncs.h"

namespace ofx { namespace ae {

bool Composition::load(const std::filesystem::path &filepath)
{
	return setup(ofLoadJson(filepath), ofFilePath::getEnclosingDirectory(filepath));
}


bool Composition::setup(const ofJson &json, const std::filesystem::path &base_dir) {
#define EXTRACT_INFO2(k,n) json::extract(json, #k, info_.n)
#define EXTRACT_INFO(n) EXTRACT_INFO2(n, n)
	EXTRACT_INFO(duration);
	EXTRACT_INFO(fps);
	EXTRACT_INFO(width);
	EXTRACT_INFO(height);
	EXTRACT_INFO2(startFrame, start_frame);
	EXTRACT_INFO2(endFrame, end_frame);
#undef EXTRACT_INFO2
#undef EXTRACT_INFO

	info_.layers.clear();
	if (json.contains("layers") && json["layers"].is_array()) {
		for (const auto &layer : json["layers"]) {
			info_.layers.emplace_back(layer["name"], layer["uniqueName"], layer["file"], layer["parent"], layer["offset"]);
		}
	}

	if (json.contains("markers")) {
		if (!Marker::parseMarkers(json["markers"], info_.markers)) {
			ofLogWarning("ofxAEComposition") << "Failed to parse markers";
		}
	}

	layers_.clear();
	name_layers_map_.clear();
	unique_name_layers_map_.clear();

	for(auto info : info_.layers) {
		std::filesystem::path layer_file = base_dir / info.filepath;

		if (!std::filesystem::exists(layer_file)) {
			ofLogWarning("ofxAEComposition") << "Layer file not found: " << layer_file;
			continue;
		}
		auto layer = std::make_shared<Layer>();
		if(layer->load(layer_file)) {
			layers_.push_back(layer);
			name_layers_map_.insert({info.name, layer});
			unique_name_layers_map_.insert({info.unique_name, layer});
			layer_offsets_.insert({layer, info.offset});
		} else {
			ofLogError("ofxAEComposition") << "Failed to load layer: " << layer_file;
		}
	}

	for(auto info : info_.layers) {
		if(info.parent == "") continue;
		auto c = unique_name_layers_map_[info.unique_name].lock();
		auto p = unique_name_layers_map_[info.parent].lock();
		if(!c || !p) continue;
		c->setParent(p);
	}

	current_frame_ = -1;
	return !layers_.empty();
}

bool Composition::setFrame(int frame)
{
	if(current_frame_ == frame) {
		return false;
	}
	bool ret = false;
	auto offset = [this](std::shared_ptr<Layer> layer) {
		auto found = layer_offsets_.find(layer);
		if(found == end(layer_offsets_)) return 0;
		return found->second;
	};
	for (auto &layer : layers_) {
		ret |= layer->setFrame(frame - offset(layer));
	}
	current_frame_ = frame;
	return ret;
}
void Composition::update()
{
	for (auto &layer : layers_) {
		layer->update();
	}
}

void Composition::draw(float x, float y, float w, float h) const {
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale(w / info_.width, h / info_.height);
	
	for(auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
		(*it)->draw();
	}
	
	ofPopMatrix();
}

float Composition::getHeight() const {
	return static_cast<float>(info_.height);
}

float Composition::getWidth() const {
	return static_cast<float>(info_.width);
}

const Composition::Info& Composition::getInfo() const {
	return info_;
}

std::shared_ptr<Layer> Composition::getLayer(const std::string &name) const {
	for(auto &&[n,l] : name_layers_map_) {
		if(n == name) return l.lock();
	}
	return nullptr;
}

std::vector<std::shared_ptr<Layer>> Composition::getLayers() const {
	return layers_;
}

void Composition::accept(Visitor& visitor) {
	visitor.visit(*this);
}
}}
