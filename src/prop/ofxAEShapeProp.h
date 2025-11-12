#pragma once

#include "ofxAETimeProperty.h"
#include "ofxAETransformProp.h"
#include "../utils/ofxAEBlendMode.h"
#include "../data/Enums.h"
#include "../data/PathData.h"
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class BlendModeProp : public TimeProperty<BlendMode>
{
public:
	BlendModeProp() : TimeProperty<BlendMode>() {}

	BlendMode parse(const ofJson &json) const override {
		if(json.is_string()) {
			std::string blendModeStr = json.get<std::string>();
			return blendModeFromString(blendModeStr);
		}
		else {
			ofLogWarning("BlendModeProp") << "Expected string blend mode value, using NORMAL";
			return BlendMode::NORMAL;
		}
	}
};
class FillRuleProp : public TimeProperty<FillRule>
{
public:
	FillRuleProp() : TimeProperty<FillRule>() {}

	FillRule parse(const ofJson &json) const override {
		if(json.is_string()) {
			return fillRuleFromString(json.get<std::string>());
		}
		else {
			ofLogWarning("FillRuleProp") << "Expected string fill rule value, using NON_ZERO";
			return FillRule::NON_ZERO;
		}
	}
};
class WindingDirectionProp : public TimeProperty<WindingDirection>
{
public:
	WindingDirectionProp() : TimeProperty<WindingDirection>() {}

	WindingDirection parse(const ofJson &json) const override {
		if(json.is_string()) {
			return windingDirectionFromString(json.get<std::string>());
		}
		else {
			ofLogWarning("WindingDirectionProp") << "Expected string winding direction value, using DEFAULT";
			return WindingDirection::DEFAULT;
		}
	}
};

class PathProp : public TimeProperty<PathData>
{
public:
	PathProp() : TimeProperty<PathData>() {}
	
	PathData parse(const ofJson &json) const override;
};

struct ShapeData : public GroupData {
	void accept(Visitor &visitor) const override;
};

class EllipseProp : public TimePropertyGroup
{
public:
	EllipseProp() {
		registerProperty<VecTimeProp<2>>("/size");
		registerProperty<VecTimeProp<2>>("/position");
		registerProperty<WindingDirectionProp>("/direction");

		registerExtractor<EllipseData>([this](EllipseData &e) -> bool {
			bool success = true;
			
			if(!getProperty<VecTimeProp<2>>("/size")->tryExtract(e.size)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse size, using default";
				e.size = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if(!getProperty<VecTimeProp<2>>("/position")->tryExtract(e.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse position, using default";
				e.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if(!getProperty<WindingDirectionProp>("/direction")->tryExtract(e.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract ellipse direction, using default";
				e.direction = WindingDirection::DEFAULT;
				success = false;
			}
			
			return success;
		});
	}
};

class RectangleProp : public TimePropertyGroup
{
public:
	RectangleProp() {
		registerProperty<VecTimeProp<2>>("/size");
		registerProperty<VecTimeProp<2>>("/position");
		registerProperty<FloatTimeProp>("/roundness");
		registerProperty<WindingDirectionProp>("/direction");

		registerExtractor<RectangleData>([this](RectangleData& r) -> bool {
			bool success = true;
			
			if(!getProperty<VecTimeProp<2>>("/size")->tryExtract(r.size)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle size, using default";
				r.size = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if(!getProperty<VecTimeProp<2>>("/position")->tryExtract(r.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle position, using default";
				r.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/roundness")->tryExtract(r.roundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle roundness, using default";
				r.roundness = 0.0f;
				success = false;
			}
			
			if(!getProperty<WindingDirectionProp>("/direction")->tryExtract(r.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract rectangle direction, using default";
				r.direction = WindingDirection::DEFAULT;
				success = false;
			}
			
			return success;
		});
	}
};

class PolygonProp : public TimePropertyGroup
{
public:
	PolygonProp() {
		registerProperty<WindingDirectionProp>("/direction");
		registerProperty<IntTimeProp>("/type");
		registerProperty<IntTimeProp>("/points");
		registerProperty<VecTimeProp<2>>("/position");
		registerProperty<FloatTimeProp>("/rotation");
		registerProperty<FloatTimeProp>("/innerRadius");
		registerProperty<FloatTimeProp>("/outerRadius");
		registerProperty<FloatTimeProp>("/innerRoundness");
		registerProperty<FloatTimeProp>("/outerRoundness");
		
		registerExtractor<PolygonData>([this](PolygonData &p) -> bool {
			bool success = true;
			
			if(!getProperty<WindingDirectionProp>("/direction")->tryExtract(p.direction)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon direction, using default";
				p.direction = WindingDirection::DEFAULT;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/type")->tryExtract(p.type)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon type, using default";
				p.type = 1;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/points")->tryExtract(p.points)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon points, using default";
				p.points = 5;
				success = false;
			}
			
			if(!getProperty<VecTimeProp<2>>("/position")->tryExtract(p.position)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon position, using default";
				p.position = glm::vec2(0.0f, 0.0f);
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/rotation")->tryExtract(p.rotation)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon rotation, using default";
				p.rotation = 0.0f;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/innerRadius")->tryExtract(p.innerRadius)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon innerRadius, using default";
				p.innerRadius = 50.0f;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/outerRadius")->tryExtract(p.outerRadius)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon outerRadius, using default";
				p.outerRadius = 100.0f;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/innerRoundness")->tryExtract(p.innerRoundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon innerRoundness, using default";
				p.innerRoundness = 0.0f;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/outerRoundness")->tryExtract(p.outerRoundness)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract polygon outerRoundness, using default";
				p.outerRoundness = 0.0f;
				success = false;
			}
			
			return success;
		});
	}
};

class FillProp : public TimePropertyGroup
{
public:
	FillProp() {
		registerProperty<ColorTimeProp>("/color");
		registerProperty<PercentTimeProp>("/opacity");
		registerProperty<FillRuleProp>("/rule");
		registerProperty<BlendModeProp>("/blendMode");
		registerProperty<IntTimeProp>("/compositeOrder");
		
		registerExtractor<FillData>([this](FillData &f) -> bool {
			bool success = true;
			
			if(!getProperty<ColorTimeProp>("/color")->tryExtract(f.color)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill color, using default";
				f.color = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
				success = false;
			}
			
			if(!getProperty<PercentTimeProp>("/opacity")->tryExtract(f.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill opacity, using default";
				f.opacity = 1.0f;
				success = false;
			}
			
			if(!getProperty<FillRuleProp>("/rule")->tryExtract(f.rule)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill rule, using default";
				f.rule = FillRule::NON_ZERO;
				success = false;
			}
			
			if(!getProperty<BlendModeProp>("/blendMode")->tryExtract(f.blendMode)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill blendMode, using default";
				f.blendMode = BlendMode::NORMAL;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/compositeOrder")->tryExtract(f.compositeOrder)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract fill compositeOrder, using default";
				f.compositeOrder = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class StrokeProp : public TimePropertyGroup
{
public:
	StrokeProp() {
		registerProperty<ColorTimeProp>("/color");
		registerProperty<PercentTimeProp>("/opacity");
		registerProperty<FloatTimeProp>("/width");
		registerProperty<IntTimeProp>("/lineCap");
		registerProperty<IntTimeProp>("/lineJoin");
		registerProperty<FloatTimeProp>("/miterLimit");
		registerProperty<BlendModeProp>("/blendMode");
		registerProperty<IntTimeProp>("/compositeOrder");
		
		registerExtractor<StrokeData>([this](StrokeData &s) -> bool {
			bool success = true;
			
			if(!getProperty<ColorTimeProp>("/color")->tryExtract(s.color)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke color, using default";
				s.color = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
				success = false;
			}
			
			if(!getProperty<PercentTimeProp>("/opacity")->tryExtract(s.opacity)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke opacity, using default";
				s.opacity = 1.0f;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/width")->tryExtract(s.width)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke width, using default";
				s.width = 1.0f;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/lineCap")->tryExtract(s.lineCap)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke lineCap, using default";
				s.lineCap = 1;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/lineJoin")->tryExtract(s.lineJoin)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke lineJoin, using default";
				s.lineJoin = 1;
				success = false;
			}
			
			if(!getProperty<FloatTimeProp>("/miterLimit")->tryExtract(s.miterLimit)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke miterLimit, using default";
				s.miterLimit = 4.0f;
				success = false;
			}
			
			if(!getProperty<BlendModeProp>("/blendMode")->tryExtract(s.blendMode)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke blendMode, using default";
				s.blendMode = BlendMode::NORMAL;
				success = false;
			}
			
			if(!getProperty<IntTimeProp>("/compositeOrder")->tryExtract(s.compositeOrder)) {
				ofLogWarning("PropertyExtraction") << "Failed to extract stroke compositeOrder, using default";
				s.compositeOrder = 1;
				success = false;
			}
			
			return success;
		});
	}
};

class ShapeProp : public TimePropertyArray
{
public:
	ShapeProp();
	virtual void setup(const ofJson &base, const ofJson &keyframes) override;

private:
	TimePropertyBase* addPropertyForType(std::string type);
};

class GroupProp : public TimePropertyGroup
{
public:
	GroupProp();
};

}}
