#include "ofxAECompositionLayer.h"
#include "ofxAECompositionManager.h"
#include "ofLog.h"
#include "ofGraphics.h"
#include "ofUtils.h"
#include <filesystem>
#include <algorithm>

namespace ofx { namespace ae {

// Static member initialization
int CompositionLayer::max_hierarchy_depth_ = 10;

CompositionLayer::CompositionLayer() 
    : Layer()
    , child_composition_(nullptr)
    , has_time_remapping_(false)
    , hierarchy_depth_(0)
    , fbo_needs_update_(true)
    , last_render_time_(-1.0f)
    , cached_valid_(false)
    , cache_time_(-1.0f) {
}

CompositionLayer::~CompositionLayer() {
    // FBOは自動的にクリーンアップされる
    if (render_fbo_.isAllocated()) {
        render_fbo_.clear();
    }
}

bool CompositionLayer::setup(const ofJson &json) {
    // 基底クラスのsetupを呼び出し
    if (!Layer::setup(json)) {
        return false;
    }
    
    // CompositionLayer固有の設定
    if (json.contains("compositionPath") && json["compositionPath"].is_string()) {
        std::string compPath = json["compositionPath"].get<std::string>();
        if (!loadComposition(compPath)) {
            ofLogError("ofxAECompositionLayer") << "Failed to load composition: " << compPath;
            return false;
        }
    }
    
    // タイムリマッピングの解析
    if (json.contains("timeRemapping")) {
        parseTimeRemappingFromJson(json["timeRemapping"]);
    }
    
    // 階層深度の設定（デバッグ用）
    if (json.contains("hierarchyDepth") && json["hierarchyDepth"].is_number()) {
        hierarchy_depth_ = json["hierarchyDepth"].get<int>();
        
        // 最大深度チェック
        if (hierarchy_depth_ > max_hierarchy_depth_) {
            ofLogError("ofxAECompositionLayer") << "Hierarchy depth exceeds maximum: " 
                                                << hierarchy_depth_ << " > " << max_hierarchy_depth_;
            return false;
        }
    }
    
    return true;
}

void CompositionLayer::update() {
    // 基底クラスのupdateを呼び出し
    Layer::update();
    
    if (child_composition_) {
        // 子コンポジションの時間を計算
        float nestedTime = calculateNestedTime(static_cast<float>(getCurrentFrame()));
        
        // 子コンポジションのフレームを設定
        child_composition_->setCurrentFrame(static_cast<int>(nestedTime));
        
        // 子コンポジションを更新
        child_composition_->update();
        
        // レンダリング更新が必要かチェック
        if (needsRender(nestedTime)) {
            fbo_needs_update_ = true;
            invalidateCache();
        }
    }
}

void CompositionLayer::draw(float x, float y, float w, float h) const {
    if (!isVisible() || getOpacity() <= 0.0f || !child_composition_) {
        return;
    }
    
    // Transform適用
    pushMatrix();
    
    // FBOの初期化
    if (!isFboValid()) {
        initializeFbo();
    }
    
    // 現在の時間を計算
    float currentTime = calculateNestedTime(static_cast<float>(getCurrentFrame()));
    
    // FBOにレンダリング（必要な場合のみ）
    if (fbo_needs_update_ || needsRender(currentTime)) {
        renderToFbo(currentTime);
        fbo_needs_update_ = false;
        last_render_time_ = currentTime;
    }
    
    // FBOテクスチャを描画
    if (render_fbo_.isAllocated()) {
        ofSetColor(255, 255, 255, static_cast<int>(getOpacity() * 255));
        render_fbo_.draw(x, y, w, h);
        ofSetColor(255);
    }
    
    popMatrix();
}

float CompositionLayer::getHeight() const {
    return child_composition_ ? child_composition_->getHeight() : 0.0f;
}

float CompositionLayer::getWidth() const {
    return child_composition_ ? child_composition_->getWidth() : 0.0f;
}

bool CompositionLayer::loadComposition(const std::string& path) {
    auto& manager = CompositionManager::getInstance();
    
    // 循環参照チェック
    if (manager.hasCircularDependency(composition_id_, path)) {
        ofLogError("ofxAECompositionLayer") << "Circular dependency detected: "
                                           << composition_id_ << " -> " << path;
        return false;
    }
    
    // 既に読み込まれているコンポジションを取得
    child_composition_ = manager.getComposition(path);
    
    if (!child_composition_) {
        // 新しいCompositionインスタンスを作成
        child_composition_ = std::make_shared<Composition>();
        
        // コンポジションを読み込み
        if (!child_composition_->load(path)) {
            ofLogError("ofxAECompositionLayer") << "Failed to load composition from: " << path;
            child_composition_.reset();
            return false;
        }
        
        // CompositionManagerに登録
        manager.registerComposition(path, child_composition_);
    }
    
    // 依存関係を追加
    if (!composition_id_.empty()) {
        manager.addDependency(composition_id_, path);
    }
    
    // 依存関係IDを更新
    updateDependencyIds(path);
    
    // FBOのサイズを子コンポジションに合わせて設定
    const auto& info = child_composition_->getInfo();
    setFboSize(info.width, info.height);
    
    ofLogVerbose("ofxAECompositionLayer") << "Successfully loaded composition: " << path;
    return true;
}

void CompositionLayer::setTimeRemapping(const std::vector<TimeRemappingKeyframe>& keyframes) {
    time_remapping_keyframes_ = keyframes;
    has_time_remapping_ = !keyframes.empty();
    invalidateCache();
}

float CompositionLayer::calculateNestedTime(float parentTime) const {
    if (!child_composition_) {
        return 0.0f;
    }
    
    // レイヤーのinPoint/outPointを考慮した時間計算
    float layerTime = getLayerTime(parentTime);
    
    // タイムリマッピングが設定されている場合
    if (has_time_remapping_) {
        layerTime = interpolateTimeRemapping(layerTime);
    }
    
    // 子コンポジションの時間範囲にクランプ
    const auto& info = child_composition_->getInfo();
    return std::max(static_cast<float>(info.start_frame), 
                   std::min(layerTime, static_cast<float>(info.end_frame)));
}

bool CompositionLayer::hasCircularDependency(const std::string& compositionId,
                                            std::unordered_set<std::string>& visited) {
    // この静的メソッドは実際には使用されていないため、
    // 単純にfalseを返すか、適切な実装に変更する
    return false;
}

void CompositionLayer::setFboSize(int width, int height) {
    if (width <= 0 || height <= 0) {
        ofLogWarning("ofxAECompositionLayer") << "Invalid FBO size: " << width << "x" << height;
        return;
    }
    
    // FBOを再割り当て
    if (render_fbo_.isAllocated()) {
        if (render_fbo_.getWidth() != width || render_fbo_.getHeight() != height) {
            render_fbo_.clear();
            render_fbo_.allocate(width, height, GL_RGBA);
            fbo_needs_update_ = true;
        }
    } else {
        render_fbo_.allocate(width, height, GL_RGBA);
        fbo_needs_update_ = true;
    }
}

void CompositionLayer::initializeFbo() const {
    if (!child_composition_) {
        return;
    }
    
    const auto& info = child_composition_->getInfo();
    if (info.width > 0 && info.height > 0) {
        const_cast<CompositionLayer*>(this)->setFboSize(info.width, info.height);
    }
}

bool CompositionLayer::isFboValid() const {
    return render_fbo_.isAllocated() && 
           render_fbo_.getWidth() > 0 && 
           render_fbo_.getHeight() > 0;
}

void CompositionLayer::renderToFbo(float currentTime) const {
    if (!child_composition_ || !isFboValid()) {
        return;
    }
    
    // FBOレンダリング開始
    render_fbo_.begin();
    ofClear(0, 0, 0, 0); // 透明でクリア
    
    // 子コンポジションを描画
    child_composition_->draw(0, 0, render_fbo_.getWidth(), render_fbo_.getHeight());
    
    render_fbo_.end();
    
    // キャッシュ更新
    cached_valid_ = true;
    cache_time_ = currentTime;
}

float CompositionLayer::interpolateTimeRemapping(float time) const {
    if (!has_time_remapping_ || time_remapping_keyframes_.empty()) {
        return time;
    }
    
    // キーフレームが1つの場合
    if (time_remapping_keyframes_.size() == 1) {
        return time_remapping_keyframes_[0].value;
    }
    
    // 時間範囲外の処理
    if (time <= time_remapping_keyframes_.front().time) {
        return time_remapping_keyframes_.front().value;
    }
    if (time >= time_remapping_keyframes_.back().time) {
        return time_remapping_keyframes_.back().value;
    }
    
    // 線形補間
    for (size_t i = 0; i < time_remapping_keyframes_.size() - 1; ++i) {
        const auto& key1 = time_remapping_keyframes_[i];
        const auto& key2 = time_remapping_keyframes_[i + 1];
        
        if (time >= key1.time && time <= key2.time) {
            float t = (time - key1.time) / (key2.time - key1.time);
            return key1.value + t * (key2.value - key1.value);
        }
    }
    
    return time;
}

void CompositionLayer::parseTimeRemappingFromJson(const ofJson& json) {
    time_remapping_keyframes_.clear();
    
    if (!json.is_array()) {
        return;
    }
    
    for (const auto& keyframe : json) {
        if (keyframe.contains("time") && keyframe.contains("value") &&
            keyframe["time"].is_number() && keyframe["value"].is_number()) {
            
            float time = keyframe["time"].get<float>();
            float value = keyframe["value"].get<float>();
            time_remapping_keyframes_.emplace_back(time, value);
        }
    }
    
    // 時間順にソート
    std::sort(time_remapping_keyframes_.begin(), time_remapping_keyframes_.end(),
              [](const TimeRemappingKeyframe& a, const TimeRemappingKeyframe& b) {
                  return a.time < b.time;
              });
    
    has_time_remapping_ = !time_remapping_keyframes_.empty();
    
    ofLogVerbose("ofxAECompositionLayer") << "Loaded " << time_remapping_keyframes_.size() 
                                          << " time remapping keyframes";
}

void CompositionLayer::updateDependencyIds(const std::string& compositionPath) {
    composition_id_ = compositionPath;
    dependency_ids_.clear();
    dependency_ids_.insert(compositionPath);
    
    // 実際の実装では、compositionPathから依存する他のコンポジションも
    // 再帰的に解析して依存関係を構築する
}

void CompositionLayer::invalidateCache() {
    cached_valid_ = false;
    cache_time_ = -1.0f;
    fbo_needs_update_ = true;
}

bool CompositionLayer::needsRender(float currentTime) const {
    if (!cached_valid_) {
        return true;
    }
    
    // 時間が変更された場合
    if (std::abs(currentTime - cache_time_) > 0.001f) {
        return true;
    }
    
    // 子コンポジションが更新された場合の判定
    // 実際の実装では、子コンポジションの状態も考慮する
    
    return false;
}

}} // namespace ofx::ae