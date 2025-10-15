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
	virtual void setup(const ofJson &base, const ofJson &keyframes) {}
};

template<typename T>
class Property : public PropertyBase
{
public:
	using value_type = T;
	void setup(const ofJson &base, const ofJson &keyframes) override {
		setBaseValue(parse(base));
		keyframes_.clear();
		if(keyframes.is_array()) {
			for(int i = 0; i < keyframes.size(); ++i) {
				keyframes_.insert(parseKeyframe(keyframes[i]));
			}
			return;
		}
	}
	void setBaseValue(const T &t) { base_ = t; }
	void addKeyframe(int frame, const ofxAEKeyframe<T> &keyframe) {
		keyframes_.insert({frame, keyframe});
	}
	virtual T parse(const ofJson &json) const=0;
	std::pair<int, ofxAEKeyframe<T>> parseKeyframe(const ofJson &json) const {
		std::pair<int, ofxAEKeyframe<T>> ret;
		ret.first = json["frame"].get<int>();
		ret.second.value = parse(json["value"]);
		return ret;
	}
	void set(const T &t) { cache_ = t; }
	virtual void get(T &t) const { t = cache_; }
	bool hasAnimation() const override { return !keyframes_.empty(); }

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
			v->setup(base.value(p, ofJson{}), keyframes.value(p, ofJson{}));
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

class FloatProp : public Property<float>
{
public:
	float parse(const ofJson &json) const override { return json.get<float>(); }
};

class PercentProp : public Property<float>
{
public:
	float parse(const ofJson &json) const override { return json.get<float>()/100.f; }
};

template<int N, typename T=float>
class VecProp : public Property<glm::vec<N, T>>
{
public:
	using value_type = glm::vec<N, T>;
	value_type parse(const ofJson &json) const override {
		value_type ret;
		if(json.is_array()) {
			for(int i = 0; i < std::min<int>(json.size(), N); ++i) {
				ret[i] = json[i].get<T>();
			}
		}
		return ret;
	}
};

}} // namespace ofx::ae
