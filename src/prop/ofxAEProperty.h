#pragma once

#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include "ofJson.h"
#include "ofxAEKeyframe.h"
#include "ofGraphicsBaseTypes.h"

namespace ofx { namespace ae {

class Visitor;

class PropertyBase
{
public:
	virtual ~PropertyBase() = default;
	virtual void accept(Visitor& visitor);
	virtual bool hasAnimation() const { return false; }
	virtual bool setFrame(int frame) { return false; }
	virtual void setup(const ofJson &base, const ofJson &keyframes={}) {}

	template<typename T>
	bool tryExtract(T& out) const {
		auto it = extractors_.find(std::type_index(typeid(T)));
		if (it == extractors_.end()) return false;
		return it->second(reinterpret_cast<void*>(&out));
	}
protected:
	using ExtractFn = std::function<bool(void*)>;
	template<typename T, typename Fn>
	void registerExtractor(Fn&& fn) {
		extractors_[std::type_index(typeid(T))] =
			[fn = std::forward<Fn>(fn)](void* dst) -> bool {
				return fn(*reinterpret_cast<T*>(dst));
			};
	}
private:
	std::unordered_map<std::type_index, ExtractFn> extractors_;
};

template<typename T>
class Property : public PropertyBase
{
public:
	using value_type = T;

	Property() {
		registerExtractor<T>([this](T &t){
			t = get();
			return true;
		});
	}

	void setup(const ofJson &base, const ofJson &keyframes) override {
		setBaseValue(parse(base));
		keyframes_.clear();
		if(keyframes.is_array()) {
			for(int i = 0; i < keyframes.size(); ++i) {
				keyframes_.insert(parseKeyframe(keyframes[i]));
			}
		}
		else if(keyframes.is_object()) {
			for(auto it = keyframes.begin(); it != keyframes.end(); ++it) {
				auto &value = it.value();
				int start = ofToInt(it.key());
				int size = value.size();
				for(int i = 0; i < size; ++i) {
					addKeyframe(start+i, ofxAEKeyframe<T>(parse(value[i])));
				}
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
		else return ofx::ae::Keyframe::LINEAR; // default fallback
	}
	std::pair<int, ofxAEKeyframe<T>> parseKeyframe(const ofJson &json) const {
		std::pair<int, ofxAEKeyframe<T>> ret;
		ret.first = json["frame"].get<int>();
		ret.second.value = parse(json["value"]);
		
		if(json.contains("interpolation")) {
			const auto &interp = json["interpolation"];
			
			if(interp.contains("inType")) {
				std::string inType = interp["inType"].get<std::string>();
				ret.second.interpolation.in_type = parseInterpolationType(inType);
			}
			if(interp.contains("outType")) {
				std::string outType = interp["outType"].get<std::string>();
				ret.second.interpolation.out_type = parseInterpolationType(outType);
			}
			
			if(interp.contains("roving")) {
				ret.second.interpolation.roving = interp["roving"].get<bool>();
			}
			if(interp.contains("continuous")) {
				ret.second.interpolation.continuous = interp["continuous"].get<bool>();
			}
			
			if(interp.contains("temporalEase")) {
				const auto &tempEase = interp["temporalEase"];
				if(tempEase.contains("inEase")) {
					const auto &inEase = tempEase["inEase"];
					if(inEase.contains("speed")) {
						ret.second.interpolation.in_ease.speed = inEase["speed"].get<float>();
					}
					if(inEase.contains("influence")) {
						ret.second.interpolation.in_ease.influence = inEase["influence"].get<float>() / 100.0f;
					}
				}
				if(tempEase.contains("outEase")) {
					const auto &outEase = tempEase["outEase"];
					if(outEase.contains("speed")) {
						ret.second.interpolation.out_ease.speed = outEase["speed"].get<float>();
					}
					if(outEase.contains("influence")) {
						ret.second.interpolation.out_ease.influence = outEase["influence"].get<float>() / 100.0f;
					}
				}
			}
		}
		
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
	const T& get() const { return cache_.has_value() ? *cache_ : base_; }
	bool hasAnimation() const override { return !keyframes_.empty(); }
	
	bool setFrame(int frame) override {
		bool is_first = !cache_.has_value();
		if (keyframes_.empty()) {
			cache_ = base_;
			return is_first;
		}
		auto pair = ofx::ae::util::findKeyframePair(keyframes_, frame);
		if (pair.keyframe_a == nullptr || pair.keyframe_b == nullptr) {
			cache_ = base_;
			return is_first;
		}
		float fps = 30.f; // TODO: use correct fps
		cache_ = ofx::ae::util::interpolateKeyframe(*pair.keyframe_a, *pair.keyframe_b, (pair.frame_b - pair.frame_a)/fps, pair.ratio);
		return true;
	}
	
	private:
		T base_;
		std::optional<T> cache_;
		std::map<int, ofxAEKeyframe<T>> keyframes_;
	};
	
	class PropertyGroup : public PropertyBase
	{
	public:
		void accept(Visitor& visitor) override;
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
	
	class PropertyArray : public PropertyBase
	{
	public:
		void accept(Visitor& visitor) override;
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
	
	protected:
		std::vector<std::unique_ptr<PropertyBase>> properties_;
	};
	
	class FloatProp : public Property<float>
	{
	public:
		FloatProp() : Property<float>() {}
		
		float parse(const ofJson &json) const override {
			return json.is_null() ? 0.0f : json.get<float>();
		}
	};
	
	class IntProp : public Property<int>
	{
	public:
		IntProp() : Property<int>() {}
		
		int parse(const ofJson &json) const override {
			return json.is_null() ? 0 : json.get<int>();
		}
	};
	
	class PercentProp : public FloatProp
	{
	public:
		PercentProp() : FloatProp() {}
		
		float parse(const ofJson &json) const override {
			return json.is_null() ? 0.0f : json.get<float>()/100.f;
		}
	};
	
	template<int N, typename T=float>
	class VecProp : public Property<glm::vec<N, T>>
	{
	public:
		using value_type = glm::vec<N, T>;
		
		VecProp() : Property<glm::vec<N, T>>() {}
		
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
		
		PercentVecProp() : Property<glm::vec<N, T>>() {}
		
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

	class ColorProp : public Property<ofFloatColor>
	{
	public:
		ColorProp() : Property<ofFloatColor>() {}
		
		ofFloatColor parse(const ofJson &json) const override {
			ofFloatColor ret(1.0f, 1.0f, 1.0f, 1.0f);
			if(json.is_array() && json.size() >= 3) {
				float r = json[0].get<float>();
				float g = json[1].get<float>();
				float b = json[2].get<float>();
				float a = json.size() >= 4 ? json[3].get<float>() : 1.0f;
				ret.set(r, g, b, a);
			}
			return ret;
		}
	};


}} // namespace ofx::ae
