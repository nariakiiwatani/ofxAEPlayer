#pragma once

#include "ofMain.h"
#include "ofVideoPlayer.h"
#include "ofxAEComposition.h"

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	ofVideoPlayer player_;
	ofx::ae::Composition::Info info_;
	bool show_debug_info_;
	double playback_speed_;
	bool use_time_api_;

	void drawControls();
	void drawDebugInfo();
};