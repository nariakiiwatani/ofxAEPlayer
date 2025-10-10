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

class Layer : public TransformNode, public ofBaseDraws, public ofBaseUpdates
{
public:
	enum LayerType {
		AV_LAYER,
		VECTOR_LAYER,
		SHAPE_LAYER,
		COMPOSITION_LAYER
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

	virtual bool setup(const ofJson &json);
	void update() override;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
	
	bool load(const std::string &layer_path);
	const LayerInfo& getInfo() const;
	bool hasKeyframes() const;
	const ofJson& getKeyframes() const;
	void setCurrentFrame(int frame);
	int getCurrentFrame() const { return current_frame_; }
	
	// New time management methods for inPoint/outPoint handling
	float getLayerTime(float compositionTime) const;
	bool isActiveAtTime(float compositionTime) const;
	void initializeAtInPoint();
	void handleOutPoint();
	
	bool isVisible() const;
	void setVisible(bool visible);
	float getOpacity() const;
	void setOpacity(float opacity);
	
	const std::string& getParentName() const { return layer_info_.parent; }
	
private:
	LayerInfo layer_info_;
	ofJson keyframes_;
	int current_frame_;
	bool visible_;
	float opacity_;
	bool initialized_at_in_point_;
	
	void updateTransformFromKeyframes();
	void parseTransformData(const ofJson &transform_data);
	static LayerType stringToLayerType(const std::string &type_str);
	ofJson getInterpolatedValue(const std::string &property, int frame) const;
	void applyTransformValue(const std::string &property, const ofJson &value);
};

}}
