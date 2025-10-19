#include "ofxAEVisitorUtils.h"
#include "ofxAELayer.h"
#include "ofxAETransformProp.h"

namespace ofx { namespace ae { namespace utils {

void PathExtractionVisitor::visit(const Layer& layer) {
	TransformData t;
	if(layer.tryExtractTransform(t)) {
		pushTransform(t);
	} else {
		pushTransform(TransformData());
	}
	Visitor::visit(layer);
	popTransform();
}

void PathExtractionVisitor::visit(const EllipseData& ellipse) {
	path_.append(utils::ShapePathGenerator::createEllipsePath(ellipse));
	Visitor::visit(ellipse);
}

void PathExtractionVisitor::visit(const RectangleData& rectangle) {
	path_.append(utils::ShapePathGenerator::createRectanglePath(rectangle));
	Visitor::visit(rectangle);
}

void PathExtractionVisitor::visit(const PolygonData& polygon) {
	path_.append(utils::ShapePathGenerator::createPolygonPath(polygon));
	Visitor::visit(polygon);
}

void PathExtractionVisitor::visit(const FillData& fill) {
	ofPath p = path_;

	p.setFillColor(fill.color);
	p.setFilled(true);
	p.setStrokeWidth(0);

	switch(fill.rule) {
		case 1: p.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
		case 2: p.setPolyWindingMode(OF_POLY_WINDING_ODD); break;
		default: p.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
	}

	applyTransform(p);

	if (fill.compositeOrder == 1) {
		result_.push_front(p);
	} else {
		result_.push_back(p);
	}
	Visitor::visit(fill);
}

void PathExtractionVisitor::visit(const StrokeData& stroke) {
	ofPath p = path_;

	p.setStrokeColor(stroke.color);
	p.setStrokeWidth(stroke.width);
	p.setFilled(false);

	applyTransform(p);

	if (stroke.compositeOrder == 1) {
		result_.push_front(p);
	} else {
		result_.push_back(p);
	}
	Visitor::visit(stroke);
}

void PathExtractionVisitor::clear() {
	result_.clear();
	path_.clear();
	transform_ = std::stack<Transform>();
}

void PathExtractionVisitor::applyTransform(ofPath& path) const {
	if (transform_.empty()) return;

	float opacity = transform_.top().opacity;
	auto fc = path.getFillColor(); fc.a *= opacity; path.setFillColor(fc);
	auto sc = path.getStrokeColor(); sc.a *= opacity; path.setStrokeColor(sc);

	const ofMatrix4x4& transform = transform_.top().mat;
	if (transform.isIdentity()) {
		return;
	}

	glm::vec3 trans = transform.getTranslation();
	glm::vec3 scale = transform.getScale();
	ofVec3f rot_dir = transform * ofVec3f{1,0,0};
	float rot = atan2(-rot_dir.y, rot_dir.x);

	path.scale(scale.x, scale.y);
	path.rotateRad(rot, {0,0,1});
	path.translate(trans);
}

void PathExtractionVisitor::pushTransform(const TransformData& t) {
	if (transform_.empty()) {
		transform_.push({t.toOf(), t.opacity});
	} else {
		transform_.push(transform_.top()*t);
	}
}

void PathExtractionVisitor::popTransform() {
	if (transform_.size() > 1) {
		transform_.pop();
	}
	// Always keep at least the identity transform
}

}}} // namespace ofx::ae::utils
