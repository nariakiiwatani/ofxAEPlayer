#include "ofApp.h"

#include "ofxAEMarkerUtils.h"

//--------------------------------------------------------------
void ofApp::setup(){
	comp_.load("marker_test/comp.json");
}

//--------------------------------------------------------------
void ofApp::update(){
	player_.update();
	if(comp_.setFrame(player_.getFrame())) {
		comp_.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	comp_.draw(0,0);

	std::stringstream ss;
	ss << "Marker name: " << player_.name << std::endl
	<< "start at: " << player_.start << std::endl
	<< "length: " << player_.length << std::endl
	<< "repeat: " << (player_.repeat == LOOP ? "loop" : player_.repeat == PINGPONG ? "pingpong" : "once") << std::endl
	<< "local frame: " << player_.inner_frame << std::endl
	<< "reverse: " << (player_.reverse ? "true" : "false") << std::endl
	;
	ofDrawBitmapString(ss.str(), 10, 50);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	int index = key-'1';
	if(0 <= index && index < comp_.getInfo().markers.size()) {
		player_ = MarkerPlayer(comp_.getInfo().markers[index]);
	}
	else {
		player_ = MarkerPlayer();
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
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

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
