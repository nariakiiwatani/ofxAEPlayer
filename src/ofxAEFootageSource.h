#pragma once

#include "ofxAELayerSource.h"
#include "ofxAERenderContext.h"
#include "ofVideoPlayer.h"
#include "ofTexture.h"
#include "ofImage.h"
#include "ofJson.h"
#include <memory>
#include <vector>
#include <sstream>

namespace ofx { namespace ae {

class FootageSource : public LayerSource {
public:
    struct FootageInfo {
        std::string path;
        std::string type;  // "still", "video", "sequence"
        int width, height;
        float fps;
        int frameCount;
        
        FootageInfo() : width(0), height(0), fps(30.0f), frameCount(1) {}
    };
    
    FootageSource();
    ~FootageSource() override;
    
    // LayerSource interface implementation
    bool setup(const ofJson& json) override;
    void update(float currentTime) override;
    void draw(const RenderContext& context) const override;
    
    SourceType getSourceType() const override { return FOOTAGE; }
    float getWidth() const override;
    float getHeight() const override;
    ofRectangle getBounds() const override;
    
    bool canCache() const override;
    std::string getDebugInfo() const override;
    
    // Footage固有メソッド
    bool loadFootage(const std::string& path);
    void setCurrentFrame(int frame);
    ofTexture* getCurrentTexture() const;
    
    const FootageInfo& getFootageInfo() const { return footageInfo_; }
    void setTextureQuality(float quality); // LOD対応
    
private:
    FootageInfo footageInfo_;
    std::unique_ptr<ofVideoPlayer> videoPlayer_;
    ofTexture stillTexture_;
    std::vector<ofTexture> sequenceTextures_;
    int currentFrame_;
    float textureQuality_;
    bool needsUpdate_;
    
    // ヘルパーメソッド
    bool loadStillImage(const std::string& path);
    bool loadVideoFile(const std::string& path);
    bool loadSequence(const std::string& path);
    std::string detectFootageType(const std::string& path);
    std::vector<std::string> findSequenceFiles(const std::string& basePath);
    
    // 内部状態管理
    mutable ofTexture* cachedTexture_;
    float lastUpdateTime_;
    bool isLoaded_;
};

}}