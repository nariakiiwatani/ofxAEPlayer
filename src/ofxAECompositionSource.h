#pragma once

#include "ofxAELayerSource.h"
#include "ofxAEComposition.h"
#include "ofMain.h"
#include <set>
#include <sstream>

namespace ofx { namespace ae {\

class CompositionSource : public LayerSource {
public:
    CompositionSource();
    ~CompositionSource() override;
    
    // LayerSource interface implementation
	bool load(const std::filesystem::path &filepath) override;
    void update(float currentTime) override;
    void draw(const RenderContext& context) const override;
    
    SourceType getSourceType() const override { return COMPOSITION; }
    float getWidth() const override;
    float getHeight() const override;
    ofRectangle getBounds() const override;
    
    bool canCache() const override { return true; }
    std::string getDebugInfo() const override;
    
    // Composition固有メソッド（既存CompositionLayerから移植）
    void setComposition(std::shared_ptr<Composition> composition);
    std::shared_ptr<Composition> getComposition() const { return composition_; }
    
    // レンダリング制御
    void renderComposition() const;
    void setTimeOffset(float offset) { timeOffset_ = offset; }
    float getTimeOffset() const { return timeOffset_; }
    
    // フレームバッファ管理
    void setupFrameBuffer();
    void updateFrameBuffer() const;
    bool isFrameBufferDirty() const { return frameBufferDirty_; }
    void clearFrameBuffer();

private:
    // 既存CompositionLayerのメンバーを移植
    std::shared_ptr<Composition> composition_;
    float timeOffset_;
    
    // フレームバッファキャッシング（CompositionLayer::render_fbo_から移植）
    mutable ofFbo frameBuffer_;
    mutable bool frameBufferDirty_;
    mutable bool frameBufferInitialized_;
    mutable float lastRenderTime_;
    
    // パフォーマンス最適化
    mutable ofRectangle cachedBounds_;
    mutable bool boundsNeedUpdate_;
    
    // 既存CompositionLayerのヘルパーメソッドを移植
    bool parseCompositionProperties(const ofJson& data);
    void invalidateFrameBuffer() const;
    void ensureFrameBufferSize(int width, int height) const;
    
    // フレームバッファ管理ヘルパー
    void initializeFrameBuffer() const;
    bool shouldUpdateFrameBuffer(float currentTime) const;
    void markFrameBufferDirty() const;
};

}}
