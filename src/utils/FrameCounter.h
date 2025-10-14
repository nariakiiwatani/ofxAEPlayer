#pragma once

class FrameCounter {
public:
	enum LoopState {
		LOOP_NONE,
		LOOP_ONEWAY,
		LOOP_PINGPONG,
		LOOP_RANDOM,
	};
	FrameCounter();
	int update();
	int update(float elapsed_frame);
	int getCurrent() const;
	void setLoopState(LoopState state) { loop_ = state; }
	void setRange(int from, int length) { from_ = from; length_ = length; }
	void setSpeed(float speed) { speed_ = speed; }
	void setBackward(bool backward);
	void setFrame(int frame);
	void resetFrame(int frame) { frame_ = frame; first_ = true; }
	
	bool isEnd() const;
	bool isForward() const { return is_backward_ == is_backward_internal_; }
	bool isBackward() const { return is_backward_ != is_backward_internal_; }
	bool isStable() const { return speed_ == 0; }
	int getLength() const { return length_; }
	int getFrom() const { return from_; }
	float getSpeed() const { return speed_; }

private:
	float frame_;
	int frame_internal_;
	int from_, length_;
	LoopState loop_;
	bool is_backward_;
	bool is_backward_internal_;
	float speed_;
	bool first_;

private:
	int calcInternalFrame(int input);
};

/* EOF */
