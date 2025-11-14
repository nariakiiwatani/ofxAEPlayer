#include "ofxAESequenceSource.h"
#include "ofxAEVisitor.h"
#include "ofLog.h"
#include "ofJson.h"
#include "../utils/ofxAEAssetManager.h"
#include "../utils/ofxAETimeUtils.h"
#include <algorithm>

namespace ofx { namespace ae {

bool SequenceSource::load(const std::filesystem::path &filepath)
{
	pool_.clear();
	texture_.reset();
	fps_ = 30.0;
	
	if(filepath.extension() == ".json") {
		ofJson json = ofLoadJson(filepath);
		try {
			if(json.contains("fps")) {
				fps_ = json["fps"].get<double>();
			}
			
			if(!json.contains("directory")) {
				ofLogError("SequenceSource") << "JSON metadata missing 'directory' field: " << filepath;
				return false;
			}
			
			std::string relativeDir = json["directory"].get<std::string>();
			
			std::filesystem::path jsonParent = filepath.parent_path();
			std::filesystem::path sequenceDir = jsonParent / relativeDir;

			if(json.contains("frames") &&
			   json["frames"].contains("list") &&
			   json["frames"].contains("indices")) {

				auto fileList = json["frames"]["list"].get<std::vector<std::string>>();
				auto indices = json["frames"]["indices"].get<std::vector<int>>();

				pool_.reserve(indices.size());
				for(const auto& index : indices) {
					std::filesystem::path fullPath = sequenceDir / fileList[index];
					pool_.push_back(AssetManager::getInstance().getTexture(fullPath));
				}

				return !pool_.empty();
			}
			return loadImagesFromDirectory(sequenceDir);
			
		} catch(const std::exception &e) {
			ofLogError("SequenceSource") << "Error parsing JSON metadata: " << e.what();
			return false;
		}
	}
	else {
		return loadImagesFromDirectory(filepath);
	}
}

bool SequenceSource::loadImagesFromDirectory(const std::filesystem::path &dirpath)
{
	if(!ofDirectory::doesDirectoryExist(dirpath)) {
		ofLogError("SequenceSource") << "Directory does not exist: " << dirpath;
		return false;
	}
	
	ofDirectory dir;
	dir.open(dirpath);
	dir.sort();
	pool_.reserve(dir.size());
	for(auto &&file : dir.getFiles()) {
		auto tex = AssetManager::getInstance().getTexture(file.path());
		pool_.push_back(tex);
	}

	if(pool_.empty()) {
		ofLogWarning("SequenceSource") << "No images loaded from directory: " << dirpath;
	}
	
	return !pool_.empty();
}

bool SequenceSource::setTime(double time)
{
	if(util::isNearTime(current_time_, time)) {
		return false;
	}
	
	int new_index = static_cast<int>(time * fps_);
	new_index = std::clamp(new_index, 0, static_cast<int>(pool_.size()) - 1);
	
	bool changed = (new_index != current_index_);
	current_index_ = new_index;
	current_time_ = time;
	
	if(new_index >= 0 && static_cast<size_t>(new_index) < pool_.size()) {
		texture_ = pool_[new_index];
	}
	else {
		texture_.reset();
	}
	
	return changed;
}

void SequenceSource::draw(float x, float y, float w, float h) const
{
	if(auto tex = texture_.lock()) {
		tex->draw(x,y,w,h);
	}
}

double SequenceSource::getDuration() const
{
	if(pool_.empty() || fps_ <= 0.0) {
		return 0.0;
	}
	return static_cast<double>(pool_.size()) / fps_;
}

void SequenceSource::accept(Visitor &visitor) {
	visitor.visit(*this);
}

}} // namespace ofx::ae
