#pragma once

#include "ofxAELayerSource.h"
#include "ofVideoPlayer.h"
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class VideoSource : public LayerSource
{
public:
	void accept(Visitor &visitor) override;
	bool load(const std::filesystem::path &filepath) override;
	bool setFrame(float frame) override;
	void draw(float x, float y, float w, float h) const override;
	float getWidth() const override;
	float getHeight() const override;
	SourceType getSourceType() const override { return SourceType::VIDEO; }
	std::string getDebugInfo() const override;

private:
	std::shared_ptr<ofVideoPlayer> player_;
	std::filesystem::path filepath_;
};

}} // namespace ofx::ae
