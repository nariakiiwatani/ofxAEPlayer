#include "ofxAEKeyframe.h"
#include "ofLog.h"
#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

namespace ofx { namespace ae {

bool Keyframe::parseKeyframes(const ofJson &keyframe_data, std::vector<KeyframeData> &result) {
	result.clear();
	
	if (!keyframe_data.is_array()) {
		ofLogError("ofxAEKeyframe") << "Keyframe data is not an array";
		return false;
	}
	
	for (const auto &keyframe : keyframe_data) {
		KeyframeData data;
		
		// フレーム番号の取得
		if (!keyframe.contains("frame") || !keyframe["frame"].is_number()) {
			ofLogError("ofxAEKeyframe") << "Keyframe missing or invalid frame number";
			continue;
		}
		data.frame = keyframe["frame"].get<int>();
		
		// 値の取得
		if (!keyframe.contains("value")) {
			ofLogError("ofxAEKeyframe") << "Keyframe missing value";
			continue;
		}
		data.value = keyframe["value"];
		
		// 補間データの解析
		if (keyframe.contains("interpolation")) {
			if (!parseInterpolationData(keyframe["interpolation"], data.interpolation)) {
				ofLogWarning("ofxAEKeyframe") << "Failed to parse interpolation data for frame " << data.frame;
			}
		}
		
		// 空間タンジェントの解析
		if (keyframe.contains("spatialTangents")) {
			if (!parseSpatialTangents(keyframe["spatialTangents"], data.spatial_tangents)) {
				ofLogWarning("ofxAEKeyframe") << "Failed to parse spatial tangents for frame " << data.frame;
			}
		}
		
		result.push_back(data);
	}
	
	// フレーム番号でソート
	std::sort(result.begin(), result.end(), [](const KeyframeData &a, const KeyframeData &b) {
		return a.frame < b.frame;
	});
	
	return !result.empty();
}

ofJson Keyframe::interpolateValue(const KeyframeData &key1, const KeyframeData &key2, float t) {
	// tは0.0から1.0の範囲
	t = std::max(0.0f, std::min(1.0f, t));
	
	// HOLDタイプの場合は前のキーフレームの値をそのまま返す
	if (key1.interpolation.out_type == HOLD) {
		return key1.value;
	}
	
	// キャッシュキーの生成
	std::string cache_key = generateCacheKey(key1, key2, t, key1.interpolation.out_type);
	auto& cache = getCache();
	
	// キャッシュから値を取得
	if (!cache.dirty && cache.value_cache.find(cache_key) != cache.value_cache.end()) {
		return cache.value_cache[cache_key];
	}
	
	// 時間補間の適用（After Effectsの temporalEase を使用）
	float eased_t = t;
	if (key1.interpolation.out_type == BEZIER) {
		eased_t = TemporalEasing::graphEditorCompatibleEase(t, key1.interpolation.in_ease, key1.interpolation.out_ease);
	} else {
		eased_t = applyEasingFunction(t, key1.interpolation.out_type);
	}
	
	ofJson result;
	
	// 空間補間が必要かチェック（位置プロパティでタンジェントが設定されている場合）
	bool has_spatial_tangents = (!key1.spatial_tangents.in_tangent.empty() ||
							 !key1.spatial_tangents.out_tangent.empty() ||
							 !key2.spatial_tangents.in_tangent.empty() ||
							 !key2.spatial_tangents.out_tangent.empty());
	
	// 値が配列（ベクトル）の場合
	if (key1.value.is_array() && key2.value.is_array()) {
		// 空間補間を使用（3次元位置データでタンジェントがある場合）
		if (has_spatial_tangents && key1.value.size() >= 3) {
			glm::vec3 spatial_result = bezierSpatialInterpolation(key1, key2, eased_t);
			result = ofJson::array();
			result.push_back(spatial_result.x);
			result.push_back(spatial_result.y);
			result.push_back(spatial_result.z);
		} else {
			// 通常のベクトル補間
			result = ofJson::array();
			std::size_t size = std::min(key1.value.size(), key2.value.size());
			
			for (std::size_t i = 0; i < size; ++i) {
				if (key1.value[i].is_number() && key2.value[i].is_number()) {
					float val1 = key1.value[i].get<float>();
					float val2 = key2.value[i].get<float>();
					float interpolated = interpolateScalar(val1, val2, eased_t, key1.interpolation.out_type);
					result.push_back(interpolated);
				} else {
					result.push_back(key1.value[i]);
				}
			}
		}
	}
	// 値がスカラーの場合
	else if (key1.value.is_number() && key2.value.is_number()) {
		float val1 = key1.value.get<float>();
		float val2 = key2.value.get<float>();
		result = interpolateScalar(val1, val2, eased_t, key1.interpolation.out_type);
	}
	// その他の場合は最初のキーフレームの値を返す
	else {
		result = key1.value;
	}
	
	// キャッシュに保存
	cache.value_cache[cache_key] = result;
	
	return result;
}

float Keyframe::calculateTemporalEasing(const TemporalEase &ease, float t) {
	// After Effects互換のより正確なイージング計算
	if (ease.influence <= 0.0f) {
		return t;
	}
	
	// After Effectsのスピードと影響値からベジェ制御点を計算
	float influence_factor = std::clamp(ease.influence / 100.0f, 0.0f, 1.0f);
	float speed_factor = ease.speed / 1000.0f;
	
	// ベジェ制御点の計算（After Effects互換）
	float p1x = influence_factor * 0.33f;
	float p1y = speed_factor * 0.33f;
	float p2x = 1.0f - influence_factor * 0.33f;
	float p2y = 1.0f - speed_factor * 0.33f;
	
	return TemporalEasing::bezierEasing(t, p1x, p1y, p2x, p2y);
}

Keyframe::InterpolationType Keyframe::stringToInterpolationType(const std::string &type_str) {
	if (type_str == "LINEAR") return LINEAR;
	if (type_str == "BEZIER") return BEZIER;
	if (type_str == "HOLD") return HOLD;
	if (type_str == "EASE_IN") return EASE_IN;
	if (type_str == "EASE_OUT") return EASE_OUT;
	if (type_str == "EASE_IN_OUT") return EASE_IN_OUT;
	if (type_str == "CUBIC") return CUBIC;
	if (type_str == "HERMITE") return HERMITE;
	if (type_str == "CATMULL_ROM") return CATMULL_ROM;
	return LINEAR; // デフォルト
}

std::string Keyframe::interpolationTypeToString(InterpolationType type) {
	switch (type) {
		case LINEAR: return "LINEAR";
		case BEZIER: return "BEZIER";
		case HOLD: return "HOLD";
		case EASE_IN: return "EASE_IN";
		case EASE_OUT: return "EASE_OUT";
		case EASE_IN_OUT: return "EASE_IN_OUT";
		case CUBIC: return "CUBIC";
		case HERMITE: return "HERMITE";
		case CATMULL_ROM: return "CATMULL_ROM";
		default: return "LINEAR";
	}
}

bool Keyframe::parseInterpolationData(const ofJson &interp_json, InterpolationData &result) {
	// デフォルト値の設定
	result.in_type = LINEAR;
	result.out_type = LINEAR;
	result.roving = false;
	result.continuous = false;
	
	if (interp_json.contains("inType") && interp_json["inType"].is_string()) {
		result.in_type = stringToInterpolationType(interp_json["inType"].get<std::string>());
	}
	
	if (interp_json.contains("outType") && interp_json["outType"].is_string()) {
		result.out_type = stringToInterpolationType(interp_json["outType"].get<std::string>());
	}
	
	if (interp_json.contains("roving") && interp_json["roving"].is_boolean()) {
		result.roving = interp_json["roving"].get<bool>();
	}
	
	if (interp_json.contains("continuous") && interp_json["continuous"].is_boolean()) {
		result.continuous = interp_json["continuous"].get<bool>();
	}
	
	// TemporalEaseの解析
	if (interp_json.contains("temporalEase")) {
		const auto &ease_json = interp_json["temporalEase"];
		
		if (ease_json.contains("inEase")) {
			parseTemporalEase(ease_json["inEase"], result.in_ease);
		}
		
		if (ease_json.contains("outEase")) {
			parseTemporalEase(ease_json["outEase"], result.out_ease);
		}
	}
	
	return true;
}

bool Keyframe::parseSpatialTangents(const ofJson &spatial_json, SpatialTangents &result) {
	if (spatial_json.contains("inTangent") && spatial_json["inTangent"].is_array()) {
		result.in_tangent.clear();
		for (const auto &val : spatial_json["inTangent"]) {
			if (val.is_number()) {
				result.in_tangent.push_back(val.get<float>());
			}
		}
	}
	
	if (spatial_json.contains("outTangent") && spatial_json["outTangent"].is_array()) {
		result.out_tangent.clear();
		for (const auto &val : spatial_json["outTangent"]) {
			if (val.is_number()) {
				result.out_tangent.push_back(val.get<float>());
			}
		}
	}
	
	return true;
}

bool Keyframe::parseTemporalEase(const ofJson &ease_json, TemporalEase &result) {
	result.speed = 0.0f;
	result.influence = 16.666666667f; // デフォルト値
	
	if (ease_json.contains("speed") && ease_json["speed"].is_number()) {
		result.speed = ease_json["speed"].get<float>();
	}
	
	if (ease_json.contains("influence") && ease_json["influence"].is_number()) {
		result.influence = ease_json["influence"].get<float>();
	}
	
	return true;
}

// 高度な補間メソッドの実装
ofJson Keyframe::interpolateValueAdvanced(const KeyframeData &key1, const KeyframeData &key2, float t, InterpolationType type) {
	t = std::max(0.0f, std::min(1.0f, t));
	
	// HOLDタイプの場合は前のキーフレームの値をそのまま返す
	if (type == HOLD) {
		return key1.value;
	}
	
	// 時間補間を適用
	float eased_t = applyEasingFunction(t, type);
	
	// 値が配列（ベクトル）の場合
	if (key1.value.is_array() && key2.value.is_array()) {
		ofJson result = ofJson::array();
		std::size_t size = std::min(key1.value.size(), key2.value.size());
		
		for (std::size_t i = 0; i < size; ++i) {
			if (key1.value[i].is_number() && key2.value[i].is_number()) {
				float val1 = key1.value[i].get<float>();
				float val2 = key2.value[i].get<float>();
				float interpolated = interpolateScalar(val1, val2, eased_t, type);
				result.push_back(interpolated);
			} else {
				result.push_back(key1.value[i]);
			}
		}
		return result;
	}
	
	// 値がスカラーの場合
	if (key1.value.is_number() && key2.value.is_number()) {
		float val1 = key1.value.get<float>();
		float val2 = key2.value.get<float>();
		return interpolateScalar(val1, val2, eased_t, type);
	}
	
	// その他の場合は最初のキーフレームの値を返す
	return key1.value;
}

float Keyframe::applyEasingFunction(float t, InterpolationType type) {
	switch (type) {
		case LINEAR:
			return t;
		case EASE_IN:
			return TemporalEasing::easeIn(t);
		case EASE_OUT:
			return TemporalEasing::easeOut(t);
		case EASE_IN_OUT:
			return TemporalEasing::easeInOut(t);
		case CUBIC:
		case HERMITE:
		case CATMULL_ROM:
		case BEZIER:
		default:
			return t; // 基本的な線形補間
	}
}

float Keyframe::customEasingFunction(float t, const std::vector<EaseControlPoint> &control_points) {
	if (control_points.empty()) {
		return t;
	}
	
	// 制御点ベースのカスタムイージング
	// ベジェ曲線を使用して補間
	if (control_points.size() >= 2) {
		glm::vec2 p0(0.0f, 0.0f);
		glm::vec2 p1 = control_points[0].point;
		glm::vec2 p2 = control_points[control_points.size() - 1].point;
		glm::vec2 p3(1.0f, 1.0f);
		
		return TemporalEasing::bezierEasing(t, p1.x, p1.y, p2.x, p2.y);
	}
	
	return t;
}

// 空間補間メソッド
glm::vec3 Keyframe::interpolateSpatialPath(const std::vector<KeyframeData> &keyframes, float time) {
	return SpatialInterpolator::interpolatePath(keyframes, time);
}

glm::vec3 Keyframe::calculateSpatialTangent(const std::vector<KeyframeData> &keyframes, float time) {
	return SpatialInterpolator::calculateTangent(keyframes, time);
}

float Keyframe::calculatePathLength(const std::vector<KeyframeData> &keyframes) {
	return SpatialInterpolator::calculatePathLength(keyframes);
}

// 高度な補間の内部メソッド
float Keyframe::cubicInterpolation(float t, float p0, float p1, float p2, float p3) {
	float t2 = t * t;
	float t3 = t2 * t;
	
	float a0 = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
	float a1 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
	float a2 = -0.5f * p0 + 0.5f * p2;
	float a3 = p1;
	
	return a0 * t3 + a1 * t2 + a2 * t + a3;
}

float Keyframe::hermiteInterpolation(float t, float p0, float p1, float t0, float t1) {
	float t2 = t * t;
	float t3 = t2 * t;
	
	float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
	float h10 = t3 - 2.0f * t2 + t;
	float h01 = -2.0f * t3 + 3.0f * t2;
	float h11 = t3 - t2;
	
	return h00 * p0 + h10 * t0 + h01 * p1 + h11 * t1;
}

float Keyframe::catmullRomInterpolation(float t, float p0, float p1, float p2, float p3) {
	float t2 = t * t;
	float t3 = t2 * t;
	
	return 0.5f * ((2.0f * p1) +
				   (-p0 + p2) * t +
				   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
				   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

glm::vec3 Keyframe::bezierSpatialInterpolation(const KeyframeData &key1, const KeyframeData &key2, float t) {
	// 空間タンジェントを使用したベジェ補間
	glm::vec3 p0, p1, p2, p3;
	
	// キーフレーム値から位置を取得
	if (key1.value.is_array() && key1.value.size() >= 3) {
		p0 = glm::vec3(key1.value[0].get<float>(), key1.value[1].get<float>(), key1.value[2].get<float>());
	}
	if (key2.value.is_array() && key2.value.size() >= 3) {
		p3 = glm::vec3(key2.value[0].get<float>(), key2.value[1].get<float>(), key2.value[2].get<float>());
	}
	
	// 制御点の計算（空間タンジェントを使用）
	p1 = p0;
	p2 = p3;
	
	if (key1.spatial_tangents.out_tangent.size() >= 2) {
		p1.x += key1.spatial_tangents.out_tangent[0];
		p1.y += key1.spatial_tangents.out_tangent[1];
	}
	if (key2.spatial_tangents.in_tangent.size() >= 2) {
		p2.x += key2.spatial_tangents.in_tangent[0];
		p2.y += key2.spatial_tangents.in_tangent[1];
	}
	
	return SpatialInterpolator::bezierCurve(p0, p1, p2, p3, t);
}

float Keyframe::interpolateScalar(float val1, float val2, float t, InterpolationType type) {
	switch (type) {
		case LINEAR:
		case EASE_IN:
		case EASE_OUT:
		case EASE_IN_OUT:
			return val1 + (val2 - val1) * t;
		case CUBIC:
			// 3次補間（追加のキーフレームが必要だが、ここでは線形で代用）
			return val1 + (val2 - val1) * t;
		case HERMITE:
			// エルミート補間（タンジェントが必要だが、ここでは線形で代用）
			return val1 + (val2 - val1) * t;
		case CATMULL_ROM:
			// カトマル・ロム補間（4点が必要だが、ここでは線形で代用）
			return val1 + (val2 - val1) * t;
		case BEZIER:
		case HOLD:
		default:
			return val1 + (val2 - val1) * t;
	}
}

std::vector<float> Keyframe::interpolateVector(const std::vector<float> &vec1, const std::vector<float> &vec2, float t, InterpolationType type) {
	std::vector<float> result;
	std::size_t size = std::min(vec1.size(), vec2.size());
	
	for (std::size_t i = 0; i < size; ++i) {
		result.push_back(interpolateScalar(vec1[i], vec2[i], t, type));
	}
	
	return result;
}

// キャッシュシステム
thread_local Keyframe::InterpolationCache Keyframe::cache_;

Keyframe::InterpolationCache& Keyframe::getCache() {
	return cache_;
}

std::string Keyframe::generateCacheKey(const KeyframeData &key1, const KeyframeData &key2, float t, InterpolationType type) {
	return std::to_string(key1.frame) + "_" + std::to_string(key2.frame) + "_" +
		   std::to_string(t) + "_" + std::to_string(static_cast<int>(type));
}

// SpatialInterpolator実装
glm::vec3 SpatialInterpolator::interpolatePath(const std::vector<Keyframe::KeyframeData> &keyframes, float time) {
	if (keyframes.empty()) {
		return glm::vec3(0.0f);
	}
	
	if (keyframes.size() == 1) {
		const auto &kf = keyframes[0];
		if (kf.value.is_array() && kf.value.size() >= 3) {
			return glm::vec3(kf.value[0].get<float>(), kf.value[1].get<float>(), kf.value[2].get<float>());
		}
		return glm::vec3(0.0f);
	}
	
	// 時間に基づいて適切なキーフレームペアを見つける
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		if (time >= keyframes[i].frame && time <= keyframes[i + 1].frame) {
			float t = (time - keyframes[i].frame) / (keyframes[i + 1].frame - keyframes[i].frame);
			return Keyframe::bezierSpatialInterpolation(keyframes[i], keyframes[i + 1], t);
		}
	}
	
	// 範囲外の場合は最初または最後のキーフレーム
	const auto &kf = (time < keyframes[0].frame) ? keyframes[0] : keyframes.back();
	if (kf.value.is_array() && kf.value.size() >= 3) {
		return glm::vec3(kf.value[0].get<float>(), kf.value[1].get<float>(), kf.value[2].get<float>());
	}
	
	return glm::vec3(0.0f);
}

glm::vec3 SpatialInterpolator::calculateTangent(const std::vector<Keyframe::KeyframeData> &keyframes, float time) {
	if (keyframes.size() < 2) {
		return glm::vec3(0.0f);
	}
	
	// 微小な時間差で数値微分により接線ベクトルを計算
	const float epsilon = 0.01f;
	glm::vec3 pos1 = interpolatePath(keyframes, time - epsilon);
	glm::vec3 pos2 = interpolatePath(keyframes, time + epsilon);
	
	return glm::normalize(pos2 - pos1);
}

float SpatialInterpolator::calculatePathLength(const std::vector<Keyframe::KeyframeData> &keyframes) {
	if (keyframes.size() < 2) {
		return 0.0f;
	}
	
	float totalLength = 0.0f;
	
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		glm::vec3 p0, p1, p2, p3;
		
		// 制御点の設定
		if (keyframes[i].value.is_array() && keyframes[i].value.size() >= 3) {
			p0 = glm::vec3(keyframes[i].value[0].get<float>(), keyframes[i].value[1].get<float>(), keyframes[i].value[2].get<float>());
		}
		if (keyframes[i + 1].value.is_array() && keyframes[i + 1].value.size() >= 3) {
			p3 = glm::vec3(keyframes[i + 1].value[0].get<float>(), keyframes[i + 1].value[1].get<float>(), keyframes[i + 1].value[2].get<float>());
		}
		
		p1 = p0;
		p2 = p3;
		
		if (keyframes[i].spatial_tangents.out_tangent.size() >= 2) {
			p1.x += keyframes[i].spatial_tangents.out_tangent[0];
			p1.y += keyframes[i].spatial_tangents.out_tangent[1];
		}
		if (keyframes[i + 1].spatial_tangents.in_tangent.size() >= 2) {
			p2.x += keyframes[i + 1].spatial_tangents.in_tangent[0];
			p2.y += keyframes[i + 1].spatial_tangents.in_tangent[1];
		}
		
		totalLength += calculateSegmentLength(p0, p1, p2, p3);
	}
	
	return totalLength;
}

std::vector<glm::vec3> SpatialInterpolator::generatePathPoints(const std::vector<Keyframe::KeyframeData> &keyframes, int resolution) {
	std::vector<glm::vec3> points;
	
	if (keyframes.size() < 2) {
		return points;
	}
	
	float startFrame = keyframes[0].frame;
	float endFrame = keyframes.back().frame;
	float step = (endFrame - startFrame) / static_cast<float>(resolution - 1);
	
	for (int i = 0; i < resolution; ++i) {
		float time = startFrame + i * step;
		points.push_back(interpolatePath(keyframes, time));
	}
	
	return points;
}

glm::vec3 SpatialInterpolator::bezierCurve(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, float t) {
	float inv_t = 1.0f - t;
	float inv_t2 = inv_t * inv_t;
	float inv_t3 = inv_t2 * inv_t;
	float t2 = t * t;
	float t3 = t2 * t;
	
	return inv_t3 * p0 + 3.0f * inv_t2 * t * p1 + 3.0f * inv_t * t2 * p2 + t3 * p3;
}

float SpatialInterpolator::calculateSegmentLength(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, int subdivisions) {
	float length = 0.0f;
	glm::vec3 prevPoint = p0;
	
	for (int i = 1; i <= subdivisions; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(subdivisions);
		glm::vec3 currentPoint = bezierCurve(p0, p1, p2, p3, t);
		length += glm::length(currentPoint - prevPoint);
		prevPoint = currentPoint;
	}
	
	return length;
}

// TemporalEasing実装
float TemporalEasing::customEase(float t, const std::vector<glm::vec2> &control_points) {
	if (control_points.size() < 2) {
		return t;
	}
	
	// ベジェ曲線による制御点ベースのイージング
	glm::vec2 p1 = control_points[0];
	glm::vec2 p2 = control_points[control_points.size() - 1];
	
	return bezierEasing(t, p1.x, p1.y, p2.x, p2.y);
}

float TemporalEasing::multiSegmentEase(float t, const std::vector<Keyframe::EaseSegment> &segments) {
	if (segments.empty()) {
		return t;
	}
	
	// 該当するセグメントを見つける
	for (const auto &segment : segments) {
		if (t >= segment.start_time && t <= segment.end_time) {
			float local_t = (t - segment.start_time) / (segment.end_time - segment.start_time);
			
			switch (segment.type) {
				case Keyframe::EASE_IN:
					return easeIn(local_t);
				case Keyframe::EASE_OUT:
					return easeOut(local_t);
				case Keyframe::EASE_IN_OUT:
					return easeInOut(local_t);
				default:
					return local_t;
			}
		}
	}
	
	return t;
}

float TemporalEasing::graphEditorCompatibleEase(float t, const Keyframe::TemporalEase &in_ease, const Keyframe::TemporalEase &out_ease) {
	// After Effectsのグラフエディタ互換のイージング計算
	float influence_factor = std::max(in_ease.influence, out_ease.influence) / 100.0f;
	float speed_factor = (in_ease.speed + out_ease.speed) / 2000.0f;
	
	// ベジェ近似によるイージング
	float p1x = influence_factor * 0.33f;
	float p1y = speed_factor * 0.33f;
	float p2x = 1.0f - influence_factor * 0.33f;
	float p2y = 1.0f - speed_factor * 0.33f;
	
	return bezierEasing(t, p1x, p1y, p2x, p2y);
}

float TemporalEasing::easeIn(float t, float power) {
	return std::pow(t, power);
}

float TemporalEasing::easeOut(float t, float power) {
	return 1.0f - std::pow(1.0f - t, power);
}

float TemporalEasing::easeInOut(float t, float power) {
	if (t < 0.5f) {
		return 0.5f * std::pow(2.0f * t, power);
	} else {
		return 1.0f - 0.5f * std::pow(2.0f * (1.0f - t), power);
	}
}

float TemporalEasing::easeInElastic(float t) {
	if (t == 0.0f) return 0.0f;
	if (t == 1.0f) return 1.0f;
	
	float p = 0.3f;
	float s = p / 4.0f;
	return -(std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.0f - s) * (2.0f * M_PI) / p));
}

float TemporalEasing::easeOutElastic(float t) {
	if (t == 0.0f) return 0.0f;
	if (t == 1.0f) return 1.0f;
	
	float p = 0.3f;
	float s = p / 4.0f;
	return std::pow(2.0f, -10.0f * t) * std::sin((t - s) * (2.0f * M_PI) / p) + 1.0f;
}

float TemporalEasing::easeInOutElastic(float t) {
	if (t == 0.0f) return 0.0f;
	if (t == 1.0f) return 1.0f;
	
	t *= 2.0f;
	float p = 0.3f * 1.5f;
	float s = p / 4.0f;
	
	if (t < 1.0f) {
		return -0.5f * (std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.0f - s) * (2.0f * M_PI) / p));
	}
	return std::pow(2.0f, -10.0f * (t - 1.0f)) * std::sin((t - 1.0f - s) * (2.0f * M_PI) / p) * 0.5f + 1.0f;
}

float TemporalEasing::easeInBounce(float t) {
	return 1.0f - easeOutBounce(1.0f - t);
}

float TemporalEasing::easeOutBounce(float t) {
	if (t < (1.0f / 2.75f)) {
		return 7.5625f * t * t;
	} else if (t < (2.0f / 2.75f)) {
		t -= (1.5f / 2.75f);
		return 7.5625f * t * t + 0.75f;
	} else if (t < (2.5f / 2.75f)) {
		t -= (2.25f / 2.75f);
		return 7.5625f * t * t + 0.9375f;
	} else {
		t -= (2.625f / 2.75f);
		return 7.5625f * t * t + 0.984375f;
	}
}

float TemporalEasing::easeInOutBounce(float t) {
	if (t < 0.5f) {
		return easeInBounce(t * 2.0f) * 0.5f;
	}
	return easeOutBounce(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

float TemporalEasing::bezierEasing(float t, float p1x, float p1y, float p2x, float p2y) {
	// ベジェ曲線による1次元イージング
	float bezier_t = findBezierT(t, p1x, p2x);
	
	// Y値の計算
	float inv_t = 1.0f - bezier_t;
	return 3.0f * inv_t * inv_t * bezier_t * p1y +
		   3.0f * inv_t * bezier_t * bezier_t * p2y +
		   bezier_t * bezier_t * bezier_t;
}

float TemporalEasing::findBezierT(float x, float p1x, float p2x) {
	// ニュートン法でtを求める
	float t = x;
	for (int i = 0; i < 10; ++i) {
		float inv_t = 1.0f - t;
		float f = 3.0f * inv_t * inv_t * t * p1x +
				  3.0f * inv_t * t * t * p2x +
				  t * t * t - x;
		
		if (std::abs(f) < 1e-6f) break;
		
		float df = 3.0f * inv_t * inv_t * p1x +
				   6.0f * inv_t * t * (p2x - p1x) +
				   3.0f * t * t * (1.0f - p2x);
		
		if (std::abs(df) < 1e-6f) break;
		
		t = t - f / df;
		t = std::max(0.0f, std::min(1.0f, t));
	}
	
	return t;
}

}}