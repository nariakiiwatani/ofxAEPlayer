#pragma once

#include "ofJson.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "ofMath.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

namespace ofx { namespace ae {

// Forward declarations for spatial interpolation
class SpatialInterpolator;
class TemporalEasing;

class Keyframe {
public:
	enum InterpolationType {
		LINEAR,
		BEZIER,
		HOLD,
		EASE_IN,        // イージングイン補間
		EASE_OUT,       // イージングアウト補間
		EASE_IN_OUT,    // イージングイン・アウト補間
		CUBIC,          // 3次スプライン補間
		HERMITE,        // エルミート補間
		CATMULL_ROM     // カトマル・ロム補間
	};
	
	struct TemporalEase {
		float speed;
		float influence;
		
		TemporalEase() : speed(0.0f), influence(16.666666667f) {}
	};
	
	// カスタムイージング制御点
	struct EaseControlPoint {
		glm::vec2 point;
		glm::vec2 in_tangent;
		glm::vec2 out_tangent;
		
		EaseControlPoint() : point(0.0f, 0.0f), in_tangent(0.0f, 0.0f), out_tangent(0.0f, 0.0f) {}
		EaseControlPoint(float x, float y) : point(x, y), in_tangent(0.0f, 0.0f), out_tangent(0.0f, 0.0f) {}
	};
	
	// 複数セグメント対応のイージング
	struct EaseSegment {
		float start_time;
		float end_time;
		InterpolationType type;
		std::vector<EaseControlPoint> control_points;
		
		EaseSegment() : start_time(0.0f), end_time(1.0f), type(LINEAR) {}
	};
	
	struct InterpolationData {
		InterpolationType in_type;
		InterpolationType out_type;
		bool roving;
		bool continuous;
		TemporalEase in_ease;
		TemporalEase out_ease;
		
		InterpolationData() : in_type(LINEAR), out_type(LINEAR), roving(false), continuous(false) {}
	};
	
	struct SpatialTangents {
		std::vector<float> in_tangent;
		std::vector<float> out_tangent;
	};
	
	struct KeyframeData {
		int frame;
		ofJson value;
		InterpolationData interpolation;
		SpatialTangents spatial_tangents;
		
		KeyframeData() : frame(0) {}
	};
	
	static bool parseKeyframes(const ofJson &keyframe_data, std::vector<KeyframeData> &result);
	static ofJson interpolateValue(const KeyframeData &key1, const KeyframeData &key2, float t);
	static float calculateTemporalEasing(const TemporalEase &ease, float t);
	static InterpolationType stringToInterpolationType(const std::string &type_str);
	static std::string interpolationTypeToString(InterpolationType type);
	
	// 高度な補間メソッド
	static ofJson interpolateValueAdvanced(const KeyframeData &key1, const KeyframeData &key2, float t, InterpolationType type);
	static float applyEasingFunction(float t, InterpolationType type);
	static float customEasingFunction(float t, const std::vector<EaseControlPoint> &control_points);
	
	// 空間補間用メソッド
	static glm::vec3 interpolateSpatialPath(const std::vector<KeyframeData> &keyframes, float time);
	static glm::vec3 calculateSpatialTangent(const std::vector<KeyframeData> &keyframes, float time);
	static float calculatePathLength(const std::vector<KeyframeData> &keyframes);
	
	// パフォーマンス最適化用キャッシュ
	struct InterpolationCache {
		std::unordered_map<std::string, ofJson> value_cache;
		std::unordered_map<std::string, glm::vec3> spatial_cache;
		int last_frame;
		bool dirty;
		
		InterpolationCache() : last_frame(-1), dirty(true) {}
		void invalidate() { dirty = true; value_cache.clear(); spatial_cache.clear(); }
	};
	
	static InterpolationCache& getCache();
	
private:
	static bool parseInterpolationData(const ofJson &interp_json, InterpolationData &result);
	static bool parseSpatialTangents(const ofJson &spatial_json, SpatialTangents &result);
	static bool parseTemporalEase(const ofJson &ease_json, TemporalEase &result);
	
	// 高度な補間の内部メソッド
	static float cubicInterpolation(float t, float p0, float p1, float p2, float p3);
	static float hermiteInterpolation(float t, float p0, float p1, float t0, float t1);
	static float catmullRomInterpolation(float t, float p0, float p1, float p2, float p3);
public:
	static glm::vec3 bezierSpatialInterpolation(const KeyframeData &key1, const KeyframeData &key2, float t);
private:
	
	// 補間タイプごとの処理
	static float interpolateScalar(float val1, float val2, float t, InterpolationType type);
	static std::vector<float> interpolateVector(const std::vector<float> &vec1, const std::vector<float> &vec2, float t, InterpolationType type);
	
	// キャッシュシステム
	static thread_local InterpolationCache cache_;
	static std::string generateCacheKey(const KeyframeData &key1, const KeyframeData &key2, float t, InterpolationType type);
};

// 空間補間専用クラス
class SpatialInterpolator {
public:
	static glm::vec3 interpolatePath(const std::vector<Keyframe::KeyframeData> &keyframes, float time);
	static glm::vec3 calculateTangent(const std::vector<Keyframe::KeyframeData> &keyframes, float time);
	static float calculatePathLength(const std::vector<Keyframe::KeyframeData> &keyframes);
	static std::vector<glm::vec3> generatePathPoints(const std::vector<Keyframe::KeyframeData> &keyframes, int resolution = 100);
	
private:
public:
	static glm::vec3 bezierCurve(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, float t);
private:
	static float calculateSegmentLength(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, int subdivisions = 20);
};

// 時間補間拡張クラス
class TemporalEasing {
public:
	static float customEase(float t, const std::vector<glm::vec2> &control_points);
	static float multiSegmentEase(float t, const std::vector<Keyframe::EaseSegment> &segments);
	static float graphEditorCompatibleEase(float t, const Keyframe::TemporalEase &in_ease, const Keyframe::TemporalEase &out_ease);
	
	// 標準イージング関数
	static float easeIn(float t, float power = 2.0f);
	static float easeOut(float t, float power = 2.0f);
	static float easeInOut(float t, float power = 2.0f);
	
	// 高度なイージング関数
	static float easeInElastic(float t);
	static float easeOutElastic(float t);
	static float easeInOutElastic(float t);
	static float easeInBounce(float t);
	static float easeOutBounce(float t);
	static float easeInOutBounce(float t);
	
private:
public:
	static float bezierEasing(float t, float p1x, float p1y, float p2x, float p2y);
private:
	static float findBezierT(float x, float p1x, float p2x);
};

}}