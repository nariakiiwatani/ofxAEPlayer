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
		path_.append(createEllipsePath(shape));
	}
	void visit(const ofx::ae::RectangleData& shape) {
		path_.append(createRectanglePath(shape));
	}
	void visit(const ofx::ae::PolygonData& shape) {}
	void visit(const ofx::ae::FillData& fill) {
		switch(fill.rule) {
			case 1: path_.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
			case 2: path_.setPolyWindingMode(OF_POLY_WINDING_ODD); break;
		}
		path_.setStrokeWidth(1);
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
	static float signedArea(const ofPolyline& pl){
		const auto& v = pl.getVertices();
		if(v.size() < 3) return 0.f;
		double a = 0.0;
		for(size_t i=0, j=v.size()-1; i<v.size(); j=i++){
			a += (double)v[j].x * v[i].y - (double)v[i].x * v[j].y;
		}
		return (float)(0.5 * a); // >0 ならCCW
	}

	static ofPath enforceWinding(const ofPath& src, int direction){
		int desiredSign = 1;
		switch(direction){
		  case 2: desiredSign = 1; break;
		  case 3: desiredSign = -1; break;
		}
		ofPath out;
		out.setMode(src.getMode());
		out.setCurveResolution(src.getCurveResolution());
		for(const auto& poly : src.getOutline()){
			auto v = poly.getVertices();
			if(v.size() < 3) continue;
			bool ccw = signedArea(poly) > 0;
			if((ccw ? +1 : -1) != desiredSign){
				std::reverse(v.begin(), v.end());
			}
			out.moveTo(v[0]);
			for(size_t i=1;i<v.size();++i) out.lineTo(v[i]);
			out.close();
		}
		return out;
	}
	ofPath createEllipsePath(const ofx::ae::EllipseData &e){
		ofPath path;
		const float cx=e.position.x, cy=e.position.y;
		path.ellipse(cx, cy, e.size.x, e.size.y);
		return enforceWinding(path, e.direction);
	}

	ofPath createRectanglePath(const ofx::ae::RectangleData &r){
		ofPath path;
		float x=r.position.x - r.size.x*0.5f;
		float y=r.position.y - r.size.y*0.5f;
		if(r.roundness>0) path.rectRounded(x,y,r.size.x,r.size.y,r.roundness);
		else              path.rectangle(x,y,r.size.x,r.size.y);
		return enforceWinding(path, r.direction);
	}


	ofPath createPolygonPath(const ofx::ae::PolygonData &polygon) {
		ofPath path;

		int numPoints = polygon.points;
		if (numPoints < 3) {
			return path; // Invalid polygon
		}

		bool isStar = (polygon.type == 2);
		float outerRadius = polygon.outerRadius;
		float innerRadius = isStar ? polygon.innerRadius : outerRadius;

		float angleStep = TWO_PI / numPoints;
		float startAngle = polygon.rotation * DEG_TO_RAD;

		bool firstPoint = true;
		for (int i = 0; i < numPoints; i++) {
			float angle = startAngle + i * angleStep;

			float pointX = polygon.position.x + cos(angle) * outerRadius;
			float pointY = polygon.position.y + sin(angle) * outerRadius;

			if (firstPoint) {
				path.moveTo(pointX, pointY);
				firstPoint = false;
			} else {
				path.lineTo(pointX, pointY);
			}

			if (isStar) {
				float innerAngle = angle + angleStep * 0.5f;
				float innerPointX = polygon.position.x + cos(innerAngle) * innerRadius;
				float innerPointY = polygon.position.y + sin(innerAngle) * innerRadius;
				path.lineTo(innerPointX, innerPointY);
			}
		}
		path.close();

		path = enforceWinding(path, polygon.direction);
		return path;
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
		int hits = 0;
		for(const auto& pl : outlines) hits += pl.inside(pos.x, pos.y);

		switch(path.getWindingMode()) {
			case OF_POLY_WINDING_NONZERO: return hits != 0;
			case OF_POLY_WINDING_POSITIVE: return hits > 0;
			case OF_POLY_WINDING_NEGATIVE: return hits < 0;
			case OF_POLY_WINDING_ABS_GEQ_TWO: return hits >= 2;
			default:
			case OF_POLY_WINDING_ODD: return (hits % 2) != 0;
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
	for(int x = 0; x < ofGetWidth(); x += 500) {
		for(int y = 0; y < ofGetHeight(); y += 500) {
			bool is_hit = path.isHit({x, y});
			ofPushStyle();
			ofSetColor(is_hit?ofColor::red:ofColor::white);
			ofDrawCircle(x, y, is_hit?10:5);
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
