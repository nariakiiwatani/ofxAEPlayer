#include "ofxAEFootageSource.h"
#include "ofLog.h"
#include "ofUtils.h"
#include "ofFileUtils.h"
// #include "ofDirectory.h"  // Not available in this OF version
#include <algorithm>
#include <regex>
#include <sstream>

namespace ofx { namespace ae {

FootageSource::FootageSource() 
    : currentFrame_(0)
    , textureQuality_(1.0f)
    , needsUpdate_(true)
    , cachedTexture_(nullptr)
    , lastUpdateTime_(-1.0f)
    , isLoaded_(false)
{
}

FootageSource::~FootageSource() {
    if (videoPlayer_) {
        videoPlayer_->close();
    }
}

bool FootageSource::setup(const ofJson& json) {
    try {
        if (!json.contains("source")) {
            ofLogError("FootageSource") << "No source path specified in JSON";
            return false;
        }
        
        std::string sourcePath = json["source"];
        
        // Optional parameters
        if (json.contains("fps")) {
            footageInfo_.fps = json["fps"];
        }
        
        if (json.contains("frameCount")) {
            footageInfo_.frameCount = json["frameCount"];
        }
        
        // Load the footage
        if (!loadFootage(sourcePath)) {
            ofLogError("FootageSource") << "Failed to load footage: " << sourcePath;
            return false;
        }
        
        isLoaded_ = true;
        return true;
        
    } catch (const std::exception& e) {
        ofLogError("FootageSource") << "Exception in setup: " << e.what();
        return false;
    }
}

void FootageSource::update(float currentTime) {
    if (!isLoaded_ || !needsUpdate_) {
        return;
    }
    
    if (footageInfo_.type == "video" && videoPlayer_) {
        // Update video player
        videoPlayer_->update();
        
        // Calculate frame based on time and fps
        int targetFrame = static_cast<int>(currentTime * footageInfo_.fps);
        targetFrame = ofClamp(targetFrame, 0, footageInfo_.frameCount - 1);
        
        if (targetFrame != currentFrame_) {
            setCurrentFrame(targetFrame);
        }
        
        // Update cached texture pointer
        if (videoPlayer_->isLoaded() && videoPlayer_->getTexture().isAllocated()) {
            cachedTexture_ = const_cast<ofTexture*>(&videoPlayer_->getTexture());
        }
        
    } else if (footageInfo_.type == "sequence" && !sequenceTextures_.empty()) {
        // Calculate frame for sequence
        int targetFrame = static_cast<int>(currentTime * footageInfo_.fps);
        targetFrame = ofClamp(targetFrame, 0, static_cast<int>(sequenceTextures_.size()) - 1);
        
        if (targetFrame != currentFrame_) {
            setCurrentFrame(targetFrame);
        }
        
        // Update cached texture pointer
        if (currentFrame_ < sequenceTextures_.size() && sequenceTextures_[currentFrame_].isAllocated()) {
            cachedTexture_ = &sequenceTextures_[currentFrame_];
        }
        
    } else if (footageInfo_.type == "still" && stillTexture_.isAllocated()) {
        // Still image - just ensure texture is available
        cachedTexture_ = &stillTexture_;
    }
    
    lastUpdateTime_ = currentTime;
    needsUpdate_ = false;
}

void FootageSource::draw(const RenderContext& context) const {
    if (!isLoaded_ || !cachedTexture_ || !cachedTexture_->isAllocated()) {
        return;
    }
    
    // Apply context transformations and draw texture
    ofPushMatrix();
    
    // Apply any transformations from render context
    ofMultMatrix(context.transform);
    
    // Apply opacity
    ofPushStyle();
    ofSetColor(255, 255, 255, 255 * context.opacity);
    
    cachedTexture_->draw(context.x, context.y, context.w, context.h);
    
    ofPopStyle();
    ofPopMatrix();
}

float FootageSource::getWidth() const {
    return static_cast<float>(footageInfo_.width);
}

float FootageSource::getHeight() const {
    return static_cast<float>(footageInfo_.height);
}

ofRectangle FootageSource::getBounds() const {
    return ofRectangle(0, 0, getWidth(), getHeight());
}

bool FootageSource::canCache() const {
    // Still images can be cached, videos and sequences typically shouldn't be
    return footageInfo_.type == "still";
}

std::string FootageSource::getDebugInfo() const {
    std::stringstream ss;
    ss << "FootageSource[" << footageInfo_.type << "] ";
    ss << "Path: " << footageInfo_.path << " ";
    ss << "Size: " << footageInfo_.width << "x" << footageInfo_.height << " ";
    ss << "FPS: " << footageInfo_.fps << " ";
    ss << "Frames: " << footageInfo_.frameCount << " ";
    ss << "Current: " << currentFrame_ << " ";
    ss << "Loaded: " << (isLoaded_ ? "Yes" : "No");
    return ss.str();
}

bool FootageSource::loadFootage(const std::string& path) {
    footageInfo_.path = path;
    footageInfo_.type = detectFootageType(path);
    
    if (footageInfo_.type == "still") {
        return loadStillImage(path);
    } else if (footageInfo_.type == "video") {
        return loadVideoFile(path);
    } else if (footageInfo_.type == "sequence") {
        return loadSequence(path);
    }
    
    ofLogError("FootageSource") << "Unknown footage type for: " << path;
    return false;
}

void FootageSource::setCurrentFrame(int frame) {
    if (frame == currentFrame_) {
        return;
    }
    
    currentFrame_ = ofClamp(frame, 0, footageInfo_.frameCount - 1);
    
    if (footageInfo_.type == "video" && videoPlayer_) {
        // Set video frame
        float normalizedFrame = static_cast<float>(currentFrame_) / static_cast<float>(footageInfo_.frameCount - 1);
        videoPlayer_->setPosition(normalizedFrame);
    }
    
    needsUpdate_ = true;
}

ofTexture* FootageSource::getCurrentTexture() const {
    return cachedTexture_;
}

void FootageSource::setTextureQuality(float quality) {
    textureQuality_ = ofClamp(quality, 0.1f, 1.0f);
    // LOD implementation would go here
    // For now, just store the quality setting
}

bool FootageSource::loadStillImage(const std::string& path) {
    if (!ofFile::doesFileExist(path)) {
        ofLogError("FootageSource") << "Still image file not found: " << path;
        return false;
    }
    
    ofImage image;
    if (!image.load(path)) {
        ofLogError("FootageSource") << "Failed to load still image: " << path;
        return false;
    }
    
    stillTexture_ = image.getTexture();
    footageInfo_.width = image.getWidth();
    footageInfo_.height = image.getHeight();
    footageInfo_.frameCount = 1;
    
    cachedTexture_ = &stillTexture_;
    
    ofLogNotice("FootageSource") << "Loaded still image: " << path 
                                << " (" << footageInfo_.width << "x" << footageInfo_.height << ")";
    return true;
}

bool FootageSource::loadVideoFile(const std::string& path) {
    if (!ofFile::doesFileExist(path)) {
        ofLogError("FootageSource") << "Video file not found: " << path;
        return false;
    }
    
    videoPlayer_ = std::make_unique<ofVideoPlayer>();
    
    if (!videoPlayer_->load(path)) {
        ofLogError("FootageSource") << "Failed to load video: " << path;
        videoPlayer_.reset();
        return false;
    }
    
    footageInfo_.width = videoPlayer_->getWidth();
    footageInfo_.height = videoPlayer_->getHeight();
    footageInfo_.frameCount = videoPlayer_->getTotalNumFrames();
    
    // Use video's native fps if not specified
    if (footageInfo_.fps <= 0) {
        footageInfo_.fps = 30.0f; // Default fallback
    }
    
    videoPlayer_->setLoopState(OF_LOOP_NONE);
    videoPlayer_->setFrame(0);
    
    ofLogNotice("FootageSource") << "Loaded video: " << path 
                                << " (" << footageInfo_.width << "x" << footageInfo_.height << ")"
                                << " Frames: " << footageInfo_.frameCount;
    return true;
}

bool FootageSource::loadSequence(const std::string& path) {
    std::vector<std::string> sequenceFiles = findSequenceFiles(path);
    
    if (sequenceFiles.empty()) {
        ofLogError("FootageSource") << "No sequence files found for: " << path;
        return false;
    }
    
    // Load first few frames to get dimensions and test loading
    ofImage firstImage;
    if (!firstImage.load(sequenceFiles[0])) {
        ofLogError("FootageSource") << "Failed to load first sequence frame: " << sequenceFiles[0];
        return false;
    }
    
    footageInfo_.width = firstImage.getWidth();
    footageInfo_.height = firstImage.getHeight();
    footageInfo_.frameCount = static_cast<int>(sequenceFiles.size());
    
    // Pre-allocate texture vector
    sequenceTextures_.resize(sequenceFiles.size());
    
    // Load first frame immediately
    sequenceTextures_[0] = firstImage.getTexture();
    
    // For performance, we could implement lazy loading here
    // For now, load all frames (memory intensive but simpler)
    for (size_t i = 1; i < sequenceFiles.size(); ++i) {
        ofImage image;
        if (image.load(sequenceFiles[i])) {
            sequenceTextures_[i] = image.getTexture();
        } else {
            ofLogWarning("FootageSource") << "Failed to load sequence frame: " << sequenceFiles[i];
        }
    }
    
    currentFrame_ = 0;
    if (!sequenceTextures_.empty()) {
        cachedTexture_ = &sequenceTextures_[0];
    }
    
    ofLogNotice("FootageSource") << "Loaded sequence: " << path 
                                << " (" << footageInfo_.width << "x" << footageInfo_.height << ")"
                                << " Frames: " << footageInfo_.frameCount;
    return true;
}

std::string FootageSource::detectFootageType(const std::string& path) {
    std::string extension = ofToLower(ofFilePath::getFileExt(path));
    
    // Check for sequence pattern (contains ### or similar)
    if (path.find("###") != std::string::npos || 
        path.find("%") != std::string::npos ||
        path.find("sequence") != std::string::npos) {
        return "sequence";
    }
    
    // Video extensions
    if (extension == "mp4" || extension == "mov" || extension == "avi" || 
        extension == "mkv" || extension == "webm" || extension == "m4v") {
        return "video";
    }
    
    // Still image extensions
    if (extension == "jpg" || extension == "jpeg" || extension == "png" || 
        extension == "tiff" || extension == "tif" || extension == "bmp" || 
        extension == "gif") {
        return "still";
    }
    
    // Default to still image
    return "still";
}

std::vector<std::string> FootageSource::findSequenceFiles(const std::string& basePath) {
    std::vector<std::string> files;
    
    // Parse sequence pattern
    std::string directory = ofFilePath::getEnclosingDirectory(basePath);
    std::string filename = ofFilePath::getFileName(basePath);
    std::string extension = ofFilePath::getFileExt(basePath);
    
    // Handle ### pattern
    if (filename.find("###") != std::string::npos) {
        std::string pattern = filename;
        ofStringReplace(pattern, "###", "(\\d{3})");
        
        ofDirectory dir(directory);
        dir.listDir();
        
        std::regex regexPattern(pattern);
        std::smatch match;
        
        for (int i = 0; i < dir.size(); ++i) {
            std::string fname = dir.getName(i);
            if (std::regex_search(fname, match, regexPattern)) {
                files.push_back(dir.getPath(i));
            }
        }
        
        // Sort files to ensure correct order
        std::sort(files.begin(), files.end());
    }
    
    return files;
}

}}