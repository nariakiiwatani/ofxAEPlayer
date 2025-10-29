#include <sstream>

#include "ofLog.h"
#include "ofUtils.h"

#include "ofxAEComposition.h"

#include "ofxAEAssetManager.h"

namespace ofx { namespace ae {

AssetManager& AssetManager::getInstance()
{
    static AssetManager instance;
    return instance;
}

std::shared_ptr<ofTexture> AssetManager::getTexture(const std::filesystem::path& path)
{
    AssetKey key(path, AssetKey::AssetType::TEXTURE);

    auto loader = [this](const std::filesystem::path& p) {
        return createTexture(p);
    };

    return texture_cache_.get(key, loader);
}

std::shared_ptr<ofVideoPlayer> AssetManager::getVideo(const std::filesystem::path& path)
{
    AssetKey key(path, AssetKey::AssetType::VIDEO);

    auto loader = [this](const std::filesystem::path& p) {
        return createVideo(p);
    };

    return video_cache_.get(key, loader);
}

std::shared_ptr<Composition> AssetManager::getComposition(const std::filesystem::path& path)
{
    AssetKey key(path, AssetKey::AssetType::COMPOSITION);

    auto loader = [this](const std::filesystem::path& p) {
        return createComposition(p);
    };

    return composition_cache_.get(key, loader);
}

void AssetManager::cleanup()
{
    texture_cache_.cleanup();
    video_cache_.cleanup();
    composition_cache_.cleanup();
}

void AssetManager::clearTextureCache()
{
    texture_cache_.clear();
}

void AssetManager::clearVideoCache()
{
    video_cache_.clear();
}

void AssetManager::clearCompositionCache()
{
    composition_cache_.clear();
}

void AssetManager::clearAllCaches()
{
    clearTextureCache();
    clearVideoCache();
    clearCompositionCache();
}

AssetManager::AssetStats AssetManager::getStats() const
{
    AssetStats stats;
    stats.texture_stats = texture_cache_.getStats();
    stats.video_stats = video_cache_.getStats();
    stats.composition_stats = composition_cache_.getStats();
    return stats;
}

void AssetManager::resetStats()
{
    const_cast<CacheStats&>(texture_cache_.getStats()).reset();
    const_cast<CacheStats&>(video_cache_.getStats()).reset();
    const_cast<CacheStats&>(composition_cache_.getStats()).reset();
}

std::string AssetManager::getDebugInfo() const
{
    std::ostringstream oss;
    auto stats = getStats();

    oss << "AssetManager Debug Info:\n";
    oss << "========================\n";
    oss << "Overall Hit Ratio: " << (stats.getOverallHitRatio() * 100.0) << "%\n\n";

    oss << "Texture Cache:\n";
    oss << "  Items: " << stats.texture_stats.cached_items << "\n";
    oss << "  Hits: " << stats.texture_stats.hits << ", Misses: " << stats.texture_stats.misses << "\n";
    oss << "  Hit Ratio: " << (stats.texture_stats.getHitRatio() * 100.0) << "%\n\n";

    oss << "Video Cache:\n";
    oss << "  Items: " << stats.video_stats.cached_items << "\n";
    oss << "  Hits: " << stats.video_stats.hits << ", Misses: " << stats.video_stats.misses << "\n";
    oss << "  Hit Ratio: " << (stats.video_stats.getHitRatio() * 100.0) << "%\n\n";

    oss << "Composition Cache:\n";
    oss << "  Items: " << stats.composition_stats.cached_items << "\n";
    oss << "  Hits: " << stats.composition_stats.hits << ", Misses: " << stats.composition_stats.misses << "\n";
    oss << "  Hit Ratio: " << (stats.composition_stats.getHitRatio() * 100.0) << "%\n";

    return oss.str();
}

void AssetManager::logCacheStats() const
{
    ofLogNotice("AssetManager") << "\n" << getDebugInfo();
}

std::shared_ptr<ofTexture> AssetManager::createTexture(const std::filesystem::path& path)
{
    auto texture = std::make_shared<ofTexture>();

    if(ofLoadImage(*texture, path)) {
        ofLogVerbose("AssetManager") << "Loaded texture: " << path;
        return texture;
    }
    else {
        ofLogError("AssetManager") << "Failed to load texture: " << path;
        return nullptr;
    }
}

std::shared_ptr<ofVideoPlayer> AssetManager::createVideo(const std::filesystem::path& path)
{
    auto video = std::make_shared<ofVideoPlayer>();

    if(video->load(path)) {
        ofLogVerbose("AssetManager") << "Loaded video: " << path;
        return video;
    }
    else {
        ofLogError("AssetManager") << "Failed to load video: " << path;
        return nullptr;
    }
}

std::shared_ptr<Composition> AssetManager::createComposition(const std::filesystem::path& path)
{
    auto composition = std::make_shared<Composition>();

    if(composition->load(path)) {
        ofLogVerbose("AssetManager") << "Loaded composition: " << path;
        return composition;
    }
    else {
        ofLogError("AssetManager") << "Failed to load composition: " << path;
        return nullptr;
    }
}

}} // namespace ofx::ae
