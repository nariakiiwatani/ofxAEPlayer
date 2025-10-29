#pragma once

#include "ofPath.h"
#include "../data/Enums.h"
#include "../utils/ofxAEBlendMode.h"
#include "ofColor.h"
#include <glm/vec2.hpp>
#include "TransformData.h"

namespace ofx { namespace ae {
class Visitor;

struct ShapeDataBase {
	virtual ~ShapeDataBase() = default;
	virtual void accept(Visitor& visitor) const = 0;
};

struct EllipseData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	glm::vec2 size{0,0};
	glm::vec2 position{0,0};
	WindingDirection direction{WindingDirection::DEFAULT};
};

struct RectangleData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	glm::vec2 size{0,0};
	glm::vec2 position{0,0};
	float roundness{0};
	WindingDirection direction{WindingDirection::DEFAULT};
};

struct PolygonData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	WindingDirection direction{WindingDirection::DEFAULT};
	int type{1}; // 1 = polygon, 2 = star
	int points{5};
	glm::vec2 position{0,0};
	float rotation{0};
	float innerRadius{50};
	float outerRadius{100};
	float innerRoundness{0};
	float outerRoundness{0};
};

struct PathData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	std::vector<glm::vec2> vertices{};
	std::vector<glm::vec2> inTangents{};
	std::vector<glm::vec2> outTangents{};
	bool closed{true};
	WindingDirection direction{WindingDirection::DEFAULT};

	PathData operator+(const PathData& other) const;
	PathData operator-(const PathData& other) const;
	PathData operator*(float t) const;

	ofPath toOfPath() const;
};
struct FillData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	ofFloatColor color{1,1,1,1};
	float opacity{1};
	FillRule rule{FillRule::NON_ZERO};
	BlendMode blendMode{BlendMode::NORMAL};
	int compositeOrder{1};
};

struct StrokeData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	ofFloatColor color{1,1,1,1};
	float opacity{1};
	float width{1};
	int lineCap{1};
	int lineJoin{1};
	float miterLimit{4};
	BlendMode blendMode{BlendMode::NORMAL};
	int compositeOrder{1};
};

struct GroupData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	std::vector<std::unique_ptr<ShapeDataBase>> data{};
	BlendMode blendMode{BlendMode::NORMAL};
	TransformData transform;
};
}}
