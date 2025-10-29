#include "ofxAEShapeProp.h"
#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

PathData PathDataProp::parse(const ofJson &json) const
{
	PathData pathData;
	
	if(json.contains("vertices") && json["vertices"].is_array()) {
		for(const auto& vertex : json["vertices"]) {
			if(vertex.is_array() && vertex.size() >= 2) {
				pathData.vertices.emplace_back(vertex[0], vertex[1]);
			}
		}
	}
	
	if(json.contains("inTangents") && json["inTangents"].is_array()) {
		for(const auto& tangent : json["inTangents"]) {
			if(tangent.is_array() && tangent.size() >= 2) {
				pathData.inTangents.emplace_back(tangent[0], tangent[1]);
			}
		}
	}
	
	if(json.contains("outTangents") && json["outTangents"].is_array()) {
		for(const auto& tangent : json["outTangents"]) {
			if(tangent.is_array() && tangent.size() >= 2) {
				pathData.outTangents.emplace_back(tangent[0], tangent[1]);
			}
		}
	}
	
	if(json.contains("closed")) {
		pathData.closed = json["closed"];
	}
	
	if(json.contains("direction")) {
		pathData.direction = json["direction"];
	}
	
	return pathData;
}


ShapeProp::ShapeProp()
{
	registerExtractor<ShapeData>([this](ShapeData &t) -> bool {
		try {
			auto &data = t.data;
			data.clear();
			for(const auto &prop : properties_) {
				if(auto ellipseProp = dynamic_cast<const EllipseProp*>(prop.get())) {
					auto ellipse = std::make_unique<EllipseData>();
					if(!ellipseProp->tryExtract(*ellipse)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract EllipseData, skipping";
						continue;
					}
					data.push_back(std::move(ellipse));
				}
				else if(auto rectProp = dynamic_cast<const RectangleProp*>(prop.get())) {
					auto rectangle = std::make_unique<RectangleData>();
					if(!rectProp->tryExtract(*rectangle)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract RectangleData, skipping";
						continue;
					}
					data.push_back(std::move(rectangle));
				}
				else if(auto fillProp = dynamic_cast<const FillProp*>(prop.get())) {
					auto fill = std::make_unique<FillData>();
					if(!fillProp->tryExtract(*fill)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract FillData, skipping";
						continue;
					}
					data.push_back(std::move(fill));
				}
				else if(auto strokeProp = dynamic_cast<const StrokeProp*>(prop.get())) {
					auto stroke = std::make_unique<StrokeData>();
					if(!strokeProp->tryExtract(*stroke)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract StrokeData, skipping";
						continue;
					}
					data.push_back(std::move(stroke));
				}
				else if(auto pathProp = dynamic_cast<const PathProp*>(prop.get())) {
					auto path = std::make_unique<PathData>();
					if(!pathProp->tryExtract(*path)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract PathData, skipping";
						continue;
					}
					data.push_back(std::move(path));
				}
				else if(auto polygonProp = dynamic_cast<const PolygonProp*>(prop.get())) {
					auto polygon = std::make_unique<PolygonData>();
					if(!polygonProp->tryExtract(*polygon)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract PolygonData, skipping";
						continue;
					}
					data.push_back(std::move(polygon));
				}
				else if(auto groupProp = dynamic_cast<const GroupProp*>(prop.get())) {
					auto group = std::make_unique<GroupData>();
					if(!groupProp->tryExtract(*group)) {
						ofLogWarning("PropertyExtraction") << "Failed to extract GroupData, skipping";
						continue;
					}
					data.push_back(std::move(group));
				}
			}
			return true;
		} catch(const std::exception& ex) {
			return false;
		}
	});
}

void ShapeProp::setup(const ofJson &base, const ofJson &keyframes)
{
	clear();

	if(base.is_array()) {
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

PropertyBase* ShapeProp::addPropertyForType(std::string type)
{
	if(type == "ellipse") return addProperty<EllipseProp>();
	if(type == "rectangle") return addProperty<RectangleProp>();
	if(type == "path") return addProperty<PathProp>();
	if(type == "fill") return addProperty<FillProp>();
	if(type == "stroke") return addProperty<StrokeProp>();
	if(type == "polygon") return addProperty<PolygonProp>();
	if(type == "group") return addProperty<GroupProp>();
	return nullptr;
}

GroupProp::GroupProp()
{
	registerProperty<BlendModeProp>("/blendMode");
	registerProperty<TransformProp>("/transform");
	registerProperty<ShapeProp>("/shape");

	registerExtractor<GroupData>([this](GroupData &g) -> bool {
		bool success = true;

		if(!getProperty<BlendModeProp>("/blendMode")->tryExtract(g.blendMode)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group blendMode, using default";
			g.blendMode = BlendMode::NORMAL;
			success = false;
		}
		
		if(!getProperty<ShapeProp>("/shape")->tryExtract((ShapeData&)g)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group shape data, using empty";
			g.data.clear();
			success = false;
		}

		if(!getProperty<TransformProp>("/transform")->tryExtract(g.transform)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract group transform data, using empty";
			g.transform = TransformData();
			success = false;
		}

		return success;
	});
}

void GroupProp::setup(const ofJson &base, const ofJson &keyframes)
{
	PropertyGroup::setup(base, keyframes);
	auto sb = base.contains("shape") ? base["shape"] : ofJson{};
	auto sk = keyframes.contains("shape") ? keyframes["shape"] : ofJson{};
	getProperty<ShapeProp>("/shape")->setup(sb, sk);
}

}}
