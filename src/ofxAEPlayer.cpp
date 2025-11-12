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
	, target_time_(0.0)
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
		target_time_ = 0.0;
		composition_.setTime(target_time_);
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
	target_time_ = 0.0;
	composition_.setTime(0.0);
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

	double time_delta = elapsed * speed_;
	double new_time = target_time_ + time_delta;

	double duration = composition_.getDuration();

	switch(loop_state_) {
		case OF_LOOP_NONE: {
			new_time = constrainTime(new_time);
			if(new_time >= duration) {
				new_time = duration;
				is_playing_ = false;
			}
			else if(new_time < 0.0) {
				new_time = 0.0;
				is_playing_ = false;
			}
			break;
		}
		case OF_LOOP_NORMAL: {
			double start_time = 0.0;
			if(duration > 0.0) {
				double offset = new_time - start_time;
				new_time = fmod(fmod(offset, duration) + duration, duration) + start_time;
			}
			break;
		}
		case OF_LOOP_PALINDROME: {
			double start_time = 0.0;
			double end_time = duration;
			double range = end_time - start_time;
			if(range > 0.0) {
				double total_duration = range * 2.0;
				double offset = new_time - start_time;
				double wrapped = fmod(fmod(offset, total_duration) + total_duration, total_duration);
				if(wrapped >= range) {
					new_time = end_time - (wrapped - range);
				}
				else {
					new_time = start_time + wrapped;
				}
			}
			break;
		}
	}

	if(new_time != target_time_) {
		target_time_ = new_time;
		is_frame_new_ = composition_.setTime(target_time_);
	}
}

double Player::constrainTime(double time) const
{
	double start_time = 0.0;
	double end_time = composition_.getDuration();
	return ofClamp(time, start_time, end_time);
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
	target_time_ = 0.0;
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
	double duration = composition_.getDuration();
	if(duration <= 0.0) {
		return 0.0f;
	}
	return static_cast<float>(target_time_ / duration);
}

void Player::setPosition(float pct)
{
	if(!is_loaded_) {
		return;
	}
	double duration = composition_.getDuration();
	target_time_ = pct * duration;
	target_time_ = constrainTime(target_time_);
	composition_.setTime(target_time_);
	is_frame_new_ = true;
}

void Player::setFrame(float frame)
{
	if(!is_loaded_) {
		return;
	}
	// Convert frame to time
	double time = frame / composition_.getFps();
	target_time_ = constrainTime(time);
	composition_.setTime(target_time_);
	is_frame_new_ = true;
}

int Player::getCurrentFrame() const
{
	return static_cast<int>(target_time_ * composition_.getFps());
}

int Player::getTotalNumFrames() const
{
	if(!is_loaded_) {
		return 0;
	}
	return static_cast<int>(composition_.getDuration() * composition_.getFps());
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
	return static_cast<float>(composition_.getDuration());
}

bool Player::getIsMovieDone() const
{
	if(!is_loaded_) {
		return true;
	}
	double duration = composition_.getDuration();
	return !is_playing_ && target_time_ >= duration;
}

void Player::firstFrame()
{
	setFrame(0.0f);
}

void Player::nextFrame()
{
	if(!is_loaded_) {
		return;
	}
	double frame_duration = 1.0 / composition_.getFps();
	target_time_ = constrainTime(target_time_ + frame_duration);
	composition_.setTime(target_time_);
	is_frame_new_ = true;
}

void Player::previousFrame()
{
	if(!is_loaded_) {
		return;
	}
	double frame_duration = 1.0 / composition_.getFps();
	target_time_ = constrainTime(target_time_ - frame_duration);
	composition_.setTime(target_time_);
	is_frame_new_ = true;
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
