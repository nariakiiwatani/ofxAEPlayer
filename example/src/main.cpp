#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLWindowSettings settings;
	settings.setGLVersion(3, 2); // ← GL3.2 Core (GLSL #version 150)
	settings.setSize(1280, 720);
	ofCreateWindow(settings);
	ofRunApp(new ofApp());
}
