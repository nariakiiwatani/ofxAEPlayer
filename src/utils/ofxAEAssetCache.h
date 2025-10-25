#pragma once

#include "ofxAEAssetKey.h"
#include "ofMain.h"
#include <map>
#include <memory>

namespace ofx { namespace ae {

/**
 * CacheStats - Statistics for cache performance monitoring
 */
struct CacheStats {
    size_t hits = 0;
    size_t misses = 0;
    size_t cached_items = 0;
    
    double getHitRatio() const {
        size_t total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total : 0.0;
    }
    
    void reset() {
        hits = 0;
        misses = 0;
        cached_items = 0;
    }
};

/**
 * AssetCache - Simple cache for specific asset types
 * Single-threaded implementation using weak_ptr for automatic cleanup
 */
template<typename Type>
class AssetCache {
public:
    using LoadFunction = std::function<std::shared_ptr<Type>(const std::filesystem::path&)>;
    
    /**
     * Get asset from cache or load if not present
     */
    std::shared_ptr<Type> get(const AssetKey& key, LoadFunction loader = nullptr) {
        auto it = cache_.find(key);
        if(it != cache_.end()) {
            auto asset = it->second.lock();
            if(asset) {
                stats_.hits++;
                return asset;
            }
            else {
                // Asset was garbage collected, remove from cache
                cache_.erase(it);
                stats_.cached_items--;
            }
        }
        
        stats_.misses++;
        
        // Load new asset if loader provided
        if(loader) {
            auto asset = loader(key.getPath());
            if(asset) {
                store(key, asset);
                return asset;
            }
        }
        
        return nullptr;
    }
    
    /**
     * Store asset in cache
     */
    void store(const AssetKey& key, std::shared_ptr<Type> asset) {
        if(!asset) return;
        
        cache_[key] = asset;
        stats_.cached_items++;
    }
    
    /**
     * Manual cleanup of expired entries
     */
    void cleanup() {
        auto it = cache_.begin();
        while(it != cache_.end()) {
            if(it->second.expired()) {
                it = cache_.erase(it);
                stats_.cached_items--;
            }
            else {
                ++it;
            }
        }
    }
    
    /**
     * Clear all cached items
     */
    void clear() {
        cache_.clear();
        stats_.cached_items = 0;
    }
    
    /**
     * Get cache statistics
     */
    const CacheStats& getStats() const {
        return stats_;
    }

private:
    std::map<AssetKey, std::weak_ptr<Type>> cache_;
    CacheStats stats_;
};

}} // namespace ofx::ae