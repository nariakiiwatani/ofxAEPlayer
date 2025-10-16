#pragma once

#include <vector>
#include <memory>
#include "ofJson.h"
#include "ofxAEKeyframe.h"
#include "ofGraphicsBaseTypes.h"

namespace ofx { namespace ae {

class PropertyBase
{
public:
	virtual bool hasAnimation() const { return false; }
	virtual bool setFrame(int frame) { return false; }
	virtual void setup(const ofJson &base, const ofJson &keyframes={}) {}
};

template<typename T>
class Property : public PropertyBase
{
public:
	using value_type = T;
	void setup(const ofJson &base, const ofJson &keyframes) override {
		setBaseValue(parse(base));
		cache_ = base_; // Initialize cache with base value
		keyframes_.clear();
		if(keyframes.is_array()) {
			for(int i = 0; i < keyframes.size(); ++i) {
				keyframes_.insert(parseKeyframe(keyframes[i]));
			}
		}
	}
	void setBaseValue(const T &t) { base_ = t; }
	void addKeyframe(int frame, const ofxAEKeyframe<T> &keyframe) {
		keyframes_.insert({frame, keyframe});
	}
	virtual T parse(const ofJson &json) const=0;
	
	ofx::ae::Keyframe::InterpolationType parseInterpolationType(const std::string &type) const {
		if(type == "LINEAR") return ofx::ae::Keyframe::LINEAR;
		else if(type == "BEZIER") return ofx::ae::Keyframe::BEZIER;
		else if(type == "HOLD") return ofx::ae::Keyframe::HOLD;
		else if(type == "EASE_IN") return ofx::ae::Keyframe::EASE_IN;
		else if(type == "EASE_OUT") return ofx::ae::Keyframe::EASE_OUT;
		else if(type == "EASE_IN_OUT") return ofx::ae::Keyframe::EASE_IN_OUT;
		else if(type == "CUBIC") return ofx::ae::Keyframe::CUBIC;
		else if(type == "HERMITE") return ofx::ae::Keyframe::HERMITE;
		else if(type == "CATMULL_ROM") return ofx::ae::Keyframe::CATMULL_ROM;
		else return ofx::ae::Keyframe::LINEAR; // default fallback
	}
	std::pair<int, ofxAEKeyframe<T>> parseKeyframe(const ofJson &json) const {
		std::pair<int, ofxAEKeyframe<T>> ret;
		ret.first = json["frame"].get<int>();
		ret.second.value = parse(json["value"]);
		
		// Parse interpolation data
		if(json.contains("interpolation")) {
			const auto &interp = json["interpolation"];
			
			// Parse interpolation types
			if(interp.contains("inType")) {
				std::string inType = interp["inType"].get<std::string>();
				ret.second.interpolation.in_type = parseInterpolationType(inType);
			}
			if(interp.contains("outType")) {
				std::string outType = interp["outType"].get<std::string>();
				ret.second.interpolation.out_type = parseInterpolationType(outType);
			}
			
			// Parse roving and continuous flags
			if(interp.contains("roving")) {
				ret.second.interpolation.roving = interp["roving"].get<bool>();
			}
			if(interp.contains("continuous")) {
				ret.second.interpolation.continuous = interp["continuous"].get<bool>();
			}
			
			// Parse temporal ease data
			if(interp.contains("temporalEase")) {
				const auto &tempEase = interp["temporalEase"];
				if(tempEase.contains("inEase")) {
					const auto &inEase = tempEase["inEase"];
					if(inEase.contains("speed")) {
						ret.second.interpolation.in_ease.speed = inEase["speed"].get<float>();
					}
					if(inEase.contains("influence")) {
						ret.second.interpolation.in_ease.influence = inEase["influence"].get<float>();
					}
				}
				if(tempEase.contains("outEase")) {
					const auto &outEase = tempEase["outEase"];
					if(outEase.contains("speed")) {
						ret.second.interpolation.out_ease.speed = outEase["speed"].get<float>();
					}
					if(outEase.contains("influence")) {
						ret.second.interpolation.out_ease.influence = outEase["influence"].get<float>();
					}
				}
			}
		}
		
		// Parse spatial tangents data
		if(json.contains("spatialTangents")) {
			const auto &spatialTangents = json["spatialTangents"];
			if(spatialTangents.contains("inTangent") && spatialTangents["inTangent"].is_array()) {
				const auto &inTangent = spatialTangents["inTangent"];
				ret.second.spatial_tangents.in_tangent.clear();
				for(const auto &val : inTangent) {
					ret.second.spatial_tangents.in_tangent.push_back(val.get<float>());
				}
			}
			if(spatialTangents.contains("outTangent") && spatialTangents["outTangent"].is_array()) {
				const auto &outTangent = spatialTangents["outTangent"];
				ret.second.spatial_tangents.out_tangent.clear();
				for(const auto &val : outTangent) {
					ret.second.spatial_tangents.out_tangent.push_back(val.get<float>());
				}
			}
		}
		
		return ret;
	}
	void set(const T &t) { cache_ = t; }
	virtual void get(T &t) const { t = cache_; }
	bool hasAnimation() const override { return !keyframes_.empty(); }
	
	bool setFrame(int frame) override {
		if (keyframes_.empty()) {
			// No animation, use base value
			cache_ = base_;
			return false;
		}
		
		// Use the new keyframe lookup utility from ofxAEKeyframe.h
		auto pair = ofx::ae::util::findKeyframePair(keyframes_, frame);
		
		if (pair.keyframe_a == nullptr || pair.keyframe_b == nullptr) {
			// Fallback to base value if lookup failed
			cache_ = base_;
			return false;
		}
		
		// Use enhanced interpolation that considers both spatial and temporal modes
		cache_ = ofx::ae::util::interpolateKeyframe(*pair.keyframe_a, *pair.keyframe_b, pair.ratio);
		
		return true;
	}

private:
	T base_, cache_;
	std::map<int, ofxAEKeyframe<T>> keyframes_;
};

class PropertyGroup : public PropertyBase
{
public:
	template<typename T>
	T* registerProperty(std::string key) {
		auto result = props_.insert(std::make_pair(key, std::make_unique<T>()));
		return static_cast<T*>(result.first->second.get());
	}

	template<typename T>
	T* getProperty(std::string key) {
		auto found = props_.find(key);
		if(found == end(props_)) return nullptr;
		return static_cast<T*>(found->second.get());
	}
	template<typename T>
	const T* getProperty(std::string key) const {
		auto found = props_.find(key);
		if(found == end(props_)) return nullptr;
		return static_cast<const T*>(found->second.get());
	}
	void setup(const ofJson &base, const ofJson &keyframes) override {
		for(auto &&[k,v] : props_) {
			auto p = nlohmann::json::json_pointer(k);
			auto propValue = base.value(p, ofJson{});
			auto keyframeValue = keyframes.is_null() ? ofJson{} : keyframes.value(p, ofJson{});
			v->setup(propValue, keyframeValue);
		}
	}
	bool hasAnimation() const override {
		for(auto &&[_,p] : props_) {
			if(p->hasAnimation()) return true;
		}
		return false;
	}
	bool setFrame(int frame) override {
		bool ret = false;
		for(auto &&[_,p] : props_) {
			ret |= p->setFrame(frame);
		}
		return ret;
	}

private:
	std::map<std::string, std::unique_ptr<PropertyBase>> props_;
};

template<typename T>
class PropertyGroup_ : public PropertyGroup
{
public:
	virtual void extract(T &t) const=0;
};


class PropertyArray : public PropertyBase
{
public:
	void clear() { properties_.clear(); }
	template<typename T>
	T* addProperty() {
		auto p = std::make_unique<T>();
		T* ret = p.get();
		addProperty(std::move(p));
		return ret;
	}

	void addProperty(std::unique_ptr<PropertyBase> property) {
		properties_.push_back(std::move(property));
	}

	template<typename T=PropertyBase>
	T* getProperty(size_t index) {
		if (index >= properties_.size()) return nullptr;
		return static_cast<T*>(properties_[index].get());
	}

	template<typename T=PropertyBase>
	const T* getProperty(size_t index) const {
		if (index >= properties_.size()) return nullptr;
		return static_cast<const T*>(properties_[index].get());
	}

	bool hasAnimation() const override {
		for (const auto &p : properties_) {
			if (p && p->hasAnimation()) {
				return true;
			}
		}
		return false;
	}

	bool setFrame(int frame) override {
		bool changed = false;
		for (auto &p : properties_) {
			if (p) {
				changed |= p->setFrame(frame);
			}
		}
		return changed;
	}

private:
	std::vector<std::unique_ptr<PropertyBase>> properties_;
};

template<typename T>
class PropertyArray_ : public PropertyArray
{
public:
	virtual void extract(T &t) const=0;
};



class FloatProp : public Property<float>
{
public:
	float parse(const ofJson &json) const override {
		return json.is_null() ? 0.0f : json.get<float>();
	}
};

class IntProp : public Property<int>
{
public:
	int parse(const ofJson &json) const override {
		return json.is_null() ? 0 : json.get<int>();
	}
};

class PercentProp : public Property<float>
{
public:
	float parse(const ofJson &json) const override {
		return json.is_null() ? 0.0f : json.get<float>()/100.f;
	}
};

template<int N, typename T=float>
class VecProp : public Property<glm::vec<N, T>>
{
public:
	using value_type = glm::vec<N, T>;
	value_type parse(const ofJson &json) const override {
		value_type ret{};
		if(json.is_array()) {
			for(int i = 0; i < std::min<int>(json.size(), N); ++i) {
				ret[i] = json[i].get<T>();
			}
		}
		return ret;
	}
};

template<int N, typename T=float>
class PercentVecProp : public Property<glm::vec<N, T>>
{
public:
	using value_type = glm::vec<N, T>;
	value_type parse(const ofJson &json) const override {
		value_type ret{};
		if(json.is_array()) {
			for(int i = 0; i < std::min<int>(json.size(), N); ++i) {
				ret[i] = json[i].get<T>()/100.f;
			}
		}
		return ret;
	}
};


}} // namespace ofx::ae
