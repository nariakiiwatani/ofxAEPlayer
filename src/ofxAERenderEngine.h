#pragma once

#include "ofMain.h"
#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAERenderContext.h"

namespace ofx {
namespace ae {

// Forward declarations for new Source-based system
class LayerSource;

// Performance statistics for the new rendering pipeline
struct RenderStats {
    int layersRendered = 0;
    int layersCulled = 0;
    int sourcesRendered = 0;
    float renderTimeMs = 0.0f;
    size_t memoryUsedMB = 0;
    
    void reset() {
        layersRendered = 0;
        layersCulled = 0;
        sourcesRendered = 0;
        renderTimeMs = 0.0f;
        memoryUsedMB = 0;
    }
};

// BlendMode is now defined in ofxAERenderContext.h to avoid duplication

// Rendering layer types for dispatch
enum class RenderLayerType {
    FOOTAGE,     // AV_LAYER - images/video
    SHAPE,       // SHAPE_LAYER - vector shapes
    TEXT,        // Text layers
    SOLID,       // Solid color layers
    COMPOSITION, // COMPOSITION_LAYER - nested compositions
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
    
    // ========================================================================
    // Source-based rendering pipeline (new functionality)
    // ========================================================================
    
    /**
     * Render a layer using the new Source-based architecture
     * @param layer Layer with attached LayerSource
     * @param context Rendering context with transform, opacity, etc.
     */
    void renderLayer(const Layer& layer, const RenderContext& context);
    
    /**
     * Render an entire composition using Source-based layers
     * @param comp Composition containing Source-based layers
     * @param context Rendering context for the composition
     */
    void renderComposition(const Composition& comp, const RenderContext& context);
    
    // ========================================================================
    // Legacy rendering methods (maintained for backward compatibility)
    // ========================================================================
    
    /// Legacy composition rendering
    void render(const Composition& comp, float currentTime);
    /// Legacy layer rendering
    void renderLayer(const Layer& layer, float currentTime);
    
    // ========================================================================
    // Configuration and optimization
    // ========================================================================
    
    /// Set render target size
    void setSize(int width, int height);
    /// Set quality/LOD level (0.1-1.0)
    void setQuality(float quality = 1.0f);
    /// Enable/disable frustum culling
    void enableCulling(bool enable) { cullingEnabled = enable; }
    /// Enable/disable layer caching
    void enableCache(bool enable) { cacheEnabled = enable; }
    
    // New Source-based optimization features
    void enableSourceCaching(bool enable) { sourceCachingEnabled_ = enable; }
    void setLODLevel(float level) { lodLevel_ = level; }
    void enableDebugMode(bool enable) { debugMode_ = enable; }
    void clearCache();
    
    // ========================================================================
    // Render target and texture management
    // ========================================================================
    
    /// Access to render targets
    RenderTarget& getMainTarget() { return mainTarget; }
    RenderTarget& getTempTarget() { return tempTarget; }
    
    /// Footage texture management
    void loadFootage(const std::string& path, const std::string& id);
    ofTexture* getFootageTexture(const std::string& id);
    
    // ========================================================================
    // Performance monitoring
    // ========================================================================
    
    /// Legacy performance metrics (maintained for compatibility)
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
    
    /// New Source-based performance statistics
    RenderStats getLastRenderStats() const { return lastStats_; }
    
private:
    // ========================================================================
    // Source-based rendering helpers (new functionality)
    // ========================================================================
    
    /// Create layer-specific rendering context
    RenderContext createLayerContext(const Layer& layer, const RenderContext& parentContext);
    /// Apply layer transform to rendering context
    void applyLayerTransform(const Layer& layer, RenderContext& context);
    /// Apply layer blending to rendering context
    void applyLayerBlending(const Layer& layer, RenderContext& context);
    /// Check if layer should be culled for the new system
    bool shouldCullLayer(const Layer& layer, const RenderContext& context);
    
    // ========================================================================
    // Legacy rendering pipeline (maintained for compatibility)
    // ========================================================================
    
    // Core rendering pipeline
    void setupRenderState();
    void restoreRenderState();
    void applyLayerTransform(const Layer& layer, float currentTime);
    
    // Layer type specific rendering
    void renderFootage(const Layer& layer, float currentTime);
    void renderShape(const Layer& layer, float currentTime);
    void renderText(const Layer& layer, float currentTime);
    void renderSolid(const Layer& layer, float currentTime);
    void renderComposition(const Layer& layer, float currentTime);
    void renderNull(const Layer& layer, float currentTime);
    
    // Blend mode implementation
    void applyBlendMode(BlendMode mode);
    void setupBlendModeShader(BlendMode mode);
    
    // Mask system (foundation)
    void renderWithMask(const Layer& layer, float currentTime);
    void applyLayerMasks(const Layer& layer, float currentTime);
    
    // Optimization systems (legacy)
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
    
    // New Source-based configuration
    bool sourceCachingEnabled_;
    float lodLevel_;
    bool debugMode_;
    RenderStats lastStats_;
    
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