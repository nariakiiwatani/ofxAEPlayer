#include "ofApp.h"
#include "ofxAELayer.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    ofBackground(64, 64, 64);
    
    // Initialize variables
    is_playing_ = false;
    show_debug_info_ = true;

	comp_ = std::make_shared<ofx::ae::Composition>();
    // Load composition using CompositionManager singleton
	if(comp_->load("Edit_Rough.json")) {
		ofLogNotice("ofApp") << "Composition loaded successfully";
		const auto& info = comp_->getInfo();
		ofLogNotice("ofApp") << "Duration: " << info.duration;
		ofLogNotice("ofApp") << "Width: " << info.width;
		ofLogNotice("ofApp") << "Height: " << info.height;
	} else {
		ofLogError("ofApp") << "Failed to load composition";
		comp_.reset();
	}
	timeline_ = 0;
	is_playing_ = true;
}

//--------------------------------------------------------------
void ofApp::update()
{
    if(is_playing_ && comp_) {
 const auto &info = comp_->getInfo();
 if(++timeline_ >= info.end_frame) {
  timeline_ = info.start_frame;
 }
 if(comp_->setFrame(timeline_)) {
  comp_->update();
 }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofClear(64, 64, 64);

	ofPushMatrix();
	float scale = 0.6f;
	ofScale(scale, scale);
	float duration = 60;
	ofTranslate(ofMap(ofWrap(ofGetElapsedTimef(), 0, duration), 0, duration, 0, -comp_->getWidth()+ofGetWidth()), 0);
	if(comp_) {
		comp_->draw(0,0);
	}
	ofPopMatrix();

    if(show_debug_info_) {
		drawControls();
        drawDebugInfo();
    }
}

//--------------------------------------------------------------
void ofApp::drawControls()
{
    ofSetColor(255);
    string controls = "Controls:\n";
    controls += "SPACE: Play/Pause\n";
    controls += "R: Reset to beginning\n";
    controls += "D: Toggle debug info\n";
    controls += "B: Toggle blend mode test\n";
    controls += "LEFT/RIGHT: Seek -/+ 1 second";
    
    ofDrawBitmapString(controls, 20, 20);
}

//--------------------------------------------------------------
void ofApp::drawDebugInfo()
{
    if(!comp_) return;

    ofSetColor(255);
    string info = "Debug Info:\n";
    info += "Playing: " + string(is_playing_ ? "Yes" : "No") + "\n";
    info += "frame: " + ofToString(comp_->getCurrentTime(), 2) + "s\n";
    const auto &compInfo = comp_->getInfo();
    float duration = static_cast<float>(compInfo.duration) / compInfo.fps;
    info += "Duration: " + ofToString(duration, 2) + "s\n";
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
    info += "Layers: " + ofToString(compInfo.layers.size()) + "\n";
    
    // Add layer information using the new API
    info += "\nLayer Information:\n";
    auto layers = comp_->getLayers();
    for(const auto &layer : layers) {
        if(layer) {
            info += "- " + layer->getName() + ": ";
            
            // Test the sourceType field using the new API
            auto sourceTypeEnum = layer->getSourceType();
            string sourceType = ofx::ae::toString(sourceTypeEnum);
            
            if(sourceType.empty()) {
                info += "[NO SOURCE TYPE]";
            } else {
                info += sourceType;
            }
            
            // Add additional debug info using new API
            info += " [Size: " + ofToString(layer->getWidth()) + "x" + ofToString(layer->getHeight()) + "]";
            if(layer->getSource()) {
                info += " [" + layer->getSource()->getDebugInfo() + "]";
            }
            
            info += "\n";
        }
    }
    
    ofDrawBitmapString(info, 20, ofGetHeight() - 300);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch(key) {
        case ' ':
            is_playing_ = !is_playing_;
            ofLogNotice("ofApp") << "Playback " << (is_playing_ ? "started" : "paused");
            break;
            
        case 'r':
        case 'R':
			timeline_ = 0;
            ofLogNotice("ofApp") << "Reset to beginning";
            break;
            
        case 'd':
        case 'D':
            show_debug_info_ = !show_debug_info_;
            break;
            
        case OF_KEY_LEFT:
			timeline_ -= comp_->getInfo().fps;
            break;
            
        case OF_KEY_RIGHT:
			timeline_ += comp_->getInfo().fps ;
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
