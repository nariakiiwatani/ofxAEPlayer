#pragma once

#include "../data/AssetKey.h"
#include "ofMain.h"
#include <map>
#include <memory>

namespace ofx { namespace ae {

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

template<typename Type>
class AssetCache {
public:
	using LoadFunction = std::function<std::shared_ptr<Type>(const std::filesystem::path&)>;

	std::shared_ptr<Type> get(const AssetKey &key, LoadFunction loader = nullptr) {
		auto it = cache_.find(key);
		if(it != cache_.end()) {
			auto asset = it->second.lock();
			if(asset) {
				stats_.hits++;
				return asset;
			}
			else {
				cache_.erase(it);
				stats_.cached_items--;
			}
		}
		
		stats_.misses++;
		
		if(loader) {
			auto asset = loader(key.getPath());
			if(asset) {
				store(key, asset);
				return asset;
			}
		}
		
		return nullptr;
	}

	void store(const AssetKey &key, std::shared_ptr<Type> asset) {
		if(!asset) return;
		
		cache_[key] = asset;
		stats_.cached_items++;
	}

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
	
	void clear() {
		cache_.clear();
		stats_.cached_items = 0;
	}
	
	const CacheStats& getStats() const {
		return stats_;
	}

private:
	std::map<AssetKey, std::weak_ptr<Type>> cache_;
	CacheStats stats_;
};

}} // namespace ofx::ae
