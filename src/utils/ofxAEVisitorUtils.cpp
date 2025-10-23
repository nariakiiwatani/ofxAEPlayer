#include "ofxAEVisitorUtils.h"
#include "ofxAELayer.h"
#include "ofxAETransformProp.h"
#include "ofxAEFillRule.h"

namespace ofx { namespace ae {

PathExtractionVisitor::PathExtractionVisitor()
{
}

PathExtractionVisitor::PathExtractionVisitor(const GroupData &group)
: PathExtractionVisitor()
{
	renderer_.transform = group.transform.toOf();
	renderer_.opacity = group.transform.opacity;
	renderer_.blend_mode = group.blendMode;
	visitChildren(group);
}

void PathExtractionVisitor::visit(const EllipseData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	auto bb = utils::ShapePathGenerator::getBoundingBox(data);
	if(bb) {
		if(!bounding_box_.isEmpty()) bounding_box_.growToInclude(*bb);
		else bounding_box_ = *bb;
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const RectangleData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	auto bb = utils::ShapePathGenerator::getBoundingBox(data);
	if(bb) {
		if(!bounding_box_.isEmpty()) bounding_box_.growToInclude(*bb);
		else bounding_box_ = *bb;
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const PolygonData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	auto bb = utils::ShapePathGenerator::getBoundingBox(data);
	if(bb) {
		if(!bounding_box_.isEmpty()) bounding_box_.growToInclude(*bb);
		else bounding_box_ = *bb;
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const PathData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	auto bb = utils::ShapePathGenerator::getBoundingBox(data);
	if(bb) {
		if(!bounding_box_.isEmpty()) bounding_box_.growToInclude(*bb);
		else bounding_box_ = *bb;
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const FillData& data) {
	ofPath p = path_;

	p.setFillColor(data.color);
	p.setFilled(true);

	p.setPolyWindingMode(toOf(data.rule));

	auto item = std::make_shared<RenderPathItem>(p);
	item->blend_mode = data.blendMode;
	item->bounding_box = bounding_box_;

	if (data.compositeOrder == 1) {
		renderer_.item.push_front(item);
	} else {
		renderer_.item.push_back(item);
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const StrokeData& data) {
	ofPath p = path_;

	p.setStrokeColor(data.color);
	p.setStrokeWidth(data.width);

	auto item = std::make_shared<RenderPathItem>(p);
	item->blend_mode = data.blendMode;
	item->bounding_box = bounding_box_;

	if (data.compositeOrder == 1) {
		renderer_.item.push_front(item);
	} else {
		renderer_.item.push_back(item);
	}
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const GroupData& group) {
	PathExtractionVisitor visitor(group);
	auto item = std::make_shared<RenderGroupItem>(visitor.getRenderer());
	auto bb = item->getBB();
	if(!bb.isEmpty()) {
		if(!bounding_box_.isEmpty()) bounding_box_.growToInclude(bb);
		else bounding_box_ = bb;
	}
	renderer_.item.push_front(std::move(item));
	path_.append(visitor.getPath());
}


void PathExtractionVisitor::RenderPathItem::draw(float alpha) const
{
	ofPushMatrix();
	ofPushStyle();
	ofMultMatrix(transform);
	applyBlendMode(blend_mode);
	float opacity = this->opacity*alpha;
	auto mulOpacity = [](const ofFloatColor src, float opacity) {
		return ofFloatColor{src.r, src.g, src.b, src.a*opacity};
	};
	auto p = path;
	if(p.isFilled()) p.setFillColor(mulOpacity(p.getFillColor(), opacity));
	if(p.hasOutline()) p.setStrokeColor(mulOpacity(p.getStrokeColor(), opacity));
	p.draw();

	ofPopStyle();
	ofPopMatrix();
}

bool PathExtractionVisitor::RenderGroupItem::needFbo() const
{
	return std::any_of(begin(item), end(item), [](const shared_ptr<RenderItem> i) {
		return i->blend_mode != BlendMode::NORMAL;
	});
}

void PathExtractionVisitor::RenderGroupItem::draw(float alpha) const
{
	if(needFbo()) {
		auto bb = getBB();
		if(bb.isEmpty()) return;
		fbo.allocate(bb.width, bb.height, GL_RGBA);
		fbo.begin();
		ofClear(0,0);
		ofPushMatrix();
		ofTranslate(-bb.x, -bb.y);
		for(auto &&i : item) {
			i->draw(this->opacity*alpha);
		}
		ofPopMatrix();
		fbo.end();
		ofPushMatrix();
		ofPushStyle();
		ofMultMatrix(transform);
		applyBlendMode(blend_mode);
		fbo.draw(bb.x,bb.y);
		ofPopStyle();
		ofPopMatrix();
	}
	else {
		ofPushMatrix();
		ofPushStyle();
		ofMultMatrix(transform);
		applyBlendMode(blend_mode);
		for(auto &&i : item) {
			i->draw(this->opacity*alpha);
		}
		ofPopStyle();
		ofPopMatrix();
	}
}

namespace {
static inline glm::vec2 applyMat2D(const ofMatrix4x4& M, float x, float y){
	glm::vec4 h = ofVec4f(x, y, 0.0f, 1.0f) * M;
	if(h.w != 0.0f){ h.x /= h.w; h.y /= h.w; }
	return {h.x, h.y};
}

ofRectangle getTransformed(const ofRectangle &src, const ofMatrix4x4 &transform)
{
	const float x0 = src.getX();
	const float y0 = src.getY();
	const float x1 = x0 + src.getWidth();
	const float y1 = y0 + src.getHeight();

	const glm::vec2 p0 = applyMat2D(transform, x0, y0);
	const glm::vec2 p1 = applyMat2D(transform, x1, y0);
	const glm::vec2 p2 = applyMat2D(transform, x1, y1);
	const glm::vec2 p3 = applyMat2D(transform, x0, y1);

	const float minx = std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x));
	const float maxx = std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x));
	const float miny = std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y));
	const float maxy = std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y));

	return ofRectangle(minx, miny, maxx - minx, maxy - miny);
}}
ofRectangle PathExtractionVisitor::RenderGroupItem::getBB() const
{
	ofRectangle ret;
	for(auto &&i : item) {
		auto rect = i->getBB();
		if(!rect.isEmpty()) {
			if(!ret.isEmpty()) ret.growToInclude(rect);
			else ret = rect;
		}
	}
	if(!ret.isEmpty()) {
		ret = getTransformed(ret, transform);
	}
	return ret;
}

ofRectangle PathExtractionVisitor::RenderPathItem::getBB() const
{
	return bounding_box;
}


}} // namespace ofx::ae::utils
