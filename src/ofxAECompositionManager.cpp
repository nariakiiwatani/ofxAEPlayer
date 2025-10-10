#include "ofxAECompositionManager.h"
#include "ofLog.h"
#include <algorithm>

namespace ofx { namespace ae {

CompositionManager& CompositionManager::getInstance() {
    static CompositionManager instance;
    return instance;
}

bool CompositionManager::registerComposition(const std::string& id, std::shared_ptr<Composition> composition) {
    if (!composition) {
        ofLogError("CompositionManager") << "Cannot register null composition: " << id;
        return false;
    }
    
    compositions_[id] = composition;
    ofLogVerbose("CompositionManager") << "Registered composition: " << id;
    return true;
}

std::shared_ptr<Composition> CompositionManager::getComposition(const std::string& id) {
    auto it = compositions_.find(id);
    if (it != compositions_.end()) {
        return it->second;
    }
    return nullptr;
}

bool CompositionManager::addDependency(const std::string& parentId, const std::string& childId) {
    // 循環参照チェック
    if (hasCircularDependency(childId, parentId)) {
        ofLogError("CompositionManager") << "Circular dependency detected: " 
                                         << parentId << " -> " << childId;
        return false;
    }
    
    // 依存関係を追加
    dependencies_[parentId].insert(childId);
    reverse_dependencies_[childId].insert(parentId);
    
    ofLogVerbose("CompositionManager") << "Added dependency: " << parentId << " -> " << childId;
    return true;
}

bool CompositionManager::hasCircularDependency(const std::string& startId, const std::string& targetId) {
    if (startId == targetId) {
        return true;
    }
    
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> path;
    
    return dfsCircularCheck(startId, targetId, visited, path);
}

bool CompositionManager::dfsCircularCheck(const std::string& currentId, 
                                          const std::string& targetId,
                                          std::unordered_set<std::string>& visited,
                                          std::unordered_set<std::string>& path) {
    // 現在のIDをパスに追加
    path.insert(currentId);
    
    // 子の依存関係を探索
    auto it = dependencies_.find(currentId);
    if (it != dependencies_.end()) {
        for (const auto& childId : it->second) {
            // 目標に到達した場合
            if (childId == targetId) {
                return true;
            }
            
            // パス内に既に存在する場合（循環参照）
            if (path.find(childId) != path.end()) {
                return true;
            }
            
            // 未訪問の場合は再帰的に探索
            if (visited.find(childId) == visited.end()) {
                visited.insert(childId);
                if (dfsCircularCheck(childId, targetId, visited, path)) {
                    return true;
                }
            }
        }
    }
    
    // パスから削除（バックトラック）
    path.erase(currentId);
    return false;
}

std::unordered_set<std::string> CompositionManager::getDependencies(const std::string& compositionId) {
    auto it = dependencies_.find(compositionId);
    if (it != dependencies_.end()) {
        return it->second;
    }
    return std::unordered_set<std::string>();
}

std::unordered_set<std::string> CompositionManager::getReverseDependencies(const std::string& compositionId) {
    auto it = reverse_dependencies_.find(compositionId);
    if (it != reverse_dependencies_.end()) {
        return it->second;
    }
    return std::unordered_set<std::string>();
}

void CompositionManager::clearDependencies(const std::string& compositionId) {
    // 前方依存関係をクリア
    auto deps = getDependencies(compositionId);
    for (const auto& childId : deps) {
        reverse_dependencies_[childId].erase(compositionId);
    }
    dependencies_.erase(compositionId);
    
    // 後方依存関係をクリア
    auto reverseDeps = getReverseDependencies(compositionId);
    for (const auto& parentId : reverseDeps) {
        dependencies_[parentId].erase(compositionId);
    }
    reverse_dependencies_.erase(compositionId);
    
    ofLogVerbose("CompositionManager") << "Cleared dependencies for: " << compositionId;
}

void CompositionManager::unregisterComposition(const std::string& id) {
    // 依存関係をクリア
    clearDependencies(id);
    
    // コンポジションを削除
    compositions_.erase(id);
    
    ofLogVerbose("CompositionManager") << "Unregistered composition: " << id;
}

void CompositionManager::clearAll() {
    compositions_.clear();
    dependencies_.clear();
    reverse_dependencies_.clear();
    
    ofLogVerbose("CompositionManager") << "Cleared all compositions and dependencies";
}

void CompositionManager::printDependencyGraph() {
    ofLogNotice("CompositionManager") << "=== Dependency Graph ===";
    
    for (const auto& [parentId, children] : dependencies_) {
        if (!children.empty()) {
            ofLogNotice("CompositionManager") << parentId << " depends on:";
            for (const auto& childId : children) {
                ofLogNotice("CompositionManager") << "  -> " << childId;
            }
        }
    }
    
    ofLogNotice("CompositionManager") << "=== End Dependency Graph ===";
}

std::vector<std::string> CompositionManager::calculateSafeLoadOrder() {
    std::unordered_set<std::string> visited;
    std::vector<std::string> stack;
    
    // 全てのコンポジションに対してトポロジカルソートを実行
    for (const auto& [id, comp] : compositions_) {
        if (visited.find(id) == visited.end()) {
            topologicalSortUtil(id, visited, stack);
        }
    }
    
    // スタックを逆順にして正しい順序を取得
    std::reverse(stack.begin(), stack.end());
    
    ofLogVerbose("CompositionManager") << "Safe load order calculated with " << stack.size() << " compositions";
    
    return stack;
}

void CompositionManager::topologicalSortUtil(const std::string& id,
                                             std::unordered_set<std::string>& visited,
                                             std::vector<std::string>& stack) {
    visited.insert(id);
    
    // 依存している子コンポジションを先に処理
    auto it = dependencies_.find(id);
    if (it != dependencies_.end()) {
        for (const auto& childId : it->second) {
            if (visited.find(childId) == visited.end()) {
                topologicalSortUtil(childId, visited, stack);
            }
        }
    }
    
    // 自分をスタックに追加
    stack.push_back(id);
}

}} // namespace ofx::ae