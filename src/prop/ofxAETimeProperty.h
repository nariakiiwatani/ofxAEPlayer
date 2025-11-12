#pragma once

#include <map>
#include <optional>
#include <typeindex>
#include <memory>
#include "ofJson.h"
#include "ofxAEKeyframe.h"
#include "../utils/ofxAETimeUtils.h"

namespace ofx { namespace ae {

class Visitor;

class TimePropertyBase
{
public:
	virtual ~TimePropertyBase() = default;
	virtual void accept(Visitor &visitor);
	virtual bool hasAnimation() const { return false; }
	
	// Time API only
	virtual bool setTime(double time) { return false; }
	virtual double getTime() const { return 0.0; }
	
	// FPS management
	virtual void setFps(double fps) { fps_ = fps; }
	virtual double getFps() const { return fps_; }
	
	virtual void setup(const ofJson &base, const ofJson &keyframes={}) {}
	
	template<typename T>
	bool tryExtract(T &out) const {
		auto it = extractors_.find(std::type_index(typeid(T)));
		if(it == extractors_.end()) return false;
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
	
	double fps_ = 30.0;
	
private:
	std::unordered_map<std::type_index, ExtractFn> extractors_;
};

template<typename T>
class TimeProperty : public TimePropertyBase
{
public:
	using value_type = T;
	
	TimeProperty() {
		registerExtractor<T>([this](T &t){
			t = get();
			return true;
		});
	}
	
	void setup(const ofJson &base, const ofJson &keyframes) override {
		setBaseValue(parse(base));
		keyframes_time_.clear();
		
		// Time-based grouped format: {keyframes_time: {"1.0": [...]}, fps: 30}
		if(keyframes.contains("keyframes_time")) {
			if(keyframes.contains("fps")) {
				fps_ = keyframes["fps"].get<double>();
			}
			
			auto kf_time = keyframes["keyframes_time"];
			for(auto it = kf_time.begin(); it != kf_time.end(); ++it) {
				double time = std::stod(it.key());
				auto &value = it.value();
				int size = value.size();
				for(int i = 0; i < size; ++i) {
					addKeyframeTime(time + static_cast<double>(i) / fps_, parseKeyframeValue(value[i]));
				}
			}
		}
		// Time-based array format: [{time: 1.0, value: ...}]
		else if(keyframes.is_array() && !keyframes.empty() && keyframes[0].contains("time")) {
			for(int i = 0; i < keyframes.size(); ++i) {
				const auto &kf = keyframes[i];
				double time = kf["time"].get<double>();
				addKeyframeTime(time, parseKeyframeValue(kf));
			}
		}
	}
	
	void setBaseValue(const T &t) { base_ = t; }
	
	void addKeyframeTime(double time, const Keyframe::Data<T> &keyframe) {
		keyframes_time_.insert({time, keyframe});
	}
	
	virtual T parse(const ofJson &json) const = 0;
	
	Keyframe::InterpolationType parseInterpolationType(const std::string &type) const {
		if(type == "LINEAR") return Keyframe::LINEAR;
		else if(type == "BEZIER") return Keyframe::BEZIER;
		else if(type == "HOLD") return Keyframe::HOLD;
		else return Keyframe::LINEAR;
	}
	
	Keyframe::Data<T> parseKeyframeValue(const ofJson &json) const {
		Keyframe::Data<T> kf;
		
		if(json.contains("value")) {
			kf.value = parse(json["value"]);
		}
		else {
			kf.value = parse(json);
		}
		
		if(json.contains("interpolation")) {
			const auto &interp = json["interpolation"];
			
			if(interp.contains("inType")) {
				std::string inType = interp["inType"].get<std::string>();
				kf.interpolation.in_type = parseInterpolationType(inType);
			}
			if(interp.contains("outType")) {
				std::string outType = interp["outType"].get<std::string>();
				kf.interpolation.out_type = parseInterpolationType(outType);
			}
			
			if(interp.contains("roving")) {
				kf.interpolation.roving = interp["roving"].get<bool>();
			}
			if(interp.contains("continuous")) {
				kf.interpolation.continuous = interp["continuous"].get<bool>();
			}
			
			if(interp.contains("temporalEase")) {
				const auto &tempEase = interp["temporalEase"];
				if(tempEase.contains("inEase")) {
					const auto &inEase = tempEase["inEase"];
					if(inEase.contains("speed")) {
						kf.interpolation.in_ease.speed = inEase["speed"].get<float>();
					}
					if(inEase.contains("influence")) {
						kf.interpolation.in_ease.influence = inEase["influence"].get<float>() / 100.0f;
					}
				}
				if(tempEase.contains("outEase")) {
					const auto &outEase = tempEase["outEase"];
					if(outEase.contains("speed")) {
						kf.interpolation.out_ease.speed = outEase["speed"].get<float>();
					}
					if(outEase.contains("influence")) {
						kf.interpolation.out_ease.influence = outEase["influence"].get<float>() / 100.0f;
					}
				}
			}
		}
		
		if(json.contains("spatialTangents")) {
			const auto &spatialTangents = json["spatialTangents"];
			if(spatialTangents.contains("inTangent") && spatialTangents["inTangent"].is_array()) {
				const auto &inTangent = spatialTangents["inTangent"];
				kf.spatial_tangents.in_tangent.clear();
				for(const auto &val : inTangent) {
					kf.spatial_tangents.in_tangent.push_back(val.get<float>());
				}
			}
			if(spatialTangents.contains("outTangent") && spatialTangents["outTangent"].is_array()) {
				const auto &outTangent = spatialTangents["outTangent"];
				kf.spatial_tangents.out_tangent.clear();
				for(const auto &val : outTangent) {
					kf.spatial_tangents.out_tangent.push_back(val.get<float>());
				}
			}
		}
		
		return kf;
	}
	
	void set(const T &t) { cache_ = t; }
	const T& get() const { return cache_.has_value() ? *cache_ : base_; }
	bool hasAnimation() const override { return !keyframes_time_.empty(); }
	
	bool setTime(double time) override {
		bool is_first = !cache_.has_value();
		if(keyframes_time_.empty()) {
			cache_ = base_;
			last_time_ = time;
			return is_first;
		}
		
		auto pair = util::findTimeKeyframePair(keyframes_time_, time);
		if(pair.keyframe_a == nullptr || pair.keyframe_b == nullptr) {
			cache_ = base_;
			last_time_ = time;
			return is_first;
		}
		
		float dt = static_cast<float>(pair.time_b - pair.time_a);
		cache_ = util::interpolateKeyframe(*pair.keyframe_a, *pair.keyframe_b, dt, pair.ratio);
		last_time_ = time;
		return true;
	}
	
	double getTime() const override { return last_time_; }
	
private:
	T base_;
	std::optional<T> cache_;
	std::map<double, Keyframe::Data<T>> keyframes_time_;
	double last_time_ = -1.0;
};

class TimePropertyGroup : public TimePropertyBase
{
public:
	void accept(Visitor &visitor) override;
	
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
	
	bool setTime(double time) override {
		bool ret = false;
		for(auto &&[_,p] : props_) {
			ret |= p->setTime(time);
		}
		return ret;
	}
	
	void setFps(double fps) override {
		TimePropertyBase::setFps(fps);
		for(auto &&[_,p] : props_) {
			p->setFps(fps);
		}
	}
	
private:
	std::map<std::string, std::unique_ptr<TimePropertyBase>> props_;
};

class TimePropertyArray : public TimePropertyBase
{
public:
	void accept(Visitor &visitor) override;
	void clear() { properties_.clear(); }
	
	template<typename T>
	T* addProperty() {
		auto p = std::make_unique<T>();
		T* ret = p.get();
		addProperty(std::move(p));
		return ret;
	}
	
	void addProperty(std::unique_ptr<TimePropertyBase> property) {
		properties_.push_back(std::move(property));
	}
	
	template<typename T=TimePropertyBase>
	T* getProperty(size_t index) {
		if(index >= properties_.size()) return nullptr;
		return static_cast<T*>(properties_[index].get());
	}
	
	template<typename T=TimePropertyBase>
	const T* getProperty(size_t index) const {
		if(index >= properties_.size()) return nullptr;
		return static_cast<const T*>(properties_[index].get());
	}
	
	bool hasAnimation() const override {
		for(const auto &p : properties_) {
			if(p && p->hasAnimation()) {
				return true;
			}
		}
		return false;
	}
	
	bool setTime(double time) override {
		bool changed = false;
		for(auto &p : properties_) {
			if(p) {
				changed |= p->setTime(time);
			}
		}
		return changed;
	}
	
	void setFps(double fps) override {
		TimePropertyBase::setFps(fps);
		for(auto &p : properties_) {
			if(p) {
				p->setFps(fps);
			}
		}
	}
	
protected:
	std::vector<std::unique_ptr<TimePropertyBase>> properties_;
};

// Concrete property implementations using TimeProperty

class FloatTimeProp : public TimeProperty<float>
{
public:
	FloatTimeProp() : TimeProperty<float>() {}
	
	float parse(const ofJson &json) const override {
		return json.is_null() ? 0.0f : json.get<float>();
	}
};

class IntTimeProp : public TimeProperty<int>
{
public:
	IntTimeProp() : TimeProperty<int>() {}
	
	int parse(const ofJson &json) const override {
		return json.is_null() ? 0 : json.get<int>();
	}
};

class PercentTimeProp : public TimeProperty<float>
{
public:
	PercentTimeProp() : TimeProperty<float>() {}
	
	float parse(const ofJson &json) const override {
		return json.is_null() ? 0.0f : json.get<float>()/100.f;
	}
};

template<int N, typename T=float>
class VecTimeProp : public TimeProperty<glm::vec<N, T>>
{
public:
	using value_type = glm::vec<N, T>;
	
	VecTimeProp() : TimeProperty<glm::vec<N, T>>() {}
	
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
class PercentVecTimeProp : public TimeProperty<glm::vec<N, T>>
{
public:
	using value_type = glm::vec<N, T>;
	
	PercentVecTimeProp() : TimeProperty<glm::vec<N, T>>() {}
	
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

class ColorTimeProp : public TimeProperty<ofFloatColor>
{
public:
	ColorTimeProp() : TimeProperty<ofFloatColor>() {}
	
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

class BoolTimeProp : public TimeProperty<bool>
{
public:
	BoolTimeProp() : TimeProperty<bool>() {}
	
	bool parse(const ofJson &json) const override {
		return json.is_null() ? false : json.get<bool>();
	}
};

}} // namespace ofx::ae
