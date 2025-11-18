#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"

class ofApp : public ofBaseApp{

public:
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;

	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mouseMoved(int x, int y ) override;
	void mouseDragged(int x, int y, int button) override;
	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
	void mouseEntered(int x, int y) override;
	void mouseExited(int x, int y) override;
	void windowResized(int w, int h) override;
	void dragEvent(ofDragInfo dragInfo) override;
	void gotMessage(ofMessage msg) override;
private:
	enum Repeat {
		ONCE, LOOP, PINGPONG
	};
	struct MarkerPlayer {
		MarkerPlayer(){}
		MarkerPlayer(ofx::ae::MarkerData m) {
			start = m.frame;
			length = m.duration_frames;
			auto commands = ofSplitString(m.comment, "\n");
			name = commands[0];
			repeat = ONCE;
			if(commands.size() > 1) {
				auto r = commands[1];
				repeat = r == "loop" ? LOOP : r == "pingpong" ? PINGPONG : ONCE;
			}
			inner_frame = 0;
			reverse = false;
		}
		std::string name="none";
		int start=0;
		int inner_frame=0;
		int length=0;
		bool reverse=false;
		Repeat repeat=ONCE;
		void update() {
			if(++inner_frame >= length && length > 0) {
				switch(repeat) {
					case ONCE: inner_frame = length; break;
					case LOOP: inner_frame = 0; break;
					case PINGPONG: inner_frame = 0; reverse ^= true; break;
				}
			}
		}
		int getFrame() const {
			return start + (reverse ? length - inner_frame : inner_frame);
		}
	} player_;

	ofx::ae::Composition comp_;
	int num_markers_;
};
