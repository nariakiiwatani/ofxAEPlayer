#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"
#include "ofxAEKeyframe.h"
#include "utils/TransformNode.h"

namespace ofx { namespace ae {

class Layer : public ofBaseDraws, public ofBaseUpdates, public std::enable_shared_from_this<Layer> {
public:
	enum LayerType {
		AV_LAYER,
		VECTOR_LAYER,
		SHAPE_LAYER
	};
	
	struct LayerInfo {
		std::string name;
		LayerType type;
		std::string source;
		int in_point;
		int out_point;
		std::string parent;
		std::vector<MarkerData> markers;
		
		LayerInfo() : type(AV_LAYER), in_point(0), out_point(0) {}
	};
	
	// 既存メソッド
	bool setup(const ofJson &json);
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
	
	// 追加実装
	bool load(const std::string &layer_path);
	const LayerInfo& getInfo() const;
	TransformNode& getTransform();
	const TransformNode& getTransform() const;
	bool hasKeyframes() const;
	const ofJson& getKeyframes() const;
	void setCurrentFrame(int frame);
	int getCurrentFrame() const { return current_frame_; }
	
	// レイヤー制御
	bool isVisible() const;
	void setVisible(bool visible);
	float getOpacity() const;
	void setOpacity(float opacity);
	
	// 親子関係管理
	void setParentLayer(std::shared_ptr<Layer> parent);
	std::shared_ptr<Layer> getParentLayer() const;
	void addChildLayer(std::shared_ptr<Layer> child);
	void removeChildLayer(std::shared_ptr<Layer> child);
	std::vector<std::shared_ptr<Layer>> getChildLayers() const;
	
	// Transform階層計算
	void updateHierarchicalTransform();
	
	// 親レイヤー名による関係設定（Composition側で使用）
	const std::string& getParentName() const { return layer_info_.parent; }
	
private:
	LayerInfo layer_info_;
	TransformNode transform_;
	ofJson keyframes_;
	int current_frame_;
	bool visible_;
	float opacity_;
	
	// 親子関係
	std::weak_ptr<Layer> parent_layer_;
	std::vector<std::shared_ptr<Layer>> child_layers_;
	
	void updateTransformFromKeyframes();
	void parseTransformData(const ofJson &transform_data);
	static LayerType stringToLayerType(const std::string &type_str);
	ofJson getInterpolatedValue(const std::string &property, int frame) const;
	void applyTransformValue(const std::string &property, const ofJson &value);
};

}}
