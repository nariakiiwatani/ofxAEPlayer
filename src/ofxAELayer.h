#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"
#include "ofxAEKeyframe.h"
#include "ofxAERenderContext.h"
#include "ofxAELayerSource.h"
#include "utils/TransformNode.h"
#include "utils/Hierarchical.h"
#include "utils/PropertyValue.h"

namespace ofx { namespace ae {

class LayerSource;

class Layer : public TransformNode, public ofBaseDraws, public ofBaseUpdates
{
public:

	Layer();
	virtual ~Layer();

	bool load(const std::string& base_dir);
	bool setup(const ofJson& json, const std::filesystem::path &source_dir="");
	void update() override;
	void setFrame(int frame);
	using ofBaseDraws::draw;
	void draw() const { draw(0,0); }
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;

	void setSource(std::unique_ptr<LayerSource> source);
	LayerSource* getSource() const { return source_.get(); }

	template<typename T>
	T* getSource() const {
		return dynamic_cast<T*>(source_.get());
	}
	LayerSource::SourceType getSourceType() const;

	void setName(const std::string& name) { name_ = name; }
	const std::string& getName() const { return name_; }

	void setBlendMode(BlendMode mode) { blendMode_ = mode; }
	BlendMode getBlendMode() const { return blendMode_; }

	bool isActiveAtFrame(int frame) const;
	int getLocalFrame(int global) const;

	bool shouldRender(int frame) const;
	void prepareForRendering(int frame);


	// ========================================================================
	// Debug and Diagnostics
	// ========================================================================

	/**
	 * Get debug information about this layer
	 * @return String containing layer and source debug info
	 */
	std::string getDebugInfo() const;

	/**
	 * Get a vec3 property value at specified time
	 * @param propertyPath Path to property (e.g., "transform.position")
	 * @param time Time to evaluate property at
	 * @return Property value
	 */
	glm::vec3 getPropertyVec3(const std::string& propertyPath, float time) const;

	/**
	 * Check if a property exists
	 * @param propertyPath Path to property
	 * @return true if property exists
	 */
	bool hasProperty(const std::string& propertyPath) const;

	/**
	 * Get list of all available properties
	 * @return Vector of property paths
	 */
	std::vector<std::string> getAvailableProperties() const;

private:
	std::unique_ptr<LayerSource> source_;

	std::string name_;
	int in_, out_;
	int current_frame_;

	BlendMode blendMode_;

	struct TransformProps {
		TransformProps();
		PropertyValue<glm::vec3> position;
		PropertyValue<glm::vec3> scale;
		PropertyValue<glm::vec3> rotation;
		PropertyValue<glm::vec3> anchorPoint;
		PropertyValue<float> opacity;
		void loadInitialValue(const ofJson &data);
		void loadAnimation(const ofJson &data);
	} transform_;
};

}} // namespace ofx::ae
