#include "ofxAELayer.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include <algorithm>

namespace ofx { namespace ae {

bool Layer::setup(const ofJson &json) {
	parseTransformData(json["transform"]);
	return true;
}

void Layer::update() {
	if (hasKeyframes()) {
		updateTransformFromKeyframes();
	}

	if(isDirty()) {
		refreshMatrix();
	}
}

void Layer::draw(float x, float y, float w, float h) const {
	if (!visible_ || opacity_ <= 0.0f) {
		return;
	}
	// TransformNodeの変換を適用
	pushMatrix();
	
	// 実際の描画処理（基本実装では何も描画しない）
	// サブクラスでオーバーライドして具体的な描画を実装
	
	popMatrix();
}

float Layer::getHeight() const {
	// 基本実装では0を返す
	// サブクラスでオーバーライドして適切な高さを返す
	return 0.0f;
}

float Layer::getWidth() const {
	// 基本実装では0を返す
	// サブクラスでオーバーライドして適切な幅を返す
	return 0.0f;
}

bool Layer::load(const std::string &layer_path) {
	std::ifstream file(layer_path);
	if (!file.is_open()) {
		ofLogError("ofxAELayer") << "Cannot open file: " << layer_path;
		return false;
	}
	
	ofJson json;
	try {
		file >> json;
	} catch (const std::exception &e) {
		ofLogError("ofxAELayer") << "JSON parse error: " << e.what();
		return false;
	}
	
	// 基本情報の解析
	if (json.contains("name") && json["name"].is_string()) {
		layer_info_.name = json["name"].get<std::string>();
	}
	
	if (json.contains("layerType") && json["layerType"].is_string()) {
		layer_info_.type = stringToLayerType(json["layerType"].get<std::string>());
	}
	
	if (json.contains("source") && json["source"].is_string()) {
		layer_info_.source = json["source"].get<std::string>();
	}
	
	if (json.contains("sourceType") && json["sourceType"].is_string()) {
		layer_info_.sourceType = json["sourceType"].get<std::string>();
	}
	
	if (json.contains("in") && json["in"].is_number()) {
		layer_info_.in_point = json["in"].get<int>();
	}
	
	if (json.contains("out") && json["out"].is_number()) {
		layer_info_.out_point = json["out"].get<int>();
	}
	
	if (json.contains("parent") && json["parent"].is_string()) {
		layer_info_.parent = json["parent"].get<std::string>();
	}
	
	// マーカーの解析
	if (json.contains("markers")) {
		if (!Marker::parseMarkers(json["markers"], layer_info_.markers)) {
			ofLogWarning("ofxAELayer") << "Failed to parse markers for layer: " << layer_info_.name;
		}
	}
	
	// レイヤーのTransformデータ解析
	if (json.contains("layer") && json["layer"].contains("transform")) {
		parseTransformData(json["layer"]["transform"]);
	}
	
	// キーフレームデータの保存
	if (json.contains("keyframes")) {
		keyframes_ = json["keyframes"];
	}
	
	// 初期化
	current_frame_ = 0;
	visible_ = true;
	opacity_ = 1.0f;
	initialized_at_in_point_ = false;
	
	return true;
}

const Layer::LayerInfo& Layer::getInfo() const {
	return layer_info_;
}


bool Layer::hasKeyframes() const {
	return !keyframes_.empty();
}

const ofJson& Layer::getKeyframes() const {
	return keyframes_;
}

void Layer::setCurrentFrame(int frame) {
	if (current_frame_ != frame) {
		current_frame_ = frame;
		
		// Check if we need to initialize at inPoint
		if (frame == layer_info_.in_point && !initialized_at_in_point_) {
			initializeAtInPoint();
		}
		
		// Handle outPoint if reached
		if (frame == layer_info_.out_point) {
			handleOutPoint();
		}
		
		// フレーム変更時のTransform自動更新
		if (hasKeyframes()) {
			updateTransformFromKeyframes();
		}
	}
}

bool Layer::isVisible() const {
	return visible_;
}

void Layer::setVisible(bool visible) {
	visible_ = visible;
}

float Layer::getOpacity() const {
	return opacity_;
}

void Layer::setOpacity(float opacity) {
	opacity_ = std::max(0.0f, std::min(1.0f, opacity));
}

void Layer::updateTransformFromKeyframes() {
	if (!hasKeyframes() || !keyframes_.contains("transform")) {
		return;
	}
	
	const auto &transform_keyframes = keyframes_["transform"];
	
	// Use layer-relative frame for keyframe interpolation
	// Convert global frame to layer-relative frame
	int layerFrame = current_frame_ - layer_info_.in_point;
	if (layerFrame < 0) {
		// Before layer starts, use initial values
		layerFrame = 0;
	}
	
	// キーフレーム補間によるTransform値の自動更新
	// 位置（Position）
	if (transform_keyframes.contains("position")) {
		ofJson interpolated_value = getInterpolatedValue("position", layerFrame);
		applyTransformValue("position", interpolated_value);
	}
	
	// スケール（Scale）
	if (transform_keyframes.contains("scale")) {
		ofJson interpolated_value = getInterpolatedValue("scale", layerFrame);
		applyTransformValue("scale", interpolated_value);
	}
	
	// Z軸回転（Rotation Z）
	if (transform_keyframes.contains("rotateZ")) {
		ofJson interpolated_value = getInterpolatedValue("rotateZ", layerFrame);
		applyTransformValue("rotateZ", interpolated_value);
	}
	
	// X軸回転（Rotation X）
	if (transform_keyframes.contains("rotateX")) {
		ofJson interpolated_value = getInterpolatedValue("rotateX", layerFrame);
		applyTransformValue("rotateX", interpolated_value);
	}
	
	// Y軸回転（Rotation Y）
	if (transform_keyframes.contains("rotateY")) {
		ofJson interpolated_value = getInterpolatedValue("rotateY", layerFrame);
		applyTransformValue("rotateY", interpolated_value);
	}
	
	// アンカーポイント（Anchor Point）
	if (transform_keyframes.contains("anchor")) {
		ofJson interpolated_value = getInterpolatedValue("anchor", layerFrame);
		applyTransformValue("anchor", interpolated_value);
	}
	
	// 不透明度（Opacity）
	if (transform_keyframes.contains("opacity")) {
		ofJson interpolated_value = getInterpolatedValue("opacity", layerFrame);
		applyTransformValue("opacity", interpolated_value);
	}
}

void Layer::parseTransformData(const ofJson &transform_data) {
	// AE → TransformNode マッピング統合
	if (transform_data.contains("anchor")) {
		auto anchor = transform_data["anchor"];
		if (anchor.is_array() && anchor.size() >= 2) {
			float z_val = anchor.size() >= 3 ? anchor[2].get<float>() : 0.0f;
			TransformNode::setAnchorPoint(anchor[0].get<float>(), anchor[1].get<float>(), z_val);
		}
	}
	
	if (transform_data.contains("position")) {
		auto pos = transform_data["position"];
		if (pos.is_array() && pos.size() >= 2) {
			float z_val = pos.size() >= 3 ? pos[2].get<float>() : 0.0f;
			// AE座標系からoF座標系への変換（必要に応じて座標変換を適用）
			setTranslation(pos[0].get<float>(), pos[1].get<float>(), z_val);
		}
	}
	
	if (transform_data.contains("scale")) {
		auto scale = transform_data["scale"];
		if (scale.is_array() && scale.size() >= 2) {
			float z_scale = scale.size() >= 3 ? scale[2].get<float>() : 100.0f;
			// AEは%なので0.01倍
			setScale(scale[0].get<float>() * 0.01f,
					   scale[1].get<float>() * 0.01f,
					   z_scale * 0.01f);
		}
	}
	
	if (transform_data.contains("rotateZ")) {
		// AEは度数法、TransformNodeも度数法対応
		setRotationZ(transform_data["rotateZ"].get<float>());
	}
	
	// X, Y回転も対応（3D空間での回転）
	if (transform_data.contains("rotateX")) {
		setRotationX(transform_data["rotateX"].get<float>());
	}
	
	if (transform_data.contains("rotateY")) {
		setRotationY(transform_data["rotateY"].get<float>());
	}
	
	if (transform_data.contains("opacity")) {
		opacity_ = transform_data["opacity"].get<float>() * 0.01f; // %から0-1に変換
	}
}

Layer::LayerType Layer::stringToLayerType(const std::string &type_str) {
	if (type_str == "ADBE AV Layer") return AV_LAYER;
	if (type_str == "ADBE Vector Layer") return VECTOR_LAYER;
	if (type_str == "ADBE Shape Layer") return SHAPE_LAYER;
	if (type_str == "ADBE Composition Layer") return COMPOSITION_LAYER;
	return AV_LAYER; // デフォルト
}

ofJson Layer::getInterpolatedValue(const std::string &property, int frame) const {
	if (!keyframes_.contains("transform") || !keyframes_["transform"].contains(property)) {
		return ofJson();
	}
	
	const auto &keyframe_array = keyframes_["transform"][property];
	if (!keyframe_array.is_array() || keyframe_array.empty()) {
		return ofJson();
	}
	
	// キーフレームデータをパース
	std::vector<Keyframe::KeyframeData> keyframe_data;
	if (!Keyframe::parseKeyframes(keyframe_array, keyframe_data)) {
		return ofJson();
	}
	
	// フレームに対応するキーフレームを見つける
	if (keyframe_data.empty()) {
		return ofJson();
	}
	
	// 最初のキーフレーム以前の場合
	if (frame <= keyframe_data.front().frame) {
		return keyframe_data.front().value;
	}
	
	// 最後のキーフレーム以降の場合
	if (frame >= keyframe_data.back().frame) {
		return keyframe_data.back().value;
	}
	
	// 補間が必要な場合
	for (std::size_t i = 0; i < keyframe_data.size() - 1; ++i) {
		const auto &key1 = keyframe_data[i];
		const auto &key2 = keyframe_data[i + 1];
		
		if (frame >= key1.frame && frame <= key2.frame) {
			float t = static_cast<float>(frame - key1.frame) / static_cast<float>(key2.frame - key1.frame);
			return Keyframe::interpolateValue(key1, key2, t);
		}
	}
	
	return keyframe_data.front().value;
}

void Layer::applyTransformValue(const std::string &property, const ofJson &value) {
	if (value.empty()) return;
	
	if (property == "position" && value.is_array() && value.size() >= 2) {
		float z_val = value.size() >= 3 ? value[2].get<float>() : 0.0f;
		setTranslation(value[0].get<float>(), value[1].get<float>(), z_val);
	} else if (property == "scale" && value.is_array() && value.size() >= 2) {
		float z_scale = value.size() >= 3 ? value[2].get<float>() : 100.0f;
		// AEは%なので0.01倍
		setScale(value[0].get<float>() * 0.01f,
				   value[1].get<float>() * 0.01f,
				   z_scale * 0.01f);
	} else if (property == "rotateZ" && value.is_number()) {
		setRotationZ(value.get<float>());
	} else if (property == "rotateX" && value.is_number()) {
		setRotationX(value.get<float>());
	} else if (property == "rotateY" && value.is_number()) {
		setRotationY(value.get<float>());
	} else if (property == "anchor" && value.is_array() && value.size() >= 2) {
		float z_val = value.size() >= 3 ? value[2].get<float>() : 0.0f;
		TransformNode::setAnchorPoint(value[0].get<float>(), value[1].get<float>(), z_val);
	} else if (property == "opacity" && value.is_number()) {
		setOpacity(value.get<float>() * 0.01f); // %から0-1に変換
	}
}

float Layer::getLayerTime(float compositionTime) const {
	// Convert composition time to layer-relative time
	// compositionTime is assumed to be in frames
	float layerTime = compositionTime - static_cast<float>(layer_info_.in_point);
	
	// Ensure the layer time is not negative
	return std::max(0.0f, layerTime);
}

bool Layer::isActiveAtTime(float compositionTime) const {
	// Check if the layer is active at the given composition time
	int frameTime = static_cast<int>(compositionTime);
	return frameTime >= layer_info_.in_point && frameTime <= layer_info_.out_point;
}

void Layer::initializeAtInPoint() {
	// Initialize the layer when it starts at inPoint
	initialized_at_in_point_ = true;
	
	// Set initial keyframe values if available
	if (hasKeyframes() && keyframes_.contains("transform")) {
		const auto &transform_keyframes = keyframes_["transform"];
		
		// Initialize with first keyframe values or inPoint values
		for (const auto& [property, keyframe_data] : transform_keyframes.items()) {
			if (keyframe_data.is_array() && !keyframe_data.empty()) {
				// Get the value at layer frame 0 (relative to inPoint)
				ofJson initialValue = getInterpolatedValue(property, 0);
				if (!initialValue.empty()) {
					applyTransformValue(property, initialValue);
				}
			}
		}
	}
	
	// Ensure layer is visible at start
	if (!visible_) {
		visible_ = true;
	}
}

void Layer::handleOutPoint() {
	// Handle layer when it reaches outPoint
	// For now, we maintain the final state
	// In future implementations, this could trigger fade-out effects
	// or preserve the final keyframe state
	
	// Optionally, we could set the layer to invisible after outPoint
	// but this depends on the desired behavior
}

const std::string& Layer::getSourceType() const {
	return layer_info_.sourceType;
}

bool Layer::isSourceTypeNone() const {
	return layer_info_.sourceType == "none";
}

bool Layer::isSourceTypeComposition() const {
	return layer_info_.sourceType == "composition";
}

bool Layer::isSourceTypeStill() const {
	return layer_info_.sourceType == "still";
}

bool Layer::isSourceTypeVideo() const {
	return layer_info_.sourceType == "video";
}

bool Layer::isSourceTypeSequence() const {
	return layer_info_.sourceType == "sequence";
}

bool Layer::isSourceTypeFootage() const {
	return layer_info_.sourceType == "footage";
}

bool Layer::isSourceTypeSolid() const {
	return layer_info_.sourceType == "solid";
}

bool Layer::isSourceTypeUnknown() const {
	return layer_info_.sourceType == "unknown";
}

}}
