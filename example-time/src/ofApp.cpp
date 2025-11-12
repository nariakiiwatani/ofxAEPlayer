#include "ofApp.h"
#include "ofxAEPlayer.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetVerticalSync(true);
    ofSetFrameRate(60);  // Higher frame rate for smoother time-based playback
    ofBackground(64, 64, 64);
    
    show_debug_info_ = true;
    playback_speed_ = 1.0;
    use_time_api_ = true;  // Start with time API enabled

	auto ae_player = std::make_shared<ofxAEPlayer>();
	player_.setPlayer(ae_player);
	if(player_.load("interaction-test.json")) {
		ofLogNotice("ofApp") << "Composition loaded successfully";
		auto &&comp = ae_player->getComposition();
		info_ = comp.getInfo();
		ofLogNotice("ofApp") << "Duration: " << info_.duration << " frames";
		ofLogNotice("ofApp") << "FPS: " << info_.fps;
		ofLogNotice("ofApp") << "Width: " << info_.width;
		ofLogNotice("ofApp") << "Height: " << info_.height;

		player_.setLoopState(OF_LOOP_NORMAL);
		player_.play();
	}
}

//--------------------------------------------------------------
void ofApp::update()
{
	if(use_time_api_) {
		// Time-based API demonstration (Phase 2)
		// Calculate time directly from elapsed time
		auto ae_player = std::dynamic_pointer_cast<ofxAEPlayer>(player_.getPlayer());
		if(ae_player && player_.isPlaying()) {
			auto &comp = ae_player->getComposition();
			double comp_duration_seconds = static_cast<double>(info_.duration) / info_.fps;
			double elapsed = ofGetElapsedTimef();
			double comp_time = fmod(elapsed * playback_speed_, comp_duration_seconds);
			
			// Use the new time API
			comp.setTime(comp_time);
			comp.update();
		}
	} else {
		// Traditional frame-based API (for comparison)
		player_.update();
	}
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
	string controls = "Time-Based API Example (Phase 2)\n";
	controls += "=================================\n";
    controls += "SPACE: Play/Pause\n";
    controls += "R: Reset to beginning\n";
    controls += "D: Toggle debug info\n";
    controls += "T: Toggle Time API / Frame API\n";
    controls += "+/-: Increase/Decrease playback speed\n";
    controls += "LEFT/RIGHT: Seek -/+ 1 second";
    
    ofDrawBitmapStringHighlight(controls, 20, 20);
}

//--------------------------------------------------------------
void ofApp::drawDebugInfo()
{
	auto ae_player = std::dynamic_pointer_cast<ofxAEPlayer>(player_.getPlayer());
    string info = "Debug Info:\n";
    info += "===========\n";
    info += "API Mode: " + string(use_time_api_ ? "Time API (setTime)" : "Frame API (setFrame)") + "\n";
    info += "Playing: " + string(player_.isPlaying() ? "Yes" : "No") + "\n";
    info += "Playback Speed: " + ofToString(playback_speed_, 2) + "x\n";
    
	if(ae_player) {
		auto &comp = ae_player->getComposition();
		double current_time = comp.getTime();
		float current_frame = comp.getCurrentFrame();
		
		info += "Current Time: " + ofToString(current_time, 6) + "s\n";
		info += "Current Frame: " + ofToString(current_frame, 2) + "\n";
	} else {
		info += "Frame: " + ofToString(player_.getPosition()*info_.duration) + "\n";
	}
	
    info += "Comp Duration: " + ofToString(static_cast<double>(info_.duration) / info_.fps, 2) + "s (" + ofToString(info_.duration) + " frames)\n";
    info += "Comp FPS: " + ofToString(info_.fps, 1) + "\n";
    info += "App FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
	info += "Layers: " + ofToString(info_.layers.size()) + "\n";

    ofDrawBitmapStringHighlight(info, 20, ofGetHeight() - 350);
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
		case 't':
			use_time_api_ = !use_time_api_;
			ofLogNotice("ofApp") << "Switched to " << (use_time_api_ ? "Time API" : "Frame API");
			break;
		case '+':
		case '=':
			playback_speed_ = ofClamp(playback_speed_ + 0.1, 0.1, 5.0);
			ofLogNotice("ofApp") << "Playback speed: " << playback_speed_ << "x";
			break;
		case '-':
		case '_':
			playback_speed_ = ofClamp(playback_speed_ - 0.1, 0.1, 5.0);
			ofLogNotice("ofApp") << "Playback speed: " << playback_speed_ << "x";
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