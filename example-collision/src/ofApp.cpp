#include "ofApp.h"
#include "ofxAEVisitorUtils.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	
	// Load composition
	comp_ = std::make_shared<ofx::ae::Composition>();
	if (!comp_->load("collision/comp.json")) {
		ofLogError("ofApp") << "Failed to load composition: collision/comp.json";
		return;
	}
	
	// Setup grid for collision testing
	setupGrid();
	
	ofLogNotice("ofApp") << "Simple Collision Example";
	ofLogNotice("ofApp") << "Grid: " << gridPoints_.size() << " points";
}

//--------------------------------------------------------------
void ofApp::update()
{
	if(comp_) {
		// Update timeline
		if(++timeline_ >= comp_->getInfo().duration) {
			timeline_ = 0;
		}
		if(comp_->setFrame(timeline_)) {
			comp_->update();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(40);
	
	// Draw composition
	if (comp_) {
		comp_->draw(0, 0);
	}
	auto isHit = [](const ofPath &path, glm::vec2 point) {
		if(!path.isFilled()) return false;
		auto p = path;
		p.setStrokeWidth(1);

		const auto outlines = p.getOutline();
		int hits = 0;
		for(const auto& pl : outlines) hits += pl.inside(point.x, point.y);

		switch(path.getWindingMode()) {
			case OF_POLY_WINDING_NONZERO: return hits != 0;
			case OF_POLY_WINDING_POSITIVE: return hits > 0;
			case OF_POLY_WINDING_NEGATIVE: return hits < 0;
			case OF_POLY_WINDING_ABS_GEQ_TWO: return hits >= 2;
			default:
			case OF_POLY_WINDING_ODD: return (hits % 2) != 0;
		}
	};

	auto layer = comp_->getLayer("collision");
	ofx::ae::utils::PathExtractionVisitor path;
	path.visit(*layer);
	auto &&paths = path.getPaths();
	for (const auto& point : gridPoints_) {
		bool hit = std::any_of(begin(paths), end(paths), [point,isHit](const ofPath &p) {
			return isHit(p,point);
		});
		ofSetColor(hit ? ofColor::red : ofColor::white);
		ofDrawCircle(point.x, point.y, hit ? 4 : 2);
	}
}

//--------------------------------------------------------------
void ofApp::setupGrid()
{
	gridPoints_.clear();
	
	float stepX = ofGetWidth() / (float)gridResolution_;
	float stepY = ofGetHeight() / (float)gridResolution_;
	
	for(int x = 0; x < gridResolution_; x++) {
		for(int y = 0; y < gridResolution_; y++) {
			float px = x * stepX + stepX * 0.5f;
			float py = y * stepY + stepY * 0.5f;
			gridPoints_.push_back(glm::vec2(px, py));
		}
	}
}
