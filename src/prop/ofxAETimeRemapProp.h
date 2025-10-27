#pragma once

#include "ofxAEProperty.h"
#include <optional>

namespace ofx { namespace ae {

/**
 * Time Remap Property Class
 * 
 * After Effectsのタイムリマップ機能を実装するクラス
 * FloatPropを継承し、フレーム値の非線形マッピングを提供
 */
class TimeRemapProp : public FloatProp
{
public:
    TimeRemapProp();
    
    /**
     * タイムリマップの有効/無効を設定
     * @param enabled タイムリマップを有効にするかどうか
     */
    void setEnabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * タイムリマップが有効かどうかを取得
     * @return タイムリマップが有効な場合true
     */
    bool isEnabled() const { return enabled_; }
    
    /**
     * 入力フレームをリマップされたフレーム値に変換
     * @param inputFrame 入力フレーム番号
     * @return リマップされたフレーム値（float精度）
     */
    float remapFrame(int inputFrame) const;
    
    /**
     * フレーム設定をオーバーライド（キャッシュ管理のため）
     * @param frame 設定するフレーム番号
     * @return 値が変更された場合true
     */
    bool setFrame(int frame) override;
    
    /**
     * プロパティのセットアップ
     * @param base ベースプロパティのJSON
     * @param keyframes キーフレームデータのJSON
     */
    void setup(const ofJson &base, const ofJson &keyframes) override;
    
    /**
     * デバッグ情報の取得
     * @return デバッグ情報文字列
     */
    std::string getDebugInfo() const;

protected:
    /**
     * フレーム値をクランプする
     * @param frame クランプするフレーム値
     * @param minFrame 最小フレーム値
     * @param maxFrame 最大フレーム値
     * @return クランプされたフレーム値
     */
    float clampFrame(float frame, float minFrame, float maxFrame) const;

private:
    bool enabled_;                                    ///< タイムリマップの有効フラグ
    mutable std::optional<float> remapped_frame_cache_; ///< リマップ結果のキャッシュ
    mutable int last_input_frame_;                    ///< 最後に処理した入力フレーム
};

}} // namespace ofx::ae