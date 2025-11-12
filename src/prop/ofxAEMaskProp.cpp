#include "ofxAEMaskProp.h"
#include "ofxAEVisitor.h"
#include "ofxAEMask.h"

namespace ofx { namespace ae {

MaskAtomProp::MaskAtomProp()
{
	registerProperty<PathProp>("/shape");
	registerProperty<VecTimeProp<2>>("/feather");
	registerProperty<PercentTimeProp>("/opacity");
	registerProperty<FloatTimeProp>("/offset");
	registerProperty<BoolTimeProp>("/inverted");
	registerProperty<MaskModeProp>("/mode");

	registerExtractor<MaskAtomData>([this](MaskAtomData &atom) -> bool {
		bool success = true;
		
		if(!getProperty<PathProp>("/shape")->tryExtract(atom.shape)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask shape, using default";
			atom.shape = PathData();
			success = false;
		}
		
		if(!getProperty<VecTimeProp<2>>("/feather")->tryExtract(atom.feather)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask feather, using default";
			atom.feather = glm::vec2(0.0f, 0.0f);
			success = false;
		}
		
		if(!getProperty<FloatTimeProp>("/opacity")->tryExtract(atom.opacity)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask opacity, using default";
			atom.opacity = 100.0f;
			success = false;
		}
		
		if(!getProperty<FloatTimeProp>("/offset")->tryExtract(atom.offset)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask offset, using default";
			atom.offset = 0.0f;
			success = false;
		}
		
		if(!getProperty<BoolTimeProp>("/inverted")->tryExtract(atom.inverted)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask inverted, using default";
			atom.inverted = false;
			success = false;
		}

		if(!getProperty<MaskModeProp>("/mode")->tryExtract(atom.mode)) {
			ofLogWarning("PropertyExtraction") << "Failed to extract mask mode, using default";
			atom.mode = MaskMode::ADD;
			success = false;
		}
		
		return success;
	});
}

void MaskAtomProp::accept(Visitor &visitor)
{
	TimePropertyGroup::accept(visitor);
}

MaskProp::MaskProp()
{
	registerExtractor<std::vector<MaskAtomData>>([this](std::vector<MaskAtomData> &masks) -> bool {
		masks.clear();
		masks.reserve(properties_.size());

		bool success = true;

		for(const auto &prop : properties_) {
			if(auto maskAtomProp = dynamic_cast<const MaskAtomProp*>(prop.get())) {
				MaskAtomData atom;
				if(maskAtomProp->tryExtract(atom)) {
					masks.push_back(atom);
				}
				else {
					ofLogWarning("PropertyExtraction") << "Failed to extract mask atom, skipping";
					success = false;
				}
			}
			else {
				masks.push_back(MaskAtomData());
			}
		}

		return success;
	});
}

void MaskProp::setup(const ofJson &base, const ofJson &keyframes)
{
	clear();
	
	if(!base.is_array()) {
		return;
	}
	
	for(size_t i = 0; i < base.size(); ++i) {
		const auto &atomBase = base[i];
		
		if(atomBase.is_null()) {
			addProperty<TimePropertyBase>();
			continue;
		}
		ofJson atomKeyframes = ofJson::object();
		if(keyframes.is_array() && i < keyframes.size() && !keyframes[i].is_null()) {
			atomKeyframes = keyframes[i];
		}
		
		setupMaskAtom(atomBase, atomKeyframes);
	}
}

void MaskProp::setupMaskAtom(const ofJson &atomBase, const ofJson &atomKeyframes)
{
	auto maskAtom = addProperty<MaskAtomProp>();
	
	if(atomBase.contains("atom")) {
		const auto &atom = atomBase["atom"];
		
		ofJson shapeBase = atom.contains("shape") ? atom["shape"] : ofJson::object();
		ofJson shapeKeyframes = ofJson::object();
		
		if(atomKeyframes.contains("atom") && atomKeyframes["atom"].contains("shape")) {
			shapeKeyframes = atomKeyframes["atom"]["shape"];
		}
		
		maskAtom->getProperty<PathProp>("/shape")->setup(shapeBase, shapeKeyframes);
		
		ofJson featherBase = atom.contains("feather") ? atom["feather"] : ofJson::array({0, 0});
		float opacityBase = atom.contains("opacity") ? atom["opacity"].get<float>() : 100.0f;
		float offsetBase = atom.contains("offset") ? atom["offset"].get<float>() : 0.0f;
		bool invertedBase = atom.contains("inverted") ? atom["inverted"].get<bool>() : false;
		std::string modeBase = atom.contains("mode") ? atom["mode"].get<std::string>() : "ADD";
		
		maskAtom->getProperty<VecTimeProp<2>>("/feather")->setup(featherBase, ofJson{});
		maskAtom->getProperty<FloatTimeProp>("/opacity")->setup(opacityBase, ofJson{});
		maskAtom->getProperty<FloatTimeProp>("/offset")->setup(offsetBase, ofJson{});
		maskAtom->getProperty<BoolTimeProp>("/inverted")->setup(invertedBase, ofJson{});
		maskAtom->getProperty<MaskModeProp>("/mode")->setup(modeBase, ofJson{});
	}
}

void MaskProp::accept(Visitor &visitor)
{
	TimePropertyArray::accept(visitor);
}

}} // namespace ofx::ae
