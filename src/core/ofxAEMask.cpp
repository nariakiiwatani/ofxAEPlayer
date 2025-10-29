#include "ofGraphics.h"
#include "ofFbo.h"

#include "ofxAEMask.h"
#include "../data/MaskData.h"
#include "../prop/ofxAEMaskProp.h"

namespace ofx { namespace ae {

MaskPath::MaskPath() : closed(false)
{
}

MaskPath::~MaskPath()
{
}

void MaskPath::addVertex(const MaskVertex &vertex)
{
	vertices.push_back(vertex);
}

void MaskPath::addVertex(const glm::vec2 &position)
{
	vertices.push_back(MaskVertex(position));
}

void MaskPath::clear()
{
	vertices.clear();
}

glm::vec2 MaskPath::evaluateAt(float t) const
{
	if(vertices.empty()) return glm::vec2(0, 0);
	if(vertices.size() == 1) return vertices[0].position;

	int segmentCount = closed ? vertices.size() : vertices.size() - 1;
	if(segmentCount <= 0) return vertices[0].position;

	float segmentT = t * segmentCount;
	int segmentIndex = (int)segmentT;
	float localT = segmentT - segmentIndex;

	if(segmentIndex >= segmentCount) {
		segmentIndex = segmentCount - 1;
		localT = 1.0f;
	}

	int nextIndex = (segmentIndex + 1) % vertices.size();
	if(!closed && nextIndex >= vertices.size()) {
		nextIndex = vertices.size() - 1;
	}

	return glm::mix(vertices[segmentIndex].position, vertices[nextIndex].position, localT);
}

void MaskPath::generatePolyline(ofPolyline &polyline, int resolution) const
{
	polyline.clear();

	if(vertices.empty()) return;

	for(int i = 0; i <= resolution; i++) {
		float t = (float)i / resolution;
		glm::vec2 point = evaluateAt(t);
		polyline.addVertex(point.x, point.y);
	}

	if(closed) {
		polyline.setClosed(true);
	}
}

void MaskPath::setFromPathData(const PathData &pathData)
{
	clear();
	closed = pathData.closed;
	
	for(size_t i = 0; i < pathData.vertices.size(); ++i) {
		MaskVertex vertex;
		vertex.position = pathData.vertices[i];
		
		if(i < pathData.inTangents.size()) {
			vertex.inTangent = pathData.inTangents[i];
		}
		if(i < pathData.outTangents.size()) {
			vertex.outTangent = pathData.outTangents[i];
		}
		vertex.closed = closed;
		
		addVertex(vertex);
	}
}

ofPath MaskPath::toOfPath() const
{
	ofPath path;
	
	if(vertices.empty()) {
		return path;
	}
	
	if(vertices.size() == 1) {
		path.moveTo(vertices[0].position);
		return path;
	}
	
	path.moveTo(vertices[0].position);
	
	for(size_t i = 0; i < vertices.size(); ++i) {
		size_t nextIndex = (i + 1) % vertices.size();
		
		if(!closed && nextIndex == 0) {
			break;
		}
		
		const glm::vec2& currentVertex = vertices[i].position;
		const glm::vec2& nextVertex = vertices[nextIndex].position;
		
		glm::vec2 outTangent = vertices[i].outTangent;
		glm::vec2 inTangent = vertices[nextIndex].inTangent;
		
		bool hasCurve = (glm::length(outTangent) > 0.001f || glm::length(inTangent) > 0.001f);
		
		if(hasCurve) {
			glm::vec2 cp1 = currentVertex + outTangent;
			glm::vec2 cp2 = nextVertex + inTangent;
			path.bezierTo(cp1, cp2, nextVertex);
		}
		else {
			path.lineTo(nextVertex);
		}
	}
	
	if(closed) {
		path.close();
	}
	
	return path;
}

Mask::Mask()
: mode(MaskMode::ADD)
, inverted(false)
, enabled(true)
, opacity(1.f)
, expansion(0.f)
{
}

Mask::~Mask()
{
}

void Mask::renderToFbo(ofFbo &target) const
{
	if(!enabled) return;

	target.begin();
	ofClear(0, 0, 0, 0);
	ofFloatColor color{opacity,opacity,opacity,1};

	ofPushStyle();
	if(inverted) {
		ofSetColor(color);
		ofDrawRectangle(0, 0, target.getWidth(), target.getHeight());

		ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
		ofSetColor(ofFloatColor::white);
		renderPath(path);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	}
	else {
		ofSetColor(color);
		ofFill();
		renderPath(path);
	}

	ofPopStyle();
	target.end();

	if(feather.outer > 0 || feather.inner > 0) {
		applyFeather(target);
	}
}

void Mask::renderPath(const MaskPath &path) const
{
	if(path.getVertexCount() < 2) return;

	ofPath pathObj = path.toOfPath();
	pathObj.draw();
}

void Mask::setFromMaskAtomData(const MaskAtomData &atomData)
{
	MaskPath newPath;
	newPath.setFromPathData(atomData.shape);
	setPath(newPath);
	
	setFeather(MaskFeather(atomData.feather.x, atomData.feather.y));
	setOpacity(atomData.opacity);
	setExpansion(atomData.offset);
	setInverted(atomData.inverted);
	setMode(atomData.mode);
}

void Mask::applyFeather(ofFbo &target) const
{
}

ofRectangle Mask::getBounds() const
{
	if(path.getVertexCount() == 0) {
		return ofRectangle(0, 0, 0, 0);
	}

	float minX = FLT_MAX, minY = FLT_MAX;
	float maxX = -FLT_MAX, maxY = -FLT_MAX;

	for(const auto& vertex : path.getVertices()) {
		minX = std::min(minX, vertex.position.x);
		minY = std::min(minY, vertex.position.y);
		maxX = std::max(maxX, vertex.position.x);
		maxY = std::max(maxY, vertex.position.y);
	}

	return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

bool Mask::containsPoint(const glm::vec2 &point) const
{
	if(path.getVertexCount() < 3) return false;

	bool inside = false;
	const auto& vertices = path.getVertices();

	for(size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
		const glm::vec2& vi = vertices[i].position;
		const glm::vec2& vj = vertices[j].position;

		if(((vi.y > point.y) != (vj.y > point.y)) &&
			(point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x)) {
			inside = !inside;
		}
	}

	return inside;
}

MaskCollection::MaskCollection()
{
}

MaskCollection::~MaskCollection()
{
}

void MaskCollection::addMask(const Mask &mask)
{
	masks.push_back(mask);
}

void MaskCollection::removeMask(size_t index)
{
	if(index < masks.size()) {
		masks.erase(masks.begin() + index);
	}
}

void MaskCollection::clear()
{
	masks.clear();
}

bool MaskCollection::hasActiveMasks() const
{
	for(const auto& mask : masks) {
		if(mask.isEnabled()) {
			return true;
		}
	}
	return false;
}

void MaskCollection::renderCombined(ofFbo &target) const
{
	if(masks.empty()) {
		target.begin();
		ofClear(255, 255, 255, 255);
		target.end();
		return;
	}

	target.begin();
	ofClear(0, 0, 0, 0);
	target.end();

	bool isFirst = true;
	for(const auto& mask : masks) {
		if(mask.isEnabled()) {
			combineMasks(target, mask, isFirst);
			isFirst = false;
		}
	}

	if(isFirst) {
		target.begin();
		ofClear(255, 255, 255, 255);
		target.end();
	}
}

void MaskCollection::combineMasks(ofFbo &target, const Mask &mask, bool isFirst) const
{
	ofFbo maskFbo;
	maskFbo.allocate(target.getWidth(), target.getHeight(), GL_RGBA);

	mask.renderToFbo(maskFbo);

	target.begin();

	ofPushStyle();
	if(isFirst) {
		ofSetColor(ofFloatColor::white);
		maskFbo.draw(0, 0);
	}
	else {
		switch (mask.getMode()) {
			case MaskMode::ADD:
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				break;
			case MaskMode::SUBTRACT:
				ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
				break;
			case MaskMode::INTERSECT:
				ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
				break;
			default:
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				break;
		}

		ofSetColor(ofFloatColor::white);
		maskFbo.draw(0, 0);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	}
	ofPopStyle();
	target.end();
}

void MaskCollection::setupFromMaskProp(const MaskProp &maskProp)
{
	clear();
	
	std::vector<MaskAtomData> atoms;
	maskProp.tryExtract(atoms);
	for(const auto& atomData : atoms) {
		if(!atomData.shape.vertices.empty()) {
			Mask mask;
			mask.setFromMaskAtomData(atomData);
			addMask(mask);
		}
	}
}

}} // namespace ofx::ae
