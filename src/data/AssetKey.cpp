#include <sstream>

#include "ofLog.h"
#include "ofUtils.h"

#include "AssetKey.h"

namespace ofx { namespace ae {

AssetKey::AssetKey(const std::filesystem::path& path, AssetType type, const std::string& params)
	: type_(type)
	, parameters_(params)
{
	canonicalizePath(path);
}

void AssetKey::canonicalizePath(const std::filesystem::path& path)
{
	try {
		if(std::filesystem::exists(path)) {
			canonical_path_ = std::filesystem::canonical(path);
		}
		else {
			canonical_path_ = std::filesystem::absolute(path);
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		ofLogWarning("AssetKey") << "Failed to canonicalize path: " << path << " - " << e.what();
		canonical_path_ = std::filesystem::absolute(path);
	}
}

bool AssetKey::operator<(const AssetKey& other) const
{
	if(canonical_path_ != other.canonical_path_) {
		return canonical_path_ < other.canonical_path_;
	}
	if(type_ != other.type_) {
		return type_ < other.type_;
	}
	return parameters_ < other.parameters_;
}

bool AssetKey::operator==(const AssetKey& other) const
{
	return canonical_path_ == other.canonical_path_ &&
		   type_ == other.type_ &&
		   parameters_ == other.parameters_;
}

bool AssetKey::operator!=(const AssetKey& other) const
{
	return !(*this == other);
}

std::string AssetKey::toString() const
{
	std::ostringstream oss;
	oss << assetTypeToString(type_) << ":" << canonical_path_.string();
	if(!parameters_.empty()) {
		oss << "?" << parameters_;
	}
	return oss.str();
}

std::size_t AssetKey::hash() const
{
	std::size_t h1 = std::hash<std::string>{}(canonical_path_.string());
	std::size_t h2 = std::hash<int>{}(static_cast<int>(type_));
	std::size_t h3 = std::hash<std::string>{}(parameters_);
	
	return h1 ^ (h2 << 1) ^ (h3 << 2);
}

std::string AssetKey::assetTypeToString(AssetType type)
{
	switch (type) {
		case AssetType::TEXTURE: return "texture";
		case AssetType::VIDEO: return "video";
		case AssetType::COMPOSITION: return "composition";
		case AssetType::UNKNOWN:
		default: return "unknown";
	}
}

AssetKey::AssetType AssetKey::stringToAssetType(const std::string& typeStr)
{
	if(typeStr == "texture") return AssetType::TEXTURE;
	if(typeStr == "video") return AssetType::VIDEO;
	if(typeStr == "composition") return AssetType::COMPOSITION;
	return AssetType::UNKNOWN;
}

}} // namespace ofx::ae
