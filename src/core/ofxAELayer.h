#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"

#include "../prop/ofxAEKeyframe.h"
#include "../source/ofxAELayerSource.h"
#include "../data/MarkerData.h"
#include "ofxAEMask.h"
#include "../prop/ofxAEMaskProp.h"
#include "../utils/ofxAETrackMatte.h"
#include "../prop/ofxAETransformProp.h"
#include "../libs/Hierarchical.h"
#include "../libs/TransformNode.h"

namespace ofx { namespace ae {
class Visitor;
}}

namespace ofx { namespace ae {

class LayerSource;

class Layer : public TransformNode, public ofBaseDraws, public ofBaseUpdates
{
public:
	using SourceResolver = std::function<std::unique_ptr<LayerSource>(const ofJson& json, const std::filesystem::path& base_dir)>;
	static void registerResolver(SourceResolver resolver);
	static void clearResolvers();

	Layer();
	void accept(Visitor &visitor);

	bool load(const std::string &base_dir);
	bool setup(const ofJson &json, const std::filesystem::path &source_dir="");
	void update() override;
	bool setFrame(int frame);
	
	using ofBaseDraws::draw;
	void draw() const { draw(0,0); }
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
	void setVisible(bool visible) { is_visible_ = visible; }
	bool isVisible() const { return is_visible_; }

	void setSource(std::unique_ptr<LayerSource> source);
	LayerSource* getSource() const { return source_.get(); }
	template<typename T>
	T* getSource() const {
		return dynamic_cast<T*>(source_.get());
	}
	SourceType getSourceType() const;

	void setName(const std::string &name) { name_ = name; }
	const std::string& getName() const { return name_; }

	void setBlendMode(BlendMode mode) { blend_mode_ = mode; }
	BlendMode getBlendMode() const { return blend_mode_; }

	bool isActiveAtFrame(int frame) const { return in_ <= frame && frame < out_; }

	void setTrackMatte(std::shared_ptr<Layer> src, TrackMatteType type) {
		track_matte_layer_ = src;
		track_matte_shader_ = createShaderForTrackMatteType(type);
	}

	void setUseAsTrackMatte(bool use) { is_track_matte_ = use; }
	bool hasTrackMatte() const { return track_matte_layer_.lock() != nullptr; }
	bool isTrackMatte() const { return is_track_matte_; }
	ofTexture getTexture() const { return layer_fbo_.isAllocated() ? layer_fbo_.getTexture() : ofTexture(); }
	glm::vec2 getFboOffset() const { return fbo_offset_; }

	std::string getDebugInfo() const;

private:
	void updateLayerFBO();
	
	std::unique_ptr<LayerSource> source_;

	std::string name_;
	int in_, out_;
	int current_frame_;

	TransformProp transform_;
	FloatProp time_remap_;

	MaskProp mask_;
	MaskCollection mask_collection_;
	mutable ofFbo mask_fbo_;

	std::weak_ptr<Layer> track_matte_layer_;
	std::unique_ptr<ofShader> track_matte_shader_;
	bool is_track_matte_ = false;

	bool isUseFbo() const { return is_track_matte_ || !mask_collection_.empty() || hasTrackMatte(); }

	mutable ofFbo layer_fbo_;
	glm::vec2 fbo_offset_{0,0};
	float opacity_=1;
	BlendMode blend_mode_;
	bool is_visible_ = false;

	static std::vector<SourceResolver> resolvers_;
	std::unique_ptr<LayerSource> resolveSource(const ofJson &json, const std::filesystem::path &base_dir);
};

}} // namespace ofx::ae
