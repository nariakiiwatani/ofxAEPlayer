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
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const RectangleData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const PolygonData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const PathData& data) {
	path_.append(utils::ShapePathGenerator::createPath(data));
	Visitor::visit(data);
}

void PathExtractionVisitor::visit(const FillData& data) {
	ofPath p = path_;

	p.setFillColor(data.color);
	p.setFilled(true);

	p.setPolyWindingMode(toOf(data.rule));

	auto item = std::make_shared<RenderPathItem>(p);
	item->blend_mode = data.blendMode;

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
		fbo.allocate(1280, 720, GL_RGBA);
		fbo.begin();
		ofClear(0,0);
		ofPushMatrix();
		ofTranslate(640,360);
		for(auto &&i : item) {
			i->draw(this->opacity*alpha);
		}
		ofPopMatrix();
		fbo.end();
		ofPushMatrix();
		ofPushStyle();
		ofMultMatrix(transform);
		applyBlendMode(blend_mode);
		fbo.draw(-640,-360);
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


}} // namespace ofx::ae::utils
