#pragma once

#include <filesystem>
#include <string>
#include <functional>

namespace ofx { namespace ae {

/**
 * AssetKey - Unique identifier for cacheable assets
 * Combines file path, asset type, and optional parameters for unique identification
 */
class AssetKey {
public:
    enum class AssetType {
        TEXTURE,
        VIDEO, 
        COMPOSITION,
        UNKNOWN
    };

    AssetKey(const std::filesystem::path& path, AssetType type, const std::string& params = "");
    
    // Comparison operators for use in std::map
    bool operator<(const AssetKey& other) const;
    bool operator==(const AssetKey& other) const;
    bool operator!=(const AssetKey& other) const;
    
    // Getters
    const std::filesystem::path& getPath() const { return canonical_path_; }
    AssetType getType() const { return type_; }
    const std::string& getParameters() const { return parameters_; }
    
    // Utility methods
    std::string toString() const;
    std::size_t hash() const;
    
    // Static helpers
    static std::string assetTypeToString(AssetType type);
    static AssetType stringToAssetType(const std::string& typeStr);

private:
    std::filesystem::path canonical_path_;
    AssetType type_;
    std::string parameters_;
    
    void canonicalizePath(const std::filesystem::path& path);
};

}} // namespace ofx::ae

// Hash specialization for std::unordered_map support
namespace std {
    template<>
    struct hash<ofx::ae::AssetKey> {
        std::size_t operator()(const ofx::ae::AssetKey& key) const {
            return key.hash();
        }
    };
}