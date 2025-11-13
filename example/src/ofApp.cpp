#include "ofApp.h"
#include "ofxAEPlayer.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    ofBackground(64, 64, 64);
    
    show_debug_info_ = true;

	auto ae_player = std::make_shared<ofxAEPlayer>();
	player_.setPlayer(ae_player);
	if(player_.load("09_oohamu_main.json")) {
		ofLogNotice("ofApp") << "Composition loaded successfully";
		auto &&comp = ae_player->getComposition();
		info_ = comp.getInfo();
		ofLogNotice("ofApp") << "Duration: " << info_.duration;
		ofLogNotice("ofApp") << "Width: " << info_.width;
		ofLogNotice("ofApp") << "Height: " << info_.height;

		player_.setLoopState(OF_LOOP_NORMAL);
		player_.play();
	}
}

//--------------------------------------------------------------
void ofApp::update()
{
	player_.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	player_.draw(0,0);
    if(show_debug_info_) {
		drawControls();
        drawDebugInfo();
    }
}

//--------------------------------------------------------------
void ofApp::drawControls()
{
	string controls = "Controls:\n";
    controls += "SPACE: Play/Pause\n";
    controls += "R: Reset to beginning\n";
    controls += "D: Toggle debug info\n";
    controls += "LEFT/RIGHT: Seek -/+ 1 second";
    
    ofDrawBitmapString(controls, 20, 20);
}

//--------------------------------------------------------------
void ofApp::drawDebugInfo()
{
    string info = "Debug Info:\n";
    info += "Playing: " + string(player_.isPlaying() ? "Yes" : "No") + "\n";
	info += "time: " + ofToString(player_.getPosition()*info_.duration) + "s\n";
    info += "Duration: " + ofToString(info_.duration, 2) + "s\n";
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
	info += "Layers: " + ofToString(info_.layers.size()) + "\n";

    ofDrawBitmapString(info, 20, ofGetHeight() - 300);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch(key) {
        case ' ':
			player_.setPaused(player_.isPlaying());
			break;
        case 'r':
			player_.setPosition(0);
            break;
        case 'd':
            show_debug_info_ = !show_debug_info_;
            break;
        case OF_KEY_LEFT:
			player_.setPosition(player_.getPosition()-1.f/player_.getDuration());
            break;
        case OF_KEY_RIGHT:
			player_.setPosition(player_.getPosition()+1.f/player_.getDuration());
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
