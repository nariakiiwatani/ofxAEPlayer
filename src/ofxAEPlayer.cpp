#include "ofxAEPlayer.h"
#include "ofUtils.h"
#include "ofGraphics.h"

namespace ofx { namespace ae {

Player::Player()
	: is_loaded_(false)
	, is_playing_(false)
	, is_paused_(false)
	, is_frame_new_(false)
	, loop_state_(OF_LOOP_NONE)
	, speed_(1.0f)
	, last_update_time_(0.0f)
	, target_frame_(0.0f)
	, pixel_format_(OF_PIXELS_RGBA)
	, use_fbo_(true)
	, fbo_needs_update_(true)
{
}

bool Player::load(const of::filesystem::path &fileName)
{
	bool result = composition_.load(fileName);
	if(result) {
		is_loaded_ = true;
		target_frame_ = static_cast<float>(composition_.getInfo().start_frame);
		composition_.setFrame(target_frame_);
		last_update_time_ = ofGetElapsedTimef();
		
		if(use_fbo_) {
			allocateFbo();
		}
	}
	return result;
}

bool Player::load(std::string fileName)
{
	return load(of::filesystem::path(fileName));
}

void Player::play()
{
	is_playing_ = true;
	is_paused_ = false;
	last_update_time_ = ofGetElapsedTimef();
}

void Player::stop()
{
	is_playing_ = false;
	is_paused_ = false;
	target_frame_ = static_cast<float>(composition_.getInfo().start_frame);
	composition_.setFrame(target_frame_);
}

float Player::getWidth() const
{
	return composition_.getWidth();
}

float Player::getHeight() const
{
	return composition_.getHeight();
}

bool Player::isPaused() const
{
	return is_paused_;
}

bool Player::isLoaded() const
{
	return is_loaded_;
}

bool Player::isPlaying() const
{
	return is_playing_ && !is_paused_;
}

void Player::update()
{
	if(!is_loaded_) {
		return;
	}

	is_frame_new_ = false;

	if(is_playing_ && !is_paused_) {
		updatePlayback();
	}

	composition_.update();
	
	if(use_fbo_ && is_loaded_) {
		renderToFbo();
	}
}

void Player::updatePlayback()
{
	float current_time = ofGetElapsedTimef();
	float elapsed = current_time - last_update_time_;
	last_update_time_ = current_time;

	const auto &info = composition_.getInfo();
	float frames_per_second = info.fps;
	float frame_delta = elapsed * frames_per_second * speed_;

	float new_frame = target_frame_ + frame_delta;

	switch(loop_state_) {
		case OF_LOOP_NONE: {
			new_frame = constrainFrame(new_frame);
			if(new_frame >= info.end_frame) {
				new_frame = info.end_frame;
				is_playing_ = false;
			}
			else if(new_frame < info.start_frame) {
				new_frame = info.start_frame;
				is_playing_ = false;
			}
			break;
		}
		case OF_LOOP_NORMAL: {
			int range = info.end_frame - info.start_frame;
			if(range > 0) {
				new_frame = ((new_frame - info.start_frame) % range + range) % range + info.start_frame;
			}
			break;
		}
		case OF_LOOP_PALINDROME: {
			int range = info.end_frame - info.start_frame;
			if(range > 0) {
				int total_frames = range * 2;
				int wrapped = ((new_frame - info.start_frame) % total_frames + total_frames) % total_frames;
				if(wrapped >= range) {
					new_frame = info.end_frame - (wrapped - range);
				}
				else {
					new_frame = info.start_frame + wrapped;
				}
			}
			break;
		}
	}

	if(new_frame != target_frame_) {
		target_frame_ = new_frame;
		is_frame_new_ = composition_.setFrame(target_frame_);
	}
}

int Player::constrainFrame(int frame) const
{
	const auto &info = composition_.getInfo();
	return ofClamp(frame, info.start_frame, info.end_frame);
}

float Player::constrainFrame(float frame) const
{
	const auto &info = composition_.getInfo();
	return ofClamp(frame, static_cast<float>(info.start_frame), static_cast<float>(info.end_frame));
}

bool Player::isFrameNew() const
{
	return is_frame_new_;
}

void Player::close()
{
	is_loaded_ = false;
	is_playing_ = false;
	is_paused_ = false;
	is_frame_new_ = false;
	target_frame_ = 0;
}

bool Player::setPixelFormat(ofPixelFormat pixelFormat)
{
	pixel_format_ = pixelFormat;
	return true;
}

ofPixelFormat Player::getPixelFormat() const
{
	return pixel_format_;
}

ofPixels& Player::getPixels()
{
	return dummy_pixels_;
}

const ofPixels& Player::getPixels() const
{
	return dummy_pixels_;
}

float Player::getPosition() const
{
	if(!is_loaded_) {
		return 0.0f;
	}
	const auto &info = composition_.getInfo();
	int range = info.end_frame - info.start_frame;
	if(range <= 0) {
		return 0.0f;
	}
	return static_cast<float>(target_frame_ - info.start_frame) / static_cast<float>(range);
}

void Player::setPosition(float pct)
{
	if(!is_loaded_) {
		return;
	}
	const auto &info = composition_.getInfo();
	int range = info.end_frame - info.start_frame;
	target_frame_ = static_cast<float>(info.start_frame) + pct * static_cast<float>(range);
	target_frame_ = constrainFrame(target_frame_);
	composition_.setFrame(target_frame_);
	is_frame_new_ = true;
}

void Player::setFrame(float frame)
{
	if(!is_loaded_) {
		return;
	}
	target_frame_ = constrainFrame(frame);
	composition_.setFrame(target_frame_);
	is_frame_new_ = true;
}

int Player::getCurrentFrame() const
{
	return static_cast<int>(target_frame_);
}

int Player::getTotalNumFrames() const
{
	if(!is_loaded_) {
		return 0;
	}
	const auto &info = composition_.getInfo();
	return info.end_frame - info.start_frame;
}

void Player::setLoopState(ofLoopType state)
{
	loop_state_ = state;
}

ofLoopType Player::getLoopState() const
{
	return loop_state_;
}

void Player::setSpeed(float speed)
{
	speed_ = speed;
}

float Player::getSpeed() const
{
	return speed_;
}

float Player::getDuration() const
{
	if(!is_loaded_) {
		return 0.0f;
	}
	const auto &info = composition_.getInfo();
	return static_cast<float>(info.end_frame - info.start_frame) / info.fps;
}

bool Player::getIsMovieDone() const
{
	if(!is_loaded_) {
		return true;
	}
	const auto &info = composition_.getInfo();
	return !is_playing_ && target_frame_ >= static_cast<float>(info.end_frame);
}

void Player::firstFrame()
{
	setFrame(static_cast<float>(composition_.getInfo().start_frame));
}

void Player::nextFrame()
{
	setFrame(target_frame_ + 1);
}

void Player::previousFrame()
{
	setFrame(target_frame_ - 1);
}

void Player::setVolume(float volume)
{
	// Not applicable for AE compositions (no audio support)
}

void Player::setPaused(bool bPause)
{
	is_paused_ = bPause;
	if(!is_paused_) {
		last_update_time_ = ofGetElapsedTimef();
	}
}

ofTexture* Player::getTexturePtr()
{
	if(use_fbo_ && fbo_.isAllocated()) {
		return &fbo_.getTexture();
	}
	return nullptr;
}

void Player::allocateFbo()
{
	if(!is_loaded_) {
		return;
	}
	
	const auto &info = composition_.getInfo();
	if(info.width > 0 && info.height > 0) {
		ofFboSettings settings;
		settings.width = info.width;
		settings.height = info.height;
		settings.internalformat = GL_RGBA;
		settings.useDepth = false;
		settings.useStencil = false;
		settings.textureTarget = GL_TEXTURE_2D;
		settings.minFilter = GL_LINEAR;
		settings.maxFilter = GL_LINEAR;
		settings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
		settings.wrapModeVertical = GL_CLAMP_TO_EDGE;
		
		fbo_.allocate(settings);
		fbo_needs_update_ = true;
	}
}

void Player::renderToFbo()
{
	if(!fbo_.isAllocated()) {
		return;
	}
	
	fbo_.begin();
	ofClear(0, 0, 0, 0);
	composition_.draw(0, 0);
	fbo_.end();
	
	fbo_needs_update_ = false;
}

}} // namespace ofx::ae
