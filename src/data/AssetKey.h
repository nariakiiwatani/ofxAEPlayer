#pragma once

#include <filesystem>
#include <string>
#include <functional>

namespace ofx { namespace ae {

class AssetKey {
public:
    enum class AssetType {
        TEXTURE,
        VIDEO, 
        COMPOSITION,
        UNKNOWN
    };

    AssetKey(const std::filesystem::path& path, AssetType type, const std::string& params = "");
    
    bool operator<(const AssetKey& other) const;
    bool operator==(const AssetKey& other) const;
    bool operator!=(const AssetKey& other) const;
    
    const std::filesystem::path& getPath() const { return canonical_path_; }
    AssetType getType() const { return type_; }
    const std::string& getParameters() const { return parameters_; }
    
    std::string toString() const;
    std::size_t hash() const;

	static std::string assetTypeToString(AssetType type);
    static AssetType stringToAssetType(const std::string& typeStr);

private:
    std::filesystem::path canonical_path_;
    AssetType type_;
    std::string parameters_;
    
    void canonicalizePath(const std::filesystem::path& path);
};

}} // namespace ofx::ae

namespace std {
    template<>
    struct hash<ofx::ae::AssetKey> {
        std::size_t operator()(const ofx::ae::AssetKey& key) const {
            return key.hash();
        }
    };
}
