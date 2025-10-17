#include "ofApp.h"

#include "ofxAEShapeProp.h"
#include "ofxAEShapeSource.h"
#include "ofxAEContentVisitor.h"
#include "ofxAELayer.h"

namespace {
class ExtractPath : public ofx::ae::ShapeVisitor
{
public:
	void visit(const ofx::ae::Layer &layer) {
		auto &&mat = layer.getWorldMatrix();
		layer.getSource()->accept(*this);
		glm::vec3 trans = mat->getTranslation();
		glm::vec3 scale = mat->getScale();
		ofVec3f rot_dir = *mat * ofVec3f{1,0,0};
		float rot = atan2(-rot_dir.y, rot_dir.x);

		for(auto &&p : filled_) {
			p.scale(scale.x, scale.y);
			p.rotateRad(rot, {0,0,1});
			p.translate(trans);
		}
	}
	void visit(const ofx::ae::ShapeSource &source) {
		ofx::ae::ShapeData data;
		if(source.tryExtract(data)) {
			data.accept(*this);
		}
	}
	void visit(const ofx::ae::EllipseData& shape) {
		path_.append(createPath(shape));
	}
	void visit(const ofx::ae::RectangleData& shape) {
		path_.append(createPath(shape));
	}
	void visit(const ofx::ae::PolygonData& shape) {}
	void visit(const ofx::ae::FillData& fill) {
		switch(fill.rule) {
			case 1: path_.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
			case 2: path_.setPolyWindingMode(OF_POLY_WINDING_ODD); break;
		}
		filled_.push_back(path_);
	}
	void visit(const ofx::ae::StrokeData& stroke) {}
	bool isHit(const glm::vec2& pos) const {
		return std::any_of(begin(filled_), end(filled_), [this,pos](const ofPath &p) {
			return isHit(p, pos);
		});
	}
private:
	ofPath path_;
	std::vector<ofPath> filled_;
	// TODO: DRY (ofxAEShapeSpource.cpp)
	ofPath createPath(const ofx::ae::EllipseData &ellipse) {
		ofPath path;
		float ellipseX = ellipse.position.x - ellipse.size.x * 0.5f;
		float ellipseY = ellipse.position.y - ellipse.size.y * 0.5f;
		path.ellipse(ellipseX + ellipse.size.x * 0.5f, ellipseY + ellipse.size.y * 0.5f, ellipse.size.x, ellipse.size.y);

		if (ellipse.direction < 0) {
			path = reversePath(path);
		}
		return path;
	}
	// TODO: DRY (ofxAEShapeSpource.cpp)
	ofPath createPath(const ofx::ae::RectangleData &rectangle) {
		ofPath path;
		float rectX = rectangle.position.x - rectangle.size.x * 0.5f;
		float rectY = rectangle.position.y - rectangle.size.y * 0.5f;

		if (rectangle.roundness > 0) {
			path.rectRounded(rectX, rectY, rectangle.size.x, rectangle.size.y, rectangle.roundness);
		} else {
			path.rectangle(rectX, rectY, rectangle.size.x, rectangle.size.y);
		}

		if (rectangle.direction < 0) {
		//	path = reversePath(path);
		}
		return path;
	}
	ofPath reversePath(const ofPath &path) {
		ofPath reversedPath;
		const auto& outlines = path.getOutline();

		for (const auto& outline : outlines) {
			if (outline.size() < 2) continue;

			auto vertices = outline.getVertices();
			if (vertices.empty()) continue;

			reversedPath.moveTo(vertices.back());
			for (int i = vertices.size() - 2; i >= 0; i--) {
				reversedPath.lineTo(vertices[i]);
			}
			reversedPath.close();
		}

		return reversedPath;
	}
	int evenOddHit(const ofPolyline& poly, const glm::vec2& p) const {
		const auto& v = poly.getVertices();
		if(v.size()<2) return 0;
		bool inside=false;
		for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
			bool c = ((v[i].y>p.y)!=(v[j].y>p.y)) &&
					 (p.x < (v[j].x-v[i].x)*(p.y-v[i].y)/(v[j].y-v[i].y + 0.0f) + v[i].x);
			if(c) inside = !inside;
		}
		return inside?1:0;
	}

	int windingHit(const ofPolyline& poly, const glm::vec2& p) const {
		const auto& v = poly.getVertices();
		if(v.size()<2) return 0;
		int w = 0;
		for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
			bool up   = v[j].y <= p.y && v[i].y >  p.y;
			bool down = v[j].y >  p.y && v[i].y <= p.y;
			if(up || down){
				float x = v[j].x + (p.y - v[j].y) * (v[i].x - v[j].x) / (v[i].y - v[j].y);
				if(x > p.x) w += up ? +1 : -1;
			}
		}
		return w; // 0なら外、非0なら内
	}
	bool isHit(const ofPath &path, glm::vec2 pos) const {
		const auto outlines = path.getOutline();

		if(path.getWindingMode()==OF_POLY_WINDING_ODD){ // 偶奇
			int hits = 0;
			for(const auto& pl : outlines) hits += evenOddHit(pl, pos);
			return (hits % 2) != 0;
		}else{ // OF_POLY_WINDING_NONZERO 他はNonZero相当で処理
			int wsum = 0;
			for(const auto& pl : outlines) wsum += windingHit(pl, pos);
			return wsum != 0;
		}
	}
};
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	comp_ = std::make_shared<ofx::ae::Composition>();
	comp_->load("collision/comp.json");
}

//--------------------------------------------------------------
void ofApp::update(){
	if(comp_) {
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
	comp_->draw(0,0);
	ExtractPath path;
	if(auto &&layer = comp_->getLayer("collision")) {
		layer->accept(path);
	}
	for(int x = 0; x < ofGetWidth(); x += 50) {
		for(int y = 0; y < ofGetHeight(); y += 50) {
			bool is_hit = path.isHit({x, y});
			ofPushStyle();
			ofSetColor(is_hit?ofColor::red:ofColor::white);
			ofDrawCircle(x, y, 10);
			ofPopStyle();
		}
	}
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
