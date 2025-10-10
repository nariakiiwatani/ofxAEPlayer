#include "ofxAELayer.h"
#include "ofxAELayerSourceFactory.h"
#include "ofLog.h"
#include "ofUtils.h"
#include <fstream>
#include <algorithm>

namespace ofx { namespace ae {

// ========================================================================
// Constructor & Destructor
// ========================================================================

Layer::Layer()
    : TransformNode()
    , source_(nullptr)
    , name_("")
    , startTime_(0.0f)
    , duration_(0.0f)
    , opacity_(1.0f)
    , blendMode_(BlendMode::NORMAL)
    , visible_(true)
    , isVisible_(true)
    , lastUpdateTime_(-1.0f)
    , current_frame_(0)
    , initialized_at_in_point_(false)
{
}

Layer::~Layer() = default;

// ========================================================================
// Core Lifecycle Methods (Source-based implementation)
// ========================================================================

bool Layer::setup(const ofJson& json, const std::filesystem::path &source_dir) {
    // 1. Parse layer-level properties first
    if (!parseLayerProperties(json)) {
        ofLogError("Layer") << "Failed to parse layer properties";
        return false;
    }
    
    // 2. Create and setup source based on JSON
    auto source = LayerSourceFactory::createSourceOfType(layer_info_.sourceType);
    if (!source) {
        ofLogWarning("Layer") << "No source created for layer: " << name_;
        // Continue without source for layers that don't need one
    } else {
        setSource(std::move(source));
		source_->load(source_dir / layer_info_.source);
    }
    
    // 3. Parse legacy transform data for backward compatibility
    if (json.contains("transform")) {
        parseTransformData(json["transform"]);
    }
    
    return true;
}

void Layer::update(float currentTime) {
    lastUpdateTime_ = currentTime;
    
    // Update legacy keyframes if present
    if (hasKeyframes()) {
        updateLegacyKeyframes(currentTime);
    }
    
    // Update transform matrix if dirty
    if (isDirty()) {
        refreshMatrix();
    }
    
    // Update source if present and needs update
    if (source_ && source_->needsUpdate(currentTime)) {
        source_->update(currentTime);
    }
}

void Layer::draw(const RenderContext& context) const {
    if (!shouldRender(context.currentTime)) {
        return;
    }
    
    if (!source_) {
        // No source to render
        return;
    }
    
    // Create layer-specific render context
    RenderContext layerContext = context;
    
    // Apply layer-level properties
    layerContext.opacity *= opacity_;
    layerContext.blendMode = blendMode_;
    
    // Apply layer transformation matrix
    ofMatrix4x4 layerTransform = *getLocalMatrix() * context.transform;
    // Create new context with transformed matrix
    RenderContext transformedContext(layerContext.x, layerContext.y, layerContext.w, layerContext.h,
                                   layerContext.currentTime, layerContext.opacity, layerTransform, layerContext.blendMode);
    
    // Delegate rendering to source
    source_->draw(transformedContext);
}

// ========================================================================
// Legacy Interface Support
// ========================================================================

void Layer::draw(float x, float y, float w, float h) const {
    if (!visible_ || opacity_ <= 0.0f) {
        return;
    }
    
    // Create basic render context for legacy interface
    ofMatrix4x4 identity;
    identity.makeIdentityMatrix();
    RenderContext context(x, y, w, h, lastUpdateTime_, opacity_, identity, blendMode_);
    
    draw(context);
}

void Layer::update() {
    // Legacy update method - use current time or increment
    float currentTime = lastUpdateTime_ >= 0.0f ? lastUpdateTime_ + (1.0f/30.0f) : 0.0f;
    update(currentTime);
}

float Layer::getHeight() const {
    if (source_) {
        return source_->getHeight();
    }
    return 0.0f;
}

float Layer::getWidth() const {
    if (source_) {
        return source_->getWidth();
    }
    return 0.0f;
}

// ========================================================================
// Source Management
// ========================================================================

void Layer::setSource(std::unique_ptr<LayerSource> source) {
    source_ = std::move(source);
}

LayerSource::SourceType Layer::getSourceType() const {
    if (source_) {
        return source_->getSourceType();
    }
    
    // Return appropriate type based on legacy layer type
    switch (layer_info_.type) {
        case SHAPE_LAYER:
            return LayerSource::SHAPE;
        case COMPOSITION_LAYER:
            return LayerSource::COMPOSITION;
        case AV_LAYER:
        case VECTOR_LAYER:
        default:
            return LayerSource::FOOTAGE;
    }
}

// ========================================================================
// Time Management
// ========================================================================

bool Layer::isActiveAtTime(float time) const {
    float frameTime = time * 30.0f; // Convert to frame-based time
    return frameTime >= static_cast<float>(layer_info_.in_point) &&
           frameTime <= static_cast<float>(layer_info_.out_point);
}

float Layer::getLocalTime(float globalTime) const {
    return globalTime - startTime_;
}

bool Layer::shouldRender(float currentTime) const {
    return visible_ &&
           opacity_ > 0.0f &&
           isActiveAtTime(currentTime) &&
           source_ &&
           source_->isVisible();
}

void Layer::prepareForRendering(float currentTime) {
    // Update keyframes and transform for current time
    if (hasKeyframes()) {
        updateLegacyKeyframes(currentTime);
    }
    
    // Ensure transform is up to date
    if (isDirty()) {
        refreshMatrix();
    }
    
    // Update source if needed
    if (source_ && source_->needsUpdate(currentTime)) {
        source_->update(currentTime);
    }
}

// ========================================================================
// JSON Parsing and Source Creation
// ========================================================================

bool Layer::parseLayerProperties(const ofJson& json) {
    // Parse basic layer information
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
        layer_info_.name = name_;
    }
    
    // Parse layer type
    if (json.contains("layerType") && json["layerType"].is_string()) {
        layer_info_.type = stringToLayerType(json["layerType"].get<std::string>());
    }
    
    // Parse source information
    if (json.contains("source") && json["source"].is_string()) {
        layer_info_.source = json["source"].get<std::string>();
    }
    
    if (json.contains("sourceType") && json["sourceType"].is_string()) {
        layer_info_.sourceType = json["sourceType"].get<std::string>();
    }
    
    // Parse timing properties
    if (json.contains("in") && json["in"].is_number()) {
        layer_info_.in_point = json["in"].get<int>();
        startTime_ = static_cast<float>(layer_info_.in_point) / 30.0f; // Convert frames to seconds
    }
    
    if (json.contains("out") && json["out"].is_number()) {
        layer_info_.out_point = json["out"].get<int>();
        duration_ = (static_cast<float>(layer_info_.out_point - layer_info_.in_point)) / 30.0f;
    }
    
    // Parse parent information
    if (json.contains("parent") && json["parent"].is_string()) {
        layer_info_.parent = json["parent"].get<std::string>();
    }
    
    // Parse visual properties
    if (json.contains("opacity") && json["opacity"].is_number()) {
        opacity_ = json["opacity"].get<float>() * 0.01f; // Convert percentage to 0-1
    }
    
    if (json.contains("visible") && json["visible"].is_boolean()) {
        visible_ = json["visible"].get<bool>();
    }
    
    // Parse blend mode
    if (json.contains("blendMode")) {
        if (json["blendMode"].is_string()) {
            std::string blendModeStr = json["blendMode"].get<std::string>();
            // Map blend mode strings to enum values
            if (blendModeStr == "normal") blendMode_ = BlendMode::NORMAL;
            else if (blendModeStr == "add") blendMode_ = BlendMode::ADD;
            else if (blendModeStr == "multiply") blendMode_ = BlendMode::MULTIPLY;
            else if (blendModeStr == "screen") blendMode_ = BlendMode::SCREEN;
            // Add more mappings as needed
        } else if (json["blendMode"].is_number()) {
            blendMode_ = static_cast<BlendMode>(json["blendMode"].get<int>());
        }
    }
    
    // Parse markers
    if (json.contains("markers")) {
        if (!Marker::parseMarkers(json["markers"], layer_info_.markers)) {
            ofLogWarning("Layer") << "Failed to parse markers for layer: " << name_;
        }
    }
    
    // Parse keyframes for legacy support
    if (json.contains("keyframes")) {
        keyframes_ = json["keyframes"];
    }
    
    return true;
}

// ========================================================================
// Legacy Support Methods
// ========================================================================

void Layer::updateLegacyKeyframes(float currentTime) {
    // Convert time to frame for legacy compatibility
    int frame = static_cast<int>(currentTime * 30.0f) - layer_info_.in_point;
    if (frame != current_frame_) {
        setCurrentFrame(frame + layer_info_.in_point);
    }
    
    // Update transform from keyframes
    if (hasKeyframes()) {
        updateTransformFromKeyframes();
    }
}

const std::string& Layer::getLegacySourceType() const {
    return layer_info_.sourceType;
}

// ========================================================================
// Debug and Diagnostics
// ========================================================================

std::string Layer::getDebugInfo() const {
    std::stringstream info;
    info << "Layer[" << name_ << "] ";
    info << "Type: " << static_cast<int>(layer_info_.type) << " ";
    info << "Source: " << (source_ ? source_->getDebugInfo() : "None") << " ";
    info << "Opacity: " << opacity_ << " ";
    info << "Visible: " << (visible_ ? "true" : "false") << " ";
    info << "Time: " << startTime_ << "-" << (startTime_ + duration_);
    return info.str();
}

bool Layer::load(const std::string &filepath) {
	ofJson json = ofLoadJson(filepath);
	auto source_dir = ofFilePath::getEnclosingDirectory(filepath);
	return setup(json, source_dir);
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
