#pragma once

#include "ofMain.h"
#include "ofxAEMaskProp.h"
#include <vector>

namespace ofx { namespace ae {

enum class MaskMode {
	ADD,
	SUBTRACT,
	INTERSECT,
	LIGHTEN,
	DARKEN,
	DIFFERENCE
};

struct MaskFeather {
	float inner;
	float outer;

	MaskFeather() : inner(0.0f), outer(0.0f) {}
	MaskFeather(float i, float o) : inner(i), outer(o) {}
};

struct MaskVertex {
	glm::vec2 position;
	glm::vec2 inTangent;
	glm::vec2 outTangent;
	bool closed;

	MaskVertex() : position(0, 0), inTangent(0, 0), outTangent(0, 0), closed(false) {}
	MaskVertex(const glm::vec2& pos) : position(pos), inTangent(0, 0), outTangent(0, 0), closed(false) {}
};

class MaskPath {
public:
	MaskPath();
	~MaskPath();

	void addVertex(const MaskVertex& vertex);
	void addVertex(const glm::vec2& position);
	void clear();

	void setClosed(bool closed) { this->closed = closed; }
	bool isClosed() const { return closed; }

	const std::vector<MaskVertex>& getVertices() const { return vertices; }
	std::vector<MaskVertex>& getVertices() { return vertices; }

	size_t getVertexCount() const { return vertices.size(); }

	glm::vec2 evaluateAt(float t) const;
	void generatePolyline(ofPolyline& polyline, int resolution = 100) const;
	
	void setFromPathData(const PathData& pathData);
	ofPath toOfPath() const;

private:
	std::vector<MaskVertex> vertices;
	bool closed;
};

class Mask {
public:
	Mask();
	~Mask();

	void setMode(MaskMode mode) { this->mode = mode; }
	MaskMode getMode() const { return mode; }

	void setInverted(bool inverted) { this->inverted = inverted; }
	bool isInverted() const { return inverted; }

	void setEnabled(bool enabled) { this->enabled = enabled; }
	bool isEnabled() const { return enabled; }

	void setOpacity(float opacity) { this->opacity = ofClamp(opacity, 0.0f, 100.0f); }
	float getOpacity() const { return opacity; }

	void setFeather(const MaskFeather& feather) { this->feather = feather; }
	const MaskFeather& getFeather() const { return feather; }

	void setPath(const MaskPath& path) { this->path = path; }
	const MaskPath& getPath() const { return path; }
	MaskPath& getPath() { return path; }
	
	void setFromMaskAtomData(const MaskAtomData& atomData);

	void setExpansion(float expansion) { this->expansion = expansion; }
	float getExpansion() const { return expansion; }

	void renderToFbo(ofFbo& target) const;
	ofPath toOfPath() const { return path.toOfPath(); }

	ofRectangle getBounds() const;
	bool containsPoint(const glm::vec2& point) const;

private:
	MaskMode mode;
	bool inverted;
	bool enabled;
	float opacity;
	MaskFeather feather;
	MaskPath path;
	float expansion;

	void renderPath(const MaskPath& path) const;
	void applyFeather(ofFbo& target) const;
};

class MaskCollection {
public:
	MaskCollection();
	~MaskCollection();

	void addMask(const Mask& mask);
	void removeMask(size_t index);
	void clear();

	size_t getMaskCount() const { return masks.size(); }
	const Mask& getMask(size_t index) const { return masks[index]; }
	Mask& getMask(size_t index) { return masks[index]; }

	bool hasActiveMasks() const;

	void renderCombined(ofFbo& target, int width, int height) const;
	
	void setupFromMaskProp(const MaskProp& maskProp);
	bool empty() const { return masks.empty(); }
	auto begin() const { return masks.begin(); }
	auto end() const { return masks.end(); }

private:
	std::vector<Mask> masks;

	void combineMasks(ofFbo& target, const Mask& mask, bool isFirst) const;
};

}} // namespace ae::ofx
