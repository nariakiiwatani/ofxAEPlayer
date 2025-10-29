#include "PathData.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {
void EllipseData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void RectangleData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void PolygonData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void PathData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void FillData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void StrokeData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void GroupData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

void ShapeData::accept(Visitor& visitor) const
{
	visitor.visit(*this);
}

PathData PathData::operator+(const PathData& other) const
{
	PathData result = *this;
	if(vertices.size() == other.vertices.size()) {
		for(size_t i = 0; i < vertices.size(); ++i) {
			result.vertices[i] += other.vertices[i];
			result.inTangents[i] += other.inTangents[i];
			result.outTangents[i] += other.outTangents[i];
		}
	}
	return result;
}

PathData PathData::operator-(const PathData& other) const
{
	PathData result = *this;
	if(vertices.size() == other.vertices.size()) {
		for(size_t i = 0; i < vertices.size(); ++i) {
			result.vertices[i] -= other.vertices[i];
			result.inTangents[i] -= other.inTangents[i];
			result.outTangents[i] -= other.outTangents[i];
		}
	}
	return result;
}

PathData PathData::operator*(float t) const
{
	PathData result = *this;
	for(size_t i = 0; i < vertices.size(); ++i) {
		result.vertices[i] *= t;
		result.inTangents[i] *= t;
		result.outTangents[i] *= t;
	}
	return result;
}

ofPath PathData::toOfPath() const
{
	ofPath path;

	if(vertices.empty()) {
		return path;
	}

	size_t numVertices = vertices.size();
	size_t numInTangents = inTangents.size();
	size_t numOutTangents = outTangents.size();

	path.moveTo(vertices[0]);

	for(size_t i = 0; i < numVertices; i++) {
		size_t nextIndex = (i + 1) % numVertices;

		if(!closed && nextIndex == 0) {
			break;
		}

		glm::vec2 currentVertex = vertices[i];
		glm::vec2 nextVertex = vertices[nextIndex];

		glm::vec2 outTangent = (i < numOutTangents) ? outTangents[i] : glm::vec2(0, 0);
		glm::vec2 inTangent = (nextIndex < numInTangents) ? inTangents[nextIndex] : glm::vec2(0, 0);

		glm::vec2 cp1 = currentVertex + outTangent;
		glm::vec2 cp2 = nextVertex + inTangent;

		bool hasCurve = (outTangent.x != 0 || outTangent.y != 0 ||
						inTangent.x != 0 || inTangent.y != 0);

		if(hasCurve) {
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

}}
