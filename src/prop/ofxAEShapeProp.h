#pragma once

#include "ofxAEProperty.h"

namespace ofx { namespace ae {

struct ShapeDataBase {
};
struct EllipseData : public ShapeDataBase {
	glm::vec2 size;
	glm::vec2 position;
	int direction = 1; // 1 = clockwise, -1 = counterclockwise
};

struct RectangleData : public ShapeDataBase {
	glm::vec2 size;
	glm::vec2 position;
	float roundness;
	int direction = 1; // 1 = clockwise, -1 = counterclockwise
};

struct PolygonData : public ShapeDataBase {
	int direction = 1;
	int type = 1; // 1 = polygon, 2 = star
	int points = 5;
	glm::vec2 position;
	float rotation = 0;
	float innerRadius = 50;
	float outerRadius = 100;
	float innerRoundness = 0;
	float outerRoundness = 0;
};

struct FillData : public ShapeDataBase {
	glm::vec3 color;
	float opacity;
	int rule = 1; // Fill rule
	int blendMode = 1;
	int compositeOrder = 1;
};

struct StrokeData : public ShapeDataBase {
	glm::vec3 color;
	float opacity;
	float width;
	int lineCap = 1;
	int lineJoin = 1;
	float miterLimit = 4;
	int blendMode = 1;
	int compositeOrder = 1;
};

using ShapeData = std::vector<std::unique_ptr<ShapeDataBase>>;

struct GroupData : public ShapeDataBase {
	int blendMode = 1;
	ShapeData data;
};


class EllipseProp : public PropertyGroup_<EllipseData>
{
public:
	EllipseProp() {
		registerProperty<VecProp<2>>("/size");
		registerProperty<VecProp<2>>("/position");
		registerProperty<IntProp>("/direction");
	}
	void extract(EllipseData &e) const override {
		getProperty<VecProp<2>>("/size")->get(e.size);
		getProperty<VecProp<2>>("/position")->get(e.position);
		getProperty<IntProp>("/direction")->get(e.direction);
	}
};

class RectangleProp : public PropertyGroup_<RectangleData>
{
public:
	RectangleProp() {
		registerProperty<VecProp<2>>("/size");
		registerProperty<VecProp<2>>("/position");
		registerProperty<FloatProp>("/roundness");
		registerProperty<IntProp>("/direction");
	}
	void extract(RectangleData &r) const override {
		getProperty<VecProp<2>>("/size")->get(r.size);
		getProperty<VecProp<2>>("/position")->get(r.position);
		getProperty<FloatProp>("/roundness")->get(r.roundness);
		getProperty<IntProp>("/direction")->get(r.direction);
	}
};

class PolygonProp : public PropertyGroup_<PolygonData>
{
public:
	PolygonProp() {
		registerProperty<IntProp>("/direction");
		registerProperty<IntProp>("/type");
		registerProperty<IntProp>("/points");
		registerProperty<VecProp<2>>("/position");
		registerProperty<FloatProp>("/rotation");
		registerProperty<FloatProp>("/innerRadius");
		registerProperty<FloatProp>("/outerRadius");
		registerProperty<FloatProp>("/innerRoundness");
		registerProperty<FloatProp>("/outerRoundness");
	}
	void extract(PolygonData &p) const override {
		getProperty<IntProp>("/direction")->get(p.direction);
		getProperty<IntProp>("/type")->get(p.type);
		getProperty<IntProp>("/points")->get(p.points);
		getProperty<VecProp<2>>("/position")->get(p.position);
		getProperty<FloatProp>("/rotation")->get(p.rotation);
		getProperty<FloatProp>("/innerRadius")->get(p.innerRadius);
		getProperty<FloatProp>("/outerRadius")->get(p.outerRadius);
		getProperty<FloatProp>("/innerRoundness")->get(p.innerRoundness);
		getProperty<FloatProp>("/outerRoundness")->get(p.outerRoundness);
	}
};

class GroupProp : public PropertyGroup_<GroupData>
{
public:
	GroupProp() {
		registerProperty<IntProp>("/blendMode");
		// Note: nested shapes will be handled differently via the shape array
	}
	void extract(GroupData &g) const override {
		getProperty<IntProp>("/blendMode")->get(g.blendMode);
		// Note: nested shapes are handled in ShapeProp
	}
};

class FillProp : public PropertyGroup_<FillData>
{
public:
	FillProp() {
		registerProperty<VecProp<3>>("/color");
		registerProperty<PercentProp>("/opacity");
		registerProperty<IntProp>("/rule");
		registerProperty<IntProp>("/blendMode");
		registerProperty<IntProp>("/compositeOrder");
	}
	void extract(FillData &f) const override {
		getProperty<VecProp<3>>("/color")->get(f.color);
		getProperty<PercentProp>("/opacity")->get(f.opacity);
		getProperty<IntProp>("/rule")->get(f.rule);
		getProperty<IntProp>("/blendMode")->get(f.blendMode);
		getProperty<IntProp>("/compositeOrder")->get(f.compositeOrder);
	}
};

class StrokeProp : public PropertyGroup_<StrokeData>
{
public:
	StrokeProp() {
		registerProperty<VecProp<3>>("/color");
		registerProperty<PercentProp>("/opacity");
		registerProperty<FloatProp>("/width");
		registerProperty<IntProp>("/lineCap");
		registerProperty<IntProp>("/lineJoin");
		registerProperty<FloatProp>("/miterLimit");
		registerProperty<IntProp>("/blendMode");
		registerProperty<IntProp>("/compositeOrder");
	}
	void extract(StrokeData &s) const override {
		getProperty<VecProp<3>>("/color")->get(s.color);
		getProperty<PercentProp>("/opacity")->get(s.opacity);
		getProperty<FloatProp>("/width")->get(s.width);
		getProperty<IntProp>("/lineCap")->get(s.lineCap);
		getProperty<IntProp>("/lineJoin")->get(s.lineJoin);
		getProperty<FloatProp>("/miterLimit")->get(s.miterLimit);
		getProperty<IntProp>("/blendMode")->get(s.blendMode);
		getProperty<IntProp>("/compositeOrder")->get(s.compositeOrder);
	}
};

class ShapeProp : public PropertyArray_<ShapeData>
{
public:
	void setup(const ofJson &base, const ofJson &keyframes) override {
		clear();

		if (base.is_array()) {
			for(int i = 0; i < base.size(); ++i) {
				auto b = base[i];
				ofJson k = i < keyframes.size() ? keyframes[i] : ofJson{};
				if(auto p = addPropertyForType(b["shapeType"])) {
					p->setup(b, k);
				}
			}
		}
	}
	void extract(ShapeData &t) const override {

	}

private:
	PropertyBase* addPropertyForType(std::string type) {
		if (type == "ellipse") return addProperty<EllipseProp>();
		if (type == "rectangle") return addProperty<RectangleProp>();
		if (type == "fill") return addProperty<FillProp>();
		if (type == "stroke") return addProperty<StrokeProp>();
		if (type == "polygon") return addProperty<PolygonProp>();
		if (type == "group") return addProperty<GroupProp>();
		return nullptr;
	}
};

}}
