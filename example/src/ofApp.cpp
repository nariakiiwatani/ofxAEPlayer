#include "ofApp.h"
#include "ofxAELayer.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofBackground(64, 64, 64);
    
    // Initialize variables
    isPlaying = false;
    showDebugInfo = true;
    showBlendModeTest = false;

	comp_ = std::make_shared<ofx::ae::Composition>();
    // Load composition using CompositionManager singleton
	if (comp_->load("blendmode.json")) {
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
	isPlaying = true;
	
	// Setup blend mode test
	blendModeTest_.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (isPlaying && comp_) {
		if(++timeline_ >= comp_->getInfo().duration) {
			timeline_ = 0;
		}
		if(comp_->setFrame(timeline_)) {
			comp_->update();
		}
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(64, 64, 64);
    
    if (showBlendModeTest) {
        // Draw blend mode test
        blendModeTest_.update();
        blendModeTest_.draw();
    } else {
        // Draw composition if loaded
        if (comp_) {
            comp_->draw(0,0);
        }
    }
    
    // Draw controls and debug info
    drawControls();
    if (showDebugInfo) {
        drawDebugInfo();
    }
}

//--------------------------------------------------------------
void ofApp::drawControls(){
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
void ofApp::drawDebugInfo(){
    if (!comp_) return;

    ofSetColor(255);
    string info = "Debug Info:\n";
    info += "Playing: " + string(isPlaying ? "Yes" : "No") + "\n";
    info += "frame: " + ofToString(comp_->getCurrentTime(), 2) + "s\n";
    const auto& compInfo = comp_->getInfo();
    float duration = static_cast<float>(compInfo.duration) / compInfo.fps;
    info += "Duration: " + ofToString(duration, 2) + "s\n";
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
    info += "Layers: " + ofToString(compInfo.layers.size()) + "\n";
    
    // Add layer information using the new API
    info += "\nLayer Information:\n";
    auto layers = comp_->getLayers();
    for (const auto& layer : layers) {
        if (layer) {
            info += "- " + layer->getName() + ": ";
            
            // Test the sourceType field using the new API
            auto sourceTypeEnum = layer->getSourceType();
            string sourceType = ofx::ae::LayerSource::sourceTypeToString(sourceTypeEnum);
            
            if (sourceType.empty()) {
                info += "[NO SOURCE TYPE]";
            } else {
                info += sourceType;
            }
            
            // Add additional debug info using new API
            info += " [Size: " + ofToString(layer->getWidth()) + "x" + ofToString(layer->getHeight()) + "]";
            if (layer->getSource()) {
                info += " [" + layer->getSource()->getDebugInfo() + "]";
            }
            
            info += "\n";
        }
    }
    
    ofDrawBitmapString(info, 20, ofGetHeight() - 300);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case ' ':
            isPlaying = !isPlaying;
            ofLogNotice("ofApp") << "Playback " << (isPlaying ? "started" : "paused");
            break;
            
        case 'r':
        case 'R':
			timeline_ = 0;
            ofLogNotice("ofApp") << "Reset to beginning";
            break;
            
        case 'd':
        case 'D':
            showDebugInfo = !showDebugInfo;
            break;
            
        case 'b':
        case 'B':
            showBlendModeTest = !showBlendModeTest;
            ofLogNotice("ofApp") << "Blend mode test " << (showBlendModeTest ? "enabled" : "disabled");
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
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
