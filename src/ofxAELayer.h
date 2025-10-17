#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include <vector>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"
#include "ofxAEKeyframe.h"
#include "ofxAETransformProp.h"
#include "ofxAELayerSource.h"
#include "TransformNode.h"
#include "Hierarchical.h"
#include "ofxAEContentVisitor.h"

namespace ofx { namespace ae {

class LayerSource;

class Layer : public TransformNode, public ofBaseDraws, public ofBaseUpdates
{
public:
	using SourceResolver = std::function<std::unique_ptr<LayerSource>(const ofJson &json, const std::filesystem::path& base_dir)>;
	static void registerResolver(SourceResolver resolver);
	static void clearResolvers();

	Layer();
	void accept(ContentVisitor &visitor) { visitor.visit(*this); }

	bool load(const std::string& base_dir);
	bool setup(const ofJson& json, const std::filesystem::path &source_dir="");
	void update() override;
	bool setFrame(int frame);
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

	std::string getDebugInfo() const;

private:
	std::unique_ptr<LayerSource> source_;

	std::string name_;
	int in_, out_;
	int current_frame_;

	TransformProp transform_;
	float opacity_=1;
	BlendMode blendMode_;


	static std::vector<SourceResolver> resolvers_;
	std::unique_ptr<LayerSource> resolveSource(const ofJson& json, const std::filesystem::path& base_dir);
};

}} // namespace ofx::ae
