#pragma once

#include "ofxAEProperty.h"

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
	int direction{1}; // 1 = defalt(clockwise) 2 = clockwise, 3 = counterclockwise
};

struct RectangleData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	glm::vec2 size{0,0};
	glm::vec2 position{0,0};
	float roundness{0};
	int direction{1}; // 1 = defalt(clockwise) 2 = clockwise, 3 = counterclockwise
};

struct PolygonData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	int direction{1}; // 1 = defalt(clockwise) 2 = clockwise, 3 = counterclockwise
	int type{1}; // 1 = polygon, 2 = star
	int points{5};
	glm::vec2 position{0,0};
	float rotation{0};
	float innerRadius{50};
	float outerRadius{100};
	float innerRoundness{0};
	float outerRoundness{0};
};

struct FillData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	ofFloatColor color{1,1,1,1};
	float opacity{1};
	int rule{1}; // Fill rule
	int blendMode{1};
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
	int blendMode{1};
	int compositeOrder{1};
};

struct ShapeData : public ShapeDataBase {
	void accept(Visitor& visitor) const override;
	std::vector<std::unique_ptr<ShapeDataBase>> data{};
};
struct GroupData : public ShapeData {
	void accept(Visitor& visitor) const override;
	int blendMode{1};
};

class EllipseProp : public PropertyGroup
{
public:
	EllipseProp() {
		registerProperty<VecProp<2>>("/size");
		registerProperty<VecProp<2>>("/position");
		registerProperty<IntProp>("/direction");
		
		registerExtractor<EllipseData>([this](EllipseData& e) -> bool {
			bool success = true;
			
			if (!getProperty<VecProp<2>>("/size")->tryExtract(e.size)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse size, using default";
				e.size = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if (!getProperty<VecProp<2>>("/position")->tryExtract(e.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse position, using default";
				e.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if (!getProperty<IntProp>("/direction")->tryExtract(e.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse direction, using default";
				e.direction = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class RectangleProp : public PropertyGroup
{
public:
	RectangleProp() {
		registerProperty<VecProp<2>>("/size");
		registerProperty<VecProp<2>>("/position");
		registerProperty<FloatProp>("/roundness");
		registerProperty<IntProp>("/direction");
		
		registerExtractor<RectangleData>([this](RectangleData& r) -> bool {
			bool success = true;
			
			if (!getProperty<VecProp<2>>("/size")->tryExtract(r.size)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle size, using default";
				r.size = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if (!getProperty<VecProp<2>>("/position")->tryExtract(r.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle position, using default";
				r.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if (!getProperty<FloatProp>("/roundness")->tryExtract(r.roundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle roundness, using default";
				r.roundness = 0.0f;
				success = false;
			}
			
			if (!getProperty<IntProp>("/direction")->tryExtract(r.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle direction, using default";
				r.direction = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class PolygonProp : public PropertyGroup
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
		
		registerExtractor<PolygonData>([this](PolygonData& p) -> bool {
			bool success = true;
			
			if (!getProperty<IntProp>("/direction")->tryExtract(p.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon direction, using default";
				p.direction = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/type")->tryExtract(p.type)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon type, using default";
				p.type = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/points")->tryExtract(p.points)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon points, using default";
				p.points = 5;
				success = false;
			}
			
			if (!getProperty<VecProp<2>>("/position")->tryExtract(p.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon position, using default";
				p.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if (!getProperty<FloatProp>("/rotation")->tryExtract(p.rotation)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon rotation, using default";
				p.rotation = 0.0f;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/innerRadius")->tryExtract(p.innerRadius)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon innerRadius, using default";
				p.innerRadius = 50.0f;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/outerRadius")->tryExtract(p.outerRadius)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon outerRadius, using default";
				p.outerRadius = 100.0f;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/innerRoundness")->tryExtract(p.innerRoundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon innerRoundness, using default";
				p.innerRoundness = 0.0f;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/outerRoundness")->tryExtract(p.outerRoundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon outerRoundness, using default";
				p.outerRoundness = 0.0f;
				success = false;
			}
			
			return success;
		});
	}
};

class GroupProp : public PropertyGroup
{
public:
	GroupProp();
};

class FillProp : public PropertyGroup
{
public:
	FillProp() {
		registerProperty<ColorProp>("/color");
		registerProperty<PercentProp>("/opacity");
		registerProperty<IntProp>("/rule");
		registerProperty<IntProp>("/blendMode");
		registerProperty<IntProp>("/compositeOrder");
		
		registerExtractor<FillData>([this](FillData& f) -> bool {
			bool success = true;
			
			if (!getProperty<ColorProp>("/color")->tryExtract(f.color)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill color, using default";
				f.color = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
				success = false;
			}
			
			if (!getProperty<PercentProp>("/opacity")->tryExtract(f.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill opacity, using default";
				f.opacity = 1.0f;
				success = false;
			}
			
			if (!getProperty<IntProp>("/rule")->tryExtract(f.rule)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill rule, using default";
				f.rule = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/blendMode")->tryExtract(f.blendMode)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill blendMode, using default";
				f.blendMode = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/compositeOrder")->tryExtract(f.compositeOrder)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill compositeOrder, using default";
				f.compositeOrder = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class StrokeProp : public PropertyGroup
{
public:
	StrokeProp() {
		registerProperty<ColorProp>("/color");
		registerProperty<PercentProp>("/opacity");
		registerProperty<FloatProp>("/width");
		registerProperty<IntProp>("/lineCap");
		registerProperty<IntProp>("/lineJoin");
		registerProperty<FloatProp>("/miterLimit");
		registerProperty<IntProp>("/blendMode");
		registerProperty<IntProp>("/compositeOrder");
		
		registerExtractor<StrokeData>([this](StrokeData& s) -> bool {
			bool success = true;
			
			if (!getProperty<ColorProp>("/color")->tryExtract(s.color)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke color, using default";
				s.color = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
				success = false;
			}
			
			if (!getProperty<PercentProp>("/opacity")->tryExtract(s.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke opacity, using default";
				s.opacity = 1.0f;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/width")->tryExtract(s.width)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke width, using default";
				s.width = 1.0f;
				success = false;
			}
			
			if (!getProperty<IntProp>("/lineCap")->tryExtract(s.lineCap)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke lineCap, using default";
				s.lineCap = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/lineJoin")->tryExtract(s.lineJoin)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke lineJoin, using default";
				s.lineJoin = 1;
				success = false;
			}
			
			if (!getProperty<FloatProp>("/miterLimit")->tryExtract(s.miterLimit)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke miterLimit, using default";
				s.miterLimit = 4.0f;
				success = false;
			}
			
			if (!getProperty<IntProp>("/blendMode")->tryExtract(s.blendMode)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke blendMode, using default";
				s.blendMode = 1;
				success = false;
			}
			
			if (!getProperty<IntProp>("/compositeOrder")->tryExtract(s.compositeOrder)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke compositeOrder, using default";
				s.compositeOrder = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class ShapeProp : public PropertyArray
{
public:
	ShapeProp() {
		registerExtractor<ShapeData>([this](ShapeData& t) -> bool {
			try {
				auto &data = t.data;
				data.clear();
				for (const auto& prop : properties_) {
					if (auto ellipseProp = dynamic_cast<const EllipseProp*>(prop.get())) {
						auto ellipse = std::make_unique<EllipseData>();
						if (!ellipseProp->tryExtract(*ellipse)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract EllipseData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(ellipse));
					}
					else if (auto rectProp = dynamic_cast<const RectangleProp*>(prop.get())) {
						auto rectangle = std::make_unique<RectangleData>();
						if (!rectProp->tryExtract(*rectangle)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract RectangleData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(rectangle));
					}
					else if (auto fillProp = dynamic_cast<const FillProp*>(prop.get())) {
						auto fill = std::make_unique<FillData>();
						if (!fillProp->tryExtract(*fill)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract FillData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(fill));
					}
					else if (auto strokeProp = dynamic_cast<const StrokeProp*>(prop.get())) {
						auto stroke = std::make_unique<StrokeData>();
						if (!strokeProp->tryExtract(*stroke)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract StrokeData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(stroke));
					}
					else if (auto polygonProp = dynamic_cast<const PolygonProp*>(prop.get())) {
						auto polygon = std::make_unique<PolygonData>();
						if (!polygonProp->tryExtract(*polygon)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract PolygonData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(polygon));
					}
					else if (auto groupProp = dynamic_cast<const GroupProp*>(prop.get())) {
						auto group = std::make_unique<GroupData>();
						if (!groupProp->tryExtract(*group)) {
							ofLogWarning("PropertyExtraction") << "Failed to extract GroupData, skipping";
							continue; // Skip this shape component
						}
						data.push_back(std::move(group));
					}
				}
				return true;
			} catch (const std::exception& ex) {
				return false;
			}
		});
	}
	
	void setup(const ofJson &base, const ofJson &keyframes) override {
		clear();

		if (base.is_array()) {
			for(int i = 0; i < base.size(); ++i) {
				auto b = base[i];
				if(b.is_null()) continue;
				ofJson k = i < keyframes.size() ? keyframes[i] : ofJson{};
				if(auto p = addPropertyForType(b["shapeType"])) {
					p->setup(b, k);
				}
			}
		}
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
