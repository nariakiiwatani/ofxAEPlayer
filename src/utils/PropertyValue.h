#pragma once

#include "ofJson.h"
#include "ofMath.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>

#include "../ofxAEKeyframe.h"

namespace ofx { namespace ae {

template<typename T>
class PropertyValue {
public:
    struct KeyframeData {
        float time;
        T value;
        Keyframe::InterpolationType interpolationType;
        Keyframe::TemporalEase temporalEase;
        std::vector<float> spatialInTangent;
        std::vector<float> spatialOutTangent;
        bool roving;
        bool continuous;
        
        KeyframeData()
            : time(0.0f)
            , interpolationType(Keyframe::LINEAR)
            , roving(false)
            , continuous(false) {}
        
        KeyframeData(float t, const T& val)
            : time(t)
            , value(val)
            , interpolationType(Keyframe::LINEAR)
            , roving(false)
            , continuous(false) {}
            
        // Copy assignment operator
        KeyframeData& operator=(const KeyframeData& other) {
            if (this != &other) {
                time = other.time;
                value = other.value;
                interpolationType = other.interpolationType;
                temporalEase = other.temporalEase;
                spatialInTangent = other.spatialInTangent;
                spatialOutTangent = other.spatialOutTangent;
                roving = other.roving;
                continuous = other.continuous;
            }
            return *this;
        }
    };

private:
    T initialValue_;
    std::vector<KeyframeData> keyframes_;

public:
    explicit PropertyValue(const T& initial)
        : initialValue_(initial)
	{}

    // 初期値の設定・取得
    void setInitialValue(const T& value) {
        initialValue_ = value;
    }
    
    const T& getInitialValue() const {
        return initialValue_;
    }
    
    // キーフレーム管理
    void addKeyframe(float time, const T& value) {
        KeyframeData keyframe(time, value);
        addKeyframe(keyframe);
    }
    
    void addKeyframe(const KeyframeData& keyframe) {
        // 時間順にソートして挿入
        auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), keyframe,
            [](const KeyframeData& a, const KeyframeData& b) {
                return a.time < b.time;
            });
        
        // 同じ時間のキーフレームがある場合は置換
        if (it != keyframes_.end() && std::abs(it->time - keyframe.time) < 1e-6f) {
            *it = keyframe;
        } else {
            keyframes_.insert(it, keyframe);
        }
    }
    
    void removeKeyframe(float time) {
        keyframes_.erase(
            std::remove_if(keyframes_.begin(), keyframes_.end(),
                [time](const KeyframeData& kf) {
                    return std::abs(kf.time - time) < 1e-6f;
                }),
            keyframes_.end());
    }
    
    void clearKeyframes() {
        keyframes_.clear();
    }
    
    // 補間とアニメーション
    T getValueAtTime(float time) const {
        if (!hasAnimation()) {
            return initialValue_;
        }
        
        T result = interpolateAtTime(time);

        return result;
    }
    
    // アニメーション状態
    bool hasAnimation() const { return !keyframes_.empty(); }
    size_t getKeyframeCount() const { return keyframes_.size(); }

    const std::vector<KeyframeData>& getKeyframes() const {
        return keyframes_;
    }
    
private:
    T interpolateAtTime(float time) const {
        if (keyframes_.empty()) {
            return initialValue_;
        }
        
        // 範囲チェック
        if (time <= keyframes_.front().time) {
            return keyframes_.front().value;
        }
        if (time >= keyframes_.back().time) {
            return keyframes_.back().value;
        }
        
        // 適切なキーフレームペアを探す
        for (size_t i = 0; i < keyframes_.size() - 1; ++i) {
            const auto& kf1 = keyframes_[i];
            const auto& kf2 = keyframes_[i + 1];
            
            if (time >= kf1.time && time <= kf2.time) {
                return interpolateBetween(kf1, kf2, time);
            }
        }
        
        return initialValue_;
    }
    
    T interpolateBetween(const KeyframeData& kf1, const KeyframeData& kf2, float time) const {
        if (std::abs(kf2.time - kf1.time) < 1e-6f) {
            return kf1.value;
        }
        
        float t = (time - kf1.time) / (kf2.time - kf1.time);
        t = std::clamp(t, 0.0f, 1.0f);
        
        // HOLDタイプの場合
        if (kf1.interpolationType == Keyframe::HOLD) {
            return kf1.value;
        }
        
        // 時間イージングを適用
        float easedT = applyTemporalEasing(t, kf1);
        
        // 空間補間が必要かチェック
        if (needsSpatialInterpolation(kf1, kf2)) {
            return applySpatialInterpolation(kf1, kf2, easedT);
        }
        
        // 通常の補間
        return interpolateValue(kf1.value, kf2.value, easedT, kf1.interpolationType);
    }
    
    float applyTemporalEasing(float t, const KeyframeData& kf) const {
        switch (kf.interpolationType) {
            case Keyframe::BEZIER:
                return Keyframe::calculateTemporalEasing(kf.temporalEase, t);
            case Keyframe::EASE_IN:
                return TemporalEasing::easeIn(t);
            case Keyframe::EASE_OUT:
                return TemporalEasing::easeOut(t);
            case Keyframe::EASE_IN_OUT:
                return TemporalEasing::easeInOut(t);
            default:
                return t;
        }
    }
    
    bool needsSpatialInterpolation(const KeyframeData& kf1, const KeyframeData& kf2) const {
        return (!kf1.spatialInTangent.empty() || !kf1.spatialOutTangent.empty() ||
                !kf2.spatialInTangent.empty() || !kf2.spatialOutTangent.empty());
    }
    
    // デフォルト実装（特殊化で上書き）
    T interpolateValue(const T& v1, const T& v2, float t, Keyframe::InterpolationType type) const {
        // ジェネリック線形補間（特殊化で上書きされる）
        return v1; // プレースホルダー
    }
    
    T applySpatialInterpolation(const KeyframeData& kf1, const KeyframeData& kf2, float t) const {
        // デフォルトでは通常の補間（特殊化で上書き）
        return interpolateValue(kf1.value, kf2.value, t, kf1.interpolationType);
    }
};

// ===== float特殊化 =====
template<>
class PropertyValue<float> {
public:
    struct KeyframeData {
        float time;
        float value;
        Keyframe::InterpolationType interpolationType;
        Keyframe::TemporalEase temporalEase;
        bool roving;
        bool continuous;
        
        KeyframeData()
            : time(0.0f), value(0.0f)
            , interpolationType(Keyframe::LINEAR)
            , roving(false), continuous(false) {}
        
        KeyframeData(float t, float val)
            : time(t), value(val)
            , interpolationType(Keyframe::LINEAR)
            , roving(false), continuous(false) {}
            
        // Copy assignment operator
        KeyframeData& operator=(const KeyframeData& other) {
            if (this != &other) {
                time = other.time;
                value = other.value;
                interpolationType = other.interpolationType;
                temporalEase = other.temporalEase;
                roving = other.roving;
                continuous = other.continuous;
            }
            return *this;
        }
    };

private:
    float initialValue_;
    std::vector<KeyframeData> keyframes_;

public:
    PropertyValue() : initialValue_(0.0f) {}
    explicit PropertyValue(float initial) : initialValue_(initial) {}
    
    void setInitialValue(float value) {
        initialValue_ = value;
    }
    
    float getInitialValue() const { return initialValue_; }
    
    void addKeyframe(float time, float value) {
        KeyframeData keyframe(time, value);
        addKeyframe(keyframe);
    }
    
    void addKeyframe(const KeyframeData& keyframe) {
        auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), keyframe,
            [](const KeyframeData& a, const KeyframeData& b) {
                return a.time < b.time;
            });
        
        if (it != keyframes_.end() && std::abs(it->time - keyframe.time) < 1e-6f) {
            *it = keyframe;
        } else {
            keyframes_.insert(it, keyframe);
        }
    }
    
    void removeKeyframe(float time) {
        keyframes_.erase(
            std::remove_if(keyframes_.begin(), keyframes_.end(),
                [time](const KeyframeData& kf) {
                    return std::abs(kf.time - time) < 1e-6f;
                }),
            keyframes_.end());
    }
    
    void clearKeyframes() {
        keyframes_.clear();
    }
    
    float getValueAtTime(float time) const {
        if (!hasAnimation()) {
            return initialValue_;
        }
        float result = interpolateAtTime(time);
        return result;
    }
    
    bool hasAnimation() const { return !keyframes_.empty(); }
    size_t getKeyframeCount() const { return keyframes_.size(); }
    const std::vector<KeyframeData>& getKeyframes() const { return keyframes_; }

private:
    float interpolateAtTime(float time) const {
        if (keyframes_.empty()) {
            return initialValue_;
        }
        
        if (time <= keyframes_.front().time) {
            return keyframes_.front().value;
        }
        if (time >= keyframes_.back().time) {
            return keyframes_.back().value;
        }
        
        for (size_t i = 0; i < keyframes_.size() - 1; ++i) {
            const auto& kf1 = keyframes_[i];
            const auto& kf2 = keyframes_[i + 1];
            
            if (time >= kf1.time && time <= kf2.time) {
                return interpolateBetween(kf1, kf2, time);
            }
        }
        
        return initialValue_;
    }
    
    float interpolateBetween(const KeyframeData& kf1, const KeyframeData& kf2, float time) const {
        if (std::abs(kf2.time - kf1.time) < 1e-6f) {
            return kf1.value;
        }
        
        if (kf1.interpolationType == Keyframe::HOLD) {
            return kf1.value;
        }
        
        float t = (time - kf1.time) / (kf2.time - kf1.time);
        t = std::clamp(t, 0.0f, 1.0f);
        
        // 時間イージングを適用
        float easedT = applyTemporalEasing(t, kf1);
        
        // 線形補間
        return kf1.value + (kf2.value - kf1.value) * easedT;
    }
    
    float applyTemporalEasing(float t, const KeyframeData& kf) const {
        switch (kf.interpolationType) {
            case Keyframe::BEZIER:
                return Keyframe::calculateTemporalEasing(kf.temporalEase, t);
            case Keyframe::EASE_IN:
                return TemporalEasing::easeIn(t);
            case Keyframe::EASE_OUT:
                return TemporalEasing::easeOut(t);
            case Keyframe::EASE_IN_OUT:
                return TemporalEasing::easeInOut(t);
            default:
                return t;
        }
    }
};

// ===== glm::vec3特殊化 =====
template<>
class PropertyValue<glm::vec3> {
public:
    struct KeyframeData {
        float time;
        glm::vec3 value;
        Keyframe::InterpolationType interpolationType;
        Keyframe::TemporalEase temporalEase;
        std::vector<float> spatialInTangent;
        std::vector<float> spatialOutTangent;
        bool roving;
        bool continuous;
        
        KeyframeData()
            : time(0.0f), value(0.0f)
            , interpolationType(Keyframe::LINEAR)
            , roving(false), continuous(false) {}
        
        KeyframeData(float t, const glm::vec3& val)
            : time(t), value(val)
            , interpolationType(Keyframe::LINEAR)
            , roving(false), continuous(false) {}
            
        // Copy assignment operator
        KeyframeData& operator=(const KeyframeData& other) {
            if (this != &other) {
                time = other.time;
                value = other.value;
                interpolationType = other.interpolationType;
                temporalEase = other.temporalEase;
                spatialInTangent = other.spatialInTangent;
                spatialOutTangent = other.spatialOutTangent;
                roving = other.roving;
                continuous = other.continuous;
            }
            return *this;
        }
    };

private:
    glm::vec3 initialValue_;
    std::vector<KeyframeData> keyframes_;

public:
    PropertyValue() : initialValue_(0.0f) {}
    explicit PropertyValue(const glm::vec3& initial) : initialValue_(initial)
	{}

    void setInitialValue(const glm::vec3& value) {
        initialValue_ = value;
    }
    
    const glm::vec3& getInitialValue() const { return initialValue_; }
    
    void addKeyframe(float time, const glm::vec3& value) {
        KeyframeData keyframe(time, value);
        addKeyframe(keyframe);
    }
    
    void addKeyframe(const KeyframeData& keyframe) {
        auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), keyframe,
            [](const KeyframeData& a, const KeyframeData& b) {
                return a.time < b.time;
            });
        
        if (it != keyframes_.end() && std::abs(it->time - keyframe.time) < 1e-6f) {
            *it = keyframe;
        } else {
            keyframes_.insert(it, keyframe);
        }
    }
    
    void removeKeyframe(float time) {
        keyframes_.erase(
            std::remove_if(keyframes_.begin(), keyframes_.end(),
                [time](const KeyframeData& kf) {
                    return std::abs(kf.time - time) < 1e-6f;
                }),
            keyframes_.end());
    }
    
    void clearKeyframes() {
        keyframes_.clear();
    }
    
    glm::vec3 getValueAtTime(float time) const {
        if (!hasAnimation()) {
            return initialValue_;
        }
        
        glm::vec3 result = interpolateAtTime(time);
        return result;
    }
    
    bool hasAnimation() const { return !keyframes_.empty(); }
    size_t getKeyframeCount() const { return keyframes_.size(); }
    const std::vector<KeyframeData>& getKeyframes() const { return keyframes_; }

private:
    glm::vec3 interpolateAtTime(float time) const {
        if (keyframes_.empty()) {
            return initialValue_;
        }
        
        if (time <= keyframes_.front().time) {
            return keyframes_.front().value;
        }
        if (time >= keyframes_.back().time) {
            return keyframes_.back().value;
        }
        
        for (size_t i = 0; i < keyframes_.size() - 1; ++i) {
            const auto& kf1 = keyframes_[i];
            const auto& kf2 = keyframes_[i + 1];
            
            if (time >= kf1.time && time <= kf2.time) {
                return interpolateBetween(kf1, kf2, time);
            }
        }
        
        return initialValue_;
    }
    
    glm::vec3 interpolateBetween(const KeyframeData& kf1, const KeyframeData& kf2, float time) const {
        if (std::abs(kf2.time - kf1.time) < 1e-6f) {
            return kf1.value;
        }
        
        if (kf1.interpolationType == Keyframe::HOLD) {
            return kf1.value;
        }
        
        float t = (time - kf1.time) / (kf2.time - kf1.time);
        t = std::clamp(t, 0.0f, 1.0f);
        
        // 時間イージングを適用
        float easedT = applyTemporalEasing(t, kf1);
        
        // 空間補間が必要かチェック
        if (needsSpatialInterpolation(kf1, kf2)) {
            return applySpatialInterpolation(kf1, kf2, easedT);
        }
        
        // 線形補間
        return glm::mix(kf1.value, kf2.value, easedT);
    }
    
    float applyTemporalEasing(float t, const KeyframeData& kf) const {
        switch (kf.interpolationType) {
            case Keyframe::BEZIER:
                return Keyframe::calculateTemporalEasing(kf.temporalEase, t);
            case Keyframe::EASE_IN:
                return TemporalEasing::easeIn(t);
            case Keyframe::EASE_OUT:
                return TemporalEasing::easeOut(t);
            case Keyframe::EASE_IN_OUT:
                return TemporalEasing::easeInOut(t);
            default:
                return t;
        }
    }
    
    bool needsSpatialInterpolation(const KeyframeData& kf1, const KeyframeData& kf2) const {
        return (!kf1.spatialInTangent.empty() || !kf1.spatialOutTangent.empty() ||
                !kf2.spatialInTangent.empty() || !kf2.spatialOutTangent.empty());
    }
    
    glm::vec3 applySpatialInterpolation(const KeyframeData& kf1, const KeyframeData& kf2, float t) const {
        // ベジェ曲線による空間補間
        glm::vec3 p0 = kf1.value;
        glm::vec3 p3 = kf2.value;
        
        // 制御点の計算
        glm::vec3 p1 = p0;
        glm::vec3 p2 = p3;
        
        if (kf1.spatialOutTangent.size() >= 2) {
            p1.x += kf1.spatialOutTangent[0];
            p1.y += kf1.spatialOutTangent[1];
            if (kf1.spatialOutTangent.size() >= 3) {
                p1.z += kf1.spatialOutTangent[2];
            }
        }
        
        if (kf2.spatialInTangent.size() >= 2) {
            p2.x += kf2.spatialInTangent[0];
            p2.y += kf2.spatialInTangent[1];
            if (kf2.spatialInTangent.size() >= 3) {
                p2.z += kf2.spatialInTangent[2];
            }
        }
        
        // 3次ベジェ曲線補間
        return SpatialInterpolator::bezierCurve(p0, p1, p2, p3, t);
    }
};

}} // namespace ofx::ae
