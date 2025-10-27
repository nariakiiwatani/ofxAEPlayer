#include "ofxAETimeRemapProp.h"
#include "ofLog.h"
#include "ofUtils.h"

namespace ofx { namespace ae {

TimeRemapProp::TimeRemapProp() : FloatProp(), enabled_(false), last_input_frame_(-1) {
}

float TimeRemapProp::remapFrame(int inputFrame) const {
    if (!enabled_) {
        return static_cast<float>(inputFrame);
    }
    
    // キャッシュチェック
    if (last_input_frame_ == inputFrame && remapped_frame_cache_.has_value()) {
        return *remapped_frame_cache_;
    }
    
    // キーフレームが存在しない場合は入力フレームをそのまま返す
    if (!hasAnimation()) {
        float result = static_cast<float>(inputFrame);
        last_input_frame_ = inputFrame;
        remapped_frame_cache_ = result;
        return result;
    }
    
    // 現在のフレームでプロパティを評価
    const_cast<TimeRemapProp*>(this)->setFrame(inputFrame);
    float remappedFrame = get();
    
    // キャッシュを更新
    last_input_frame_ = inputFrame;
    remapped_frame_cache_ = remappedFrame;
    
    return remappedFrame;
}

bool TimeRemapProp::setFrame(int frame) {
    // FloatPropの基底実装を呼び出し
    bool changed = FloatProp::setFrame(frame);
    
    // キャッシュをクリア（フレームが変わった場合）
    if (changed) {
        remapped_frame_cache_.reset();
        last_input_frame_ = -1;
    }
    
    return changed;
}

void TimeRemapProp::setup(const ofJson &base, const ofJson &keyframes) {
    // enabled状態を設定
    if (base.contains("enabled") && base["enabled"].is_boolean()) {
        enabled_ = base["enabled"].get<bool>();
    }
    
    // 基底クラスのsetupを呼び出し
    FloatProp::setup(base, keyframes);
    
    // キャッシュをリセット
    remapped_frame_cache_.reset();
    last_input_frame_ = -1;
}

float TimeRemapProp::clampFrame(float frame, float minFrame, float maxFrame) const {
    return std::max(minFrame, std::min(maxFrame, frame));
}

std::string TimeRemapProp::getDebugInfo() const {
    std::stringstream info;
    info << "TimeRemapProp[enabled:" << (enabled_ ? "true" : "false");
    info << ", hasAnimation:" << (hasAnimation() ? "true" : "false");
    if (remapped_frame_cache_.has_value()) {
        info << ", cached:" << *remapped_frame_cache_;
    }
    info << "]";
    return info.str();
}

}} // namespace ofx::ae
