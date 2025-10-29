#pragma once

#include "ofxAEAssetCache.h"
#include "../data/AssetKey.h"
#include "ofMain.h"
#include <memory>
#include <functional>

namespace ofx { namespace ae {
	class Composition;
}}

namespace ofx { namespace ae {

class AssetManager
{
public:
	static AssetManager& getInstance();
	
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	
	std::shared_ptr<ofTexture> getTexture(const std::filesystem::path &path);
	std::shared_ptr<ofVideoPlayer> getVideo(const std::filesystem::path &path);
	std::shared_ptr<Composition> getComposition(const std::filesystem::path &path);

	void cleanup();
	void clearTextureCache();
	void clearVideoCache();
	void clearCompositionCache();
	void clearAllCaches();
	
	struct AssetStats {
		CacheStats texture_stats;
		CacheStats video_stats;
		CacheStats composition_stats;
		
		double getOverallHitRatio() const {
			size_t total_hits = texture_stats.hits + video_stats.hits + composition_stats.hits;
			size_t total_requests = total_hits + texture_stats.misses + video_stats.misses + composition_stats.misses;
			return total_requests > 0 ? static_cast<double>(total_hits) / total_requests : 0.0;
		}
	};
	
	AssetStats getStats() const;
	void resetStats();

	std::string getDebugInfo() const;
	void logCacheStats() const;
	
private:
	AssetManager() = default;
	
	AssetCache<ofTexture> texture_cache_;
	AssetCache<ofVideoPlayer> video_cache_;
	AssetCache<Composition> composition_cache_;
	
	std::shared_ptr<ofTexture> createTexture(const std::filesystem::path& path);
	std::shared_ptr<ofVideoPlayer> createVideo(const std::filesystem::path& path);
	std::shared_ptr<Composition> createComposition(const std::filesystem::path& path);
};

}} // namespace ofx::ae
