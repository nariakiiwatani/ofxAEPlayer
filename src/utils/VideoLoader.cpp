#include "VideoLoader.h"
//#include "ofxImageSequenceVideo.h"
//#include "ofxGvTexture.hpp"

class TexturePlayer : public ofBaseVideoPlayer
{
public:
	bool load(std::string filepath) override {
		return ofLoadImage(texture_, filepath);
	}
	void update() override {}
	float getWidth() const override { return texture_.getWidth(); }
	float getHeight() const override { return texture_.getHeight(); }
	void play() override { }
	void stop() override { }
	bool isPaused() const override { return false; }
	bool isLoaded() const override { return texture_.isAllocated(); }
	bool isPlaying() const override { return true; }
	ofPixels& getPixels() override { return dummy_; }
	const ofPixels& getPixels() const override { return dummy_; }
	bool isFrameNew() const override { return true; }
	void close() override {}
	bool setPixelFormat(ofPixelFormat pixelFormat) override { return true; }
	ofPixelFormat getPixelFormat() const override { return ofPixelFormat(); }
	float getPosition()	const override { return 0; }
	void setPosition(float position) override {}
	float getDuration() const override { return 1; }
	ofTexture* getTexturePtr() override { return &texture_; }
	int getTotalNumFrames() const override { return 1; }
	void setLoopState(ofLoopType loop) override { is_loop_ = loop; }
	bool getIsMovieDone() const override { return !is_loop_; } 
private:
	ofTexture texture_;
	ofPixels dummy_;
	bool is_loop_=false;
};


class SequencePlayer : public ofBaseVideoPlayer
{
public:
	void close() override {
		for(auto &&t : texture_) {
			t.clear();
		}
		texture_.clear();
	}
	bool load(std::string filepath) override {
		if(!ofDirectory::doesDirectoryExist(filepath)) {
			return false;
		}
		is_end_ =
		is_playing_ =
		is_paused_ = false;
		ofDirectory dir;
		dir.open(filepath);
		dir.sort();
		texture_.reserve(dir.size());
		for(auto &&file : dir.getFiles()) {
			ofTexture tex;
			if(ofLoadImage(tex, file.path())) {
				texture_.push_back(tex);
			}
		}
		return isLoaded();
	}
	bool load(const std::filesystem::path &dirpath) override {
		return load(dirpath.string());
	}
	void update() override {
		if(!is_playing_ || is_paused_ || speed_ == 0) {
			return;
		}
		float dt = ofGetLastFrameTime();
		locator_ += speed_*dt*fps_;
		int end_index = getTotalNumFrames();
		if(is_loop_) {
			while(locator_ >= end_index) {
				locator_ -= end_index;
			}
			while(locator_ < 0) {
				locator_ += end_index;
			}
		}
		else if(speed_ > 0 && locator_ >= end_index) {
			locator_ = end_index-1;
			is_end_ = true;
		}
		else if(speed_ < 0 && locator_ < 0) {
			locator_ = 0;
			is_end_ = true;
		}
	}
	float getPosition()	const override {
		return locator_/(float)(getTotalNumFrames()-1);
	}
	void setPosition(float position) override {
		locator_ = (getTotalNumFrames()-1) * ofClamp(position, 0, 1);
	}
	float getWidth() const override { return getTexture().getWidth(); }
	float getHeight() const override { return getTexture().getHeight(); }
	void play() override { is_playing_ = true; is_paused_ = false; }
	void stop() override { is_playing_ = false; }
	bool isPaused() const override { return is_paused_; }
	bool isLoaded() const override { return !texture_.empty(); }
	bool isPlaying() const override { return is_playing_ && !is_paused_; }
	ofPixels& getPixels() override { return dummy_; }
	const ofPixels& getPixels() const override { return dummy_; }
	bool isFrameNew() const override { return true; }
	bool setPixelFormat(ofPixelFormat pixelFormat) override { return true; }
	ofPixelFormat getPixelFormat() const override { return ofPixelFormat(); }
	float getDuration() const override { return getTotalNumFrames()/fps_; }
	void setSpeed(float speed) override { speed_ = speed; }
	ofTexture* getTexturePtr() override { return &texture_[(int)locator_]; }
	int getTotalNumFrames() const override { return texture_.size(); }
	void setLoopState(ofLoopType loop) override { is_loop_ = (loop != OF_LOOP_NONE); }
	bool getIsMovieDone() const override { return is_end_; }
private:
	ofTexture getTexture() const { return texture_[(int)locator_]; }
	std::vector<ofTexture> texture_;
	float fps_=30;
	float locator_=0;
	bool is_playing_=false;
	bool is_paused_=false;
	bool is_frame_new_=false;
	bool is_end_=false;
	bool is_loop_=false;
	float speed_=1;
	
	ofPixels dummy_;
};

/*
static std::map<std::string, std::weak_ptr<IGpuVideoReader>> cache_;
class GvPlayer : public ofBaseVideoPlayer
{
public:
	~GvPlayer() {
		sequence_.unload();
	}
	bool load(std::string filepath) override {
		if (ofFilePath::getFileExt(filepath) != "gv") {
			return false;
		}
		auto mode = ofxGvTexture::GPU_VIDEO_STREAMING_FROM_CPU_MEMORY;
		auto found = cache_.find(filepath);
		if(found != end(cache_)) {
			if(auto reader = found->second.lock()) {
				sequence_.load(reader, mode);
			}
			else {
				found->second = sequence_.load(filepath, mode);
			}
		}
		else {
			auto reader = sequence_.load(filepath, mode);
			cache_.insert({filepath, reader});
		}
		return sequence_.isLoaded();
	}
	bool load(const std::filesystem::path &dirpath) {
		return load(dirpath.string());
	}
	void update() override {
		int frame = sequence_.getFrameAt()+1;
		if(is_loop_ && frame >= sequence_.getFrameCount()) {
			frame = 0;
		}
		sequence_.setFrame(frame);
		sequence_.update();
	}
	void setPosition(float position) override {
		if(!is_playing_) {
			play();
		}
		sequence_.setTime(position*sequence_.getDuration());
		sequence_.update();
	}
	float getWidth() const override { return sequence_.getWidth(); }
	float getHeight() const override { return sequence_.getHeight(); }
	void play() override { is_playing_ = true; }
	void stop() override { is_playing_ = false; sequence_.setFrame(0); }
	bool isPaused() const override { return !is_playing_; }
	bool isLoaded() const override { return sequence_.isLoaded(); }
	bool isPlaying() const override { return is_playing_; }
	ofPixels& getPixels() override { return dummy_; }
	const ofPixels& getPixels() const override { return dummy_; }
	bool isFrameNew() const override { return true; }
	void close() override {}
	bool setPixelFormat(ofPixelFormat pixelFormat) override { return true; }
	ofPixelFormat getPixelFormat() const override { return ofPixelFormat(); }
	float getDuration() const override { return sequence_.getDuration(); }
	ofTexture* getTexturePtr() override { return &sequence_.getTexture(); }
	int getTotalNumFrames() const override { return sequence_.getFrameCount(); }
	void setLoopState(ofLoopType loop) override { is_loop_ = loop != OF_LOOP_NONE; }
	bool getIsMovieDone() const override { return sequence_.getFrameAt() == sequence_.getFrameCount()-1; } 
private:
	ofxGvTexture sequence_;
	bool is_playing_ = false;
	ofPixels dummy_;
	bool is_loop_ = false;
};
 */


template<>
std::shared_ptr<ofVideoPlayer> VideoLoader::load(const std::string &filepath) {
	auto ret = std::make_shared<ofVideoPlayer>();
	std::shared_ptr<ofBaseVideoPlayer> player = nullptr;
	if(!player) player = load<TexturePlayer>(filepath);
	if(!player) player = load<SequencePlayer>(filepath);
//	if(!player) player = load<GvPlayer>(filepath);
	if(!player) {
		return ret->load(filepath) ? ret : nullptr;
	}
	ret->setPlayer(player);
	return ret;
}
