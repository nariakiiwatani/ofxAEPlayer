#pragma once

#include "ofxAELayerSource.h"

namespace ofx { namespace ae {

class Visitor;

class SequenceSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	
	bool setFrame(Frame frame) override;
	
	FrameCount getDurationFrames() const override;

	void draw(float x, float y, float w, float h) const override;
	float getWidth() const override { return pool_.empty() ? 0.f : pool_[0]->getWidth(); }
	float getHeight() const override { return pool_.empty() ? 0.f : pool_[0]->getHeight(); }
	SourceType getSourceType() const override { return SourceType::SEQUENCE; }
	std::string getDebugInfo() const override { return "SequenceSource"; }
	
private:
	bool loadImagesFromDirectory(const std::filesystem::path &dirpath);
	
	std::vector<std::shared_ptr<ofTexture>> pool_;
	std::weak_ptr<ofTexture> texture_;
	Frame frame_offset_ = 0.0f;
	int current_index_ = -1;
};

}} // namespace ofx::ae
