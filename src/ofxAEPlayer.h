#pragma once

#include "ofVideoBaseTypes.h"
#include "ofFbo.h"
#include "core/ofxAEComposition.h"

namespace ofx { namespace ae {

class Player : public ofBaseVideoPlayer
{
public:
	Player();
	~Player() override = default;

	bool load(const of::filesystem::path &fileName) override;
	bool load(std::string fileName) override;
	
	void play() override;
	void stop() override;
	
	float getWidth() const override;
	float getHeight() const override;
	
	bool isPaused() const override;
	bool isLoaded() const override;
	bool isPlaying() const override;
	
	void update() override;
	bool isFrameNew() const override;
	void close() override;
	bool isInitialized() const override;
	bool setPixelFormat(ofPixelFormat pixelFormat) override;
	ofPixelFormat getPixelFormat() const override;
	
	ofPixels& getPixels() override;
	const ofPixels& getPixels() const override;
	
	float getPosition() const override;
	void setPosition(float pct) override;
	void setFrame(int frame) override;
	int getCurrentFrame() const override;
	int getTotalNumFrames() const override;
	
	void setLoopState(ofLoopType state) override;
	ofLoopType getLoopState() const override;
	
	void setSpeed(float speed) override;
	float getSpeed() const override;
	
	float getDuration() const override;
	bool getIsMovieDone() const override;
	
	void firstFrame() override;
	void nextFrame() override;
	void previousFrame() override;
	
	void setVolume(float volume) override;
	
	void setPaused(bool bPause) override;
	
	ofTexture* getTexturePtr() override;
	
	Composition& getComposition() { return composition_; }
	const Composition& getComposition() const { return composition_; }

private:
	void renderToFbo();
	void allocateFbo();
	
	void updatePlayback();
	int constrainFrame(int frame) const;
	
	Composition composition_;
	
	bool is_loaded_;
	bool is_playing_;
	bool is_paused_;
	bool is_frame_new_;
	
	ofLoopType loop_state_;
	float speed_;
	
	float last_update_time_;
	int target_frame_;
	
	ofPixels dummy_pixels_;
	ofPixelFormat pixel_format_;
	
	ofFbo fbo_;
	bool use_fbo_;
	bool fbo_needs_update_;
};

}} // namespace ofx::ae

using ofxAEPlayer = ofx::ae::Player;
