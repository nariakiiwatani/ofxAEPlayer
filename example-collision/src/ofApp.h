#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"

class ofApp : public ofBaseApp{

public:
	void setup() override;
	void update() override;
	void draw() override;

private:
	// Core composition
	std::shared_ptr<ofx::ae::Composition> comp_;
	int timeline_ = 0;
	
	// Grid system for collision testing
	std::vector<glm::vec2> gridPoints_;
	int gridResolution_ = 20;

	// Methods
	void setupGrid();
};
