#include "ofxAEVisitorUtils.h"
#include "ofxAELayer.h"
#include "ofxAETransformProp.h"
#include "ofxAEFillRule.h"

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

void PathExtractionVisitor::visit(const PathData& pathData) {
	auto p = utils::ShapePathGenerator::createPath(pathData);
	applyTransform(p);
	getContext().path.append(p);
	Visitor::visit(pathData);
}

void PathExtractionVisitor::visit(const FillData& fill) {
	ofPath p = getContext().path;

	p.setFillColor(fill.color);
	p.setFilled(true);

	FillRule rule = static_cast<FillRule>(fill.rule);
	ofPolyWindingMode mode = (rule == FillRule::EVEN_ODD) ?
		OF_POLY_WINDING_ODD : OF_POLY_WINDING_NONZERO;
	p.setPolyWindingMode(mode);

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

// BlendModeAwarePathVisitor implementation
BlendModeAwarePathVisitor::BlendModeAwarePathVisitor()
	: PathExtractionVisitor()
{
}

void BlendModeAwarePathVisitor::visit(const GroupData& group) {
	if (group.requiresFBO()) {
		// Special blend mode - requires FBO rendering
		// This method is called when we need to handle group-level blend modes
		// The actual drawing will be handled by drawGroup method
		pushContext();
		getContext().transform(group.transform);
		Visitor::visit(group);
		popContext();
	} else {
		// Normal mode - use inherited PathExtractionVisitor behavior
		PathExtractionVisitor::visit(group);
	}
}

void BlendModeAwarePathVisitor::drawGroup(const GroupData& group, int width, int height) {
	if (group.blendMode == BlendMode::NORMAL) { // Normal mode
		// Direct drawing path (no FBO needed)
		pushContext();
		getContext().transform(group.transform);
		drawGroupContents(group);
		popContext();
	} else {
		// Special blend mode - use FBO caching
		group.ensureFBO(width, height);
		
		if (group.needs_update_) {
			// Save current state
			ofPushStyle();
			ofPushMatrix();
			
			// Render group contents to FBO
			group.cached_fbo_->begin();
			ofClear(0, 0, 0, 0); // Clear with transparent background
			
			// Reset transform for FBO-local rendering
			ofLoadIdentityMatrix();
			
			pushContext();
			getContext().transform(group.transform);
			
			// Create a temporary visitor for FBO rendering
			PathExtractionVisitor fboVisitor;
			for (const auto& child : group.data) {
				if (child) {
					child->accept(fboVisitor);
				}
			}
			
			// Draw paths to FBO
			auto fboPaths = fboVisitor.getPaths();
			for(const auto& path : fboPaths) {
				path.draw();
			}
			
			popContext();
			group.cached_fbo_->end();
			group.needs_update_ = false;
			
			// Restore state
			ofPopMatrix();
			ofPopStyle();
		}
		
		// The FBO is ready for blend mode application by the caller
		// ShapeSource::drawWithBlendModes will handle the actual blend mode application
	}
}

void BlendModeAwarePathVisitor::drawGroupContents(const GroupData& group) {
	// For normal mode groups, process children and accumulate paths
	for (const auto& child : group.data) {
		if (child) {
			child->accept(*this);
		}
	}
	
	// Paths are automatically accumulated in result_ by the base PathExtractionVisitor
	// when child elements call visit methods for shapes, fills, and strokes
}

}}} // namespace ofx::ae::utils
