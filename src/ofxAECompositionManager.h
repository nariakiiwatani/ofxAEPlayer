#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include "ofxAEComposition.h"

namespace ofx { namespace ae {

/**
 * @brief CompositionManager - コンポジション間の依存関係を管理し、循環参照を検出するシングルトンクラス
 * 
 * 全てのコンポジションの読み込みと依存関係を一元管理し、
 * 循環参照の検出と防止を行います。
 */
class CompositionManager {
public:
    // シングルトンインスタンス取得
    static CompositionManager& getInstance();

    /**
     * @brief コンポジションを登録
     * @param id コンポジションID（通常はファイルパス）
     * @param composition コンポジションオブジェクト
     * @return 登録成功フラグ
     */
    bool registerComposition(const std::string& id, std::shared_ptr<Composition> composition);

    /**
     * @brief コンポジションを取得
     * @param id コンポジションID
     * @return コンポジションオブジェクト（未登録の場合はnullptr）
     */
    std::shared_ptr<Composition> getComposition(const std::string& id);

    /**
     * @brief 依存関係を追加
     * @param parentId 親コンポジションID
     * @param childId 子コンポジションID
     * @return 追加成功フラグ（循環参照の場合はfalse）
     */
    bool addDependency(const std::string& parentId, const std::string& childId);

    /**
     * @brief 循環参照チェック
     * @param startId 開始コンポジションID
     * @param targetId 対象コンポジションID
     * @return 循環参照があればtrue
     */
    bool hasCircularDependency(const std::string& startId, const std::string& targetId);

    /**
     * @brief 依存関係の取得
     * @param compositionId コンポジションID
     * @return 依存しているコンポジションIDのセット
     */
    std::unordered_set<std::string> getDependencies(const std::string& compositionId);

    /**
     * @brief 逆依存関係の取得（このコンポジションに依存しているコンポジション）
     * @param compositionId コンポジションID
     * @return このコンポジションに依存しているコンポジションIDのセット
     */
    std::unordered_set<std::string> getReverseDependencies(const std::string& compositionId);

    /**
     * @brief 依存関係をクリア
     * @param compositionId クリア対象のコンポジションID
     */
    void clearDependencies(const std::string& compositionId);

    /**
     * @brief コンポジションの登録解除
     * @param id コンポジションID
     */
    void unregisterComposition(const std::string& id);

    /**
     * @brief 全ての依存関係をクリア
     */
    void clearAll();

    /**
     * @brief デバッグ用：依存関係グラフを出力
     */
    void printDependencyGraph();

    /**
     * @brief 安全な読み込み順序を計算（トポロジカルソート）
     * @return 安全な読み込み順序のコンポジションIDリスト
     */
    std::vector<std::string> calculateSafeLoadOrder();

private:
    CompositionManager() = default;
    ~CompositionManager() = default;
    CompositionManager(const CompositionManager&) = delete;
    CompositionManager& operator=(const CompositionManager&) = delete;

    // 登録されたコンポジション
    std::unordered_map<std::string, std::shared_ptr<Composition>> compositions_;

    // 依存関係グラフ（親 -> 子のセット）
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencies_;

    // 逆依存関係グラフ（子 -> 親のセット）
    std::unordered_map<std::string, std::unordered_set<std::string>> reverse_dependencies_;

    /**
     * @brief DFS（深さ優先探索）による循環参照検出
     * @param currentId 現在のコンポジションID
     * @param targetId 検索対象ID
     * @param visited 訪問済みIDセット
     * @param path 現在のパス
     * @return 循環参照があればtrue
     */
    bool dfsCircularCheck(const std::string& currentId, 
                         const std::string& targetId,
                         std::unordered_set<std::string>& visited,
                         std::unordered_set<std::string>& path);

    /**
     * @brief トポロジカルソート用のDFS
     * @param id コンポジションID
     * @param visited 訪問済みフラグ
     * @param stack 結果スタック
     */
    void topologicalSortUtil(const std::string& id,
                           std::unordered_set<std::string>& visited,
                           std::vector<std::string>& stack);
};

}} // namespace ofx::ae