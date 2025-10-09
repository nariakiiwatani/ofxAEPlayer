#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"
#include "ofxAELayer.h"

namespace ofx {
namespace ae {

// After Effects compatible blend modes
enum class BlendMode {
    NORMAL,
    MULTIPLY,
    SCREEN,
    OVERLAY,
    ADD,
    SUBTRACT,
    DARKEN,
    LIGHTEN,
    COLOR_DODGE,
    COLOR_BURN
};

// Rendering layer types for dispatch
enum class RenderLayerType {
    FOOTAGE,     // AV_LAYER - images/video
    SHAPE,       // SHAPE_LAYER - vector shapes
    TEXT,        // Text layers
    SOLID,       // Solid color layers
    NULL_LAYER   // Control layers (no visual output)
};

// Render target wrapper for efficient composition
class RenderTarget {
public:
    RenderTarget();
    ~RenderTarget();
    
    void allocate(int width, int height);
    void begin();
    void end();
    void clear();
    void clear(ofColor color);
    
    ofFbo& getFbo() { return fbo; }
    ofTexture& getTexture() { return fbo.getTexture(); }
    
    int getWidth() const { return fbo.getWidth(); }
    int getHeight() const { return fbo.getHeight(); }
    bool isAllocated() const { return fbo.isAllocated(); }
    
private:
    ofFbo fbo;
};

// Main rendering engine
class RenderEngine {
public:
    RenderEngine();
    ~RenderEngine();
    
    // Main rendering methods
    void render(const Composition& comp, float currentTime);
    void renderLayer(const Layer& layer, float currentTime);
    
    // Configuration
    void setSize(int width, int height);
    void setQuality(float quality = 1.0f); // LOD system
    void enableCulling(bool enable) { cullingEnabled = enable; }
    void enableCache(bool enable) { cacheEnabled = enable; }
    
    // Render target access
    RenderTarget& getMainTarget() { return mainTarget; }
    RenderTarget& getTempTarget() { return tempTarget; }
    
    // Texture management for footage
    void loadFootage(const std::string& path, const std::string& id);
    ofTexture* getFootageTexture(const std::string& id);
    
    // Performance metrics
    struct PerformanceMetrics {
        int layersRendered;
        int layersCulled;
        int layersCached;
        float totalRenderTime;
        float averageFPS;
        
        PerformanceMetrics() : layersRendered(0), layersCulled(0), layersCached(0), 
                              totalRenderTime(0.0f), averageFPS(0.0f) {}
    };
    
    const PerformanceMetrics& getPerformanceMetrics() const { return currentMetrics; }
    
private:
    // Core rendering pipeline
    void setupRenderState();
    void restoreRenderState();
    void applyLayerTransform(const Layer& layer, float currentTime);
    
    // Layer type specific rendering
    void renderFootage(const Layer& layer, float currentTime);
    void renderShape(const Layer& layer, float currentTime);
    void renderText(const Layer& layer, float currentTime);
    void renderSolid(const Layer& layer, float currentTime);
    void renderNull(const Layer& layer, float currentTime);
    
    // Blend mode implementation
    void applyBlendMode(BlendMode mode);
    void setupBlendModeShader(BlendMode mode);
    
    // Mask system (foundation)
    void renderWithMask(const Layer& layer, float currentTime);
    void applyLayerMasks(const Layer& layer, float currentTime);
    
    // Optimization systems
    bool shouldCullLayer(const Layer& layer, float currentTime);
    void updateLayerCache(const Layer& layer, float currentTime);
    bool isLayerCached(const Layer& layer, float currentTime);
    void drawCachedLayer(const Layer& layer);
    
    // Utility methods
    RenderLayerType getLayerRenderType(const Layer& layer);
    float calculateLayerOpacity(const Layer& layer, float currentTime);
    ofRectangle getLayerBounds(const Layer& layer, float currentTime);
    
private:
    // Render targets
    RenderTarget mainTarget;
    RenderTarget tempTarget;
    RenderTarget maskTarget;
    
    // Configuration
    int renderWidth;
    int renderHeight;
    float qualityScale;
    bool cullingEnabled;
    bool cacheEnabled;
    
    // Texture management
    std::map<std::string, ofTexture> footageTextures;
    std::map<std::string, float> textureLoadTimes;
    
    // Blend mode shaders
    std::map<BlendMode, ofShader> blendShaders;
    void initializeBlendShaders();
    
    // Performance tracking
    PerformanceMetrics currentMetrics;
    PerformanceMetrics frameMetrics;
    float lastRenderTime;
    int frameCount;
    float averageRenderTime;
    
    // Cache system
    struct LayerCache {
        ofTexture texture;
        float lastUpdateTime;
        bool isDirty;
        ofRectangle bounds;
        
        LayerCache() : lastUpdateTime(0.0f), isDirty(true) {}
    };
    std::map<int, LayerCache> layerCache;
    
    // LOD system
    struct LODLevel {
        float distance;
        float quality;
    };
    std::vector<LODLevel> lodLevels;
    void initializeLODLevels();
    float calculateLODQuality(const Layer& layer, float currentTime);
};

} // namespace ae
} // namespace ofx