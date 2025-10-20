#include "ofxAEVisitorUtils.h"
#include "ofxAELayer.h"
#include "ofxAETransformProp.h"

namespace ofx { namespace ae { namespace utils {

PathExtractionVisitor::PathExtractionVisitor()
{
	pushContext();
}

void PathExtractionVisitor::visit(const Layer& layer) {
	pushContext();
	TransformData t;
	if(layer.tryExtractTransform(t)) {
		getContext().transform(t);
	}
	Visitor::visit(layer);
	popContext();
}

void PathExtractionVisitor::visit(const EllipseData& ellipse) {

	auto p = utils::ShapePathGenerator::createEllipsePath(ellipse);
	applyTransform(p);
	getContext().path.append(p);
	Visitor::visit(ellipse);
}

void PathExtractionVisitor::visit(const RectangleData& rectangle) {
	auto p = utils::ShapePathGenerator::createRectanglePath(rectangle);
	applyTransform(p);
	getContext().path.append(p);
	Visitor::visit(rectangle);
}

void PathExtractionVisitor::visit(const PolygonData& polygon) {
	auto p = utils::ShapePathGenerator::createPolygonPath(polygon);
	applyTransform(p);
	getContext().path.append(p);
	Visitor::visit(polygon);
}

void PathExtractionVisitor::visit(const FillData& fill) {
	ofPath p = getContext().path;

	p.setFillColor(fill.color);
	p.setFilled(true);

	switch(fill.rule) {
		case 1: p.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
		case 2: p.setPolyWindingMode(OF_POLY_WINDING_ODD); break;
		default: p.setPolyWindingMode(OF_POLY_WINDING_NONZERO); break;
	}

	if (fill.compositeOrder == 1) {
		result_.push_front(p);
	} else {
		result_.push_back(p);
	}
	Visitor::visit(fill);
}

void PathExtractionVisitor::visit(const StrokeData& stroke) {
	ofPath p = getContext().path;

	p.setStrokeColor(stroke.color);
	p.setStrokeWidth(stroke.width);

	if (stroke.compositeOrder == 1) {
		result_.push_front(p);
	} else {
		result_.push_back(p);
	}
	Visitor::visit(stroke);
}

void PathExtractionVisitor::visit(const GroupData& group) {
	pushContext();
	getContext().transform(group.transform);
	Visitor::visit(group);
	popContext();
}


void PathExtractionVisitor::clear() {
	result_.clear();
	ctx_ = std::stack<Context>();
	ctx_.push(Context());
}

void PathExtractionVisitor::applyTransform(ofPath& path) const {
	if (ctx_.empty()) return;

	float opacity = getContext().opacity;
	auto fc = path.getFillColor(); fc.a *= opacity; path.setFillColor(fc);
	auto sc = path.getStrokeColor(); sc.a *= opacity; path.setStrokeColor(sc);

	const ofMatrix4x4& transform = ctx_.top().mat;
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

void PathExtractionVisitor::pushContext()
{
	if(ctx_.empty()) {
		ctx_.push(Context{});
	}
	else {
		Context new_top = getContext();
		new_top.path.clear();
		ctx_.push(new_top);
	}
}
void PathExtractionVisitor::popContext()
{
	auto path = getContext().path;
	ctx_.pop();
	getContext().path.append(path);
}
PathExtractionVisitor::Context& PathExtractionVisitor::getContext()
{
	return ctx_.top();
}
const PathExtractionVisitor::Context& PathExtractionVisitor::getContext() const
{
	return ctx_.top();
}
}}} // namespace ofx::ae::utils
