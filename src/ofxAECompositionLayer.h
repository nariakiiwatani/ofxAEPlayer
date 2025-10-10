#pragma once

#include "ofxAELayer.h"
#include "ofxAEComposition.h"
#include "ofFbo.h"
#include <memory>
#include <unordered_set>
#include <functional>

namespace ofx { namespace ae {

/**
 * @brief CompositionLayer - 入れ子Composition機能を実装するレイヤークラス
 * 
 * After Effectsのプリコンポジション機能を再現し、Layerとして動作する
 * Compositionを含むことで階層構造を実現します。
 */
class CompositionLayer : public Layer {
public:
    struct TimeRemappingKeyframe {
        float time;
        float value;
        
        TimeRemappingKeyframe(float t, float v) : time(t), value(v) {}
    };

    CompositionLayer();
    virtual ~CompositionLayer();

    // Layer interface override
    bool setup(const ofJson &json) override;
    void update() override;
    void draw(float x, float y, float w, float h) const override;
    float getHeight() const override;
    float getWidth() const override;

    // CompositionLayer specific methods
    
    /**
     * @brief 子コンポジションを読み込み
     * @param path コンポジションファイルのパス
     * @return 読み込み成功フラグ
     */
    bool loadComposition(const std::string& path);
    
    /**
     * @brief タイムリマッピングキーフレームを設定
     * @param keyframes タイムリマッピングキーフレームの配列
     */
    void setTimeRemapping(const std::vector<TimeRemappingKeyframe>& keyframes);
    
    /**
     * @brief 親コンポジションの時間を子コンポジションの時間に変換
     * @param parentTime 親コンポジションの時間
     * @return 子コンポジションの時間
     */
    float calculateNestedTime(float parentTime) const;
    
    /**
     * @brief 循環参照検出のためのIDセットを取得
     * @return 現在の依存関係ID
     */
    const std::unordered_set<std::string>& getDependencyIds() const { return dependency_ids_; }
    
    /**
     * @brief 循環参照チェック
     * @param compositionId チェック対象のコンポジションID
     * @param visited 訪問済みIDセット
     * @return 循環参照があればtrue
     */
    static bool hasCircularDependency(const std::string& compositionId, 
                                     std::unordered_set<std::string>& visited);
    
    /**
     * @brief 子コンポジションを取得
     * @return 子コンポジションのポインタ（nullptrの場合は未読み込み）
     */
    std::shared_ptr<Composition> getChildComposition() const { return child_composition_; }
    
    /**
     * @brief FBOサイズを設定
     * @param width 幅
     * @param height 高さ
     */
    void setFboSize(int width, int height);
    
    /**
     * @brief デバッグ用：階層深度を取得
     * @return 現在の階層深度
     */
    int getHierarchyDepth() const { return hierarchy_depth_; }
    
    /**
     * @brief 最大階層深度を設定（無限再帰防止）
     * @param maxDepth 最大深度
     */
    static void setMaxHierarchyDepth(int maxDepth) { max_hierarchy_depth_ = maxDepth; }

private:
    // 子コンポジション
    std::shared_ptr<Composition> child_composition_;
    
    // FBOレンダリングターゲット
    mutable ofFbo render_fbo_;
    
    // タイムリマッピング
    std::vector<TimeRemappingKeyframe> time_remapping_keyframes_;
    bool has_time_remapping_;
    
    // 循環参照検出
    std::unordered_set<std::string> dependency_ids_;
    std::string composition_id_;
    
    // 階層管理
    int hierarchy_depth_;
    static int max_hierarchy_depth_;
    
    // レンダリング状態
    mutable bool fbo_needs_update_;
    mutable float last_render_time_;
    
    // パフォーマンス最適化
    mutable bool cached_valid_;
    mutable float cache_time_;
    
    /**
     * @brief FBOを初期化
     */
    void initializeFbo() const;
    
    /**
     * @brief FBOが有効かチェック
     * @return FBOが使用可能ならtrue
     */
    bool isFboValid() const;
    
    /**
     * @brief 子コンポジションをFBOにレンダリング
     * @param currentTime 現在の時間
     */
    void renderToFbo(float currentTime) const;
    
    /**
     * @brief タイムリマッピング補間
     * @param time 補間する時間
     * @return リマップされた時間
     */
    float interpolateTimeRemapping(float time) const;
    
    /**
     * @brief JSONからタイムリマッピングデータを解析
     * @param json JSONデータ
     */
    void parseTimeRemappingFromJson(const ofJson& json);
    
    /**
     * @brief 依存関係IDを更新
     * @param compositionPath コンポジションパス
     */
    void updateDependencyIds(const std::string& compositionPath);
    
    /**
     * @brief キャッシュ無効化
     */
    void invalidateCache();
    
    /**
     * @brief レンダリングが必要かチェック
     * @param currentTime 現在の時間
     * @return レンダリングが必要ならtrue
     */
    bool needsRender(float currentTime) const;
};

}} // namespace ofx::ae