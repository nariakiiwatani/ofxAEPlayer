#include "ofxAERenderEngine.h"
#include "ofxAECompositionSource.h"
#include <algorithm>

namespace ofx {
namespace ae {

//--------------------------------------------------------------
// RenderTarget Implementation
//--------------------------------------------------------------

RenderTarget::RenderTarget() {
}

RenderTarget::~RenderTarget() {
}

void RenderTarget::allocate(int width, int height) {
    fbo.allocate(width, height, GL_RGBA);
}

void RenderTarget::begin() {
    if (fbo.isAllocated()) {
        fbo.begin();
    }
}

void RenderTarget::end() {
    if (fbo.isAllocated()) {
        fbo.end();
    }
}

void RenderTarget::clear() {
    if (fbo.isAllocated()) {
        fbo.begin();
        ofClear(0, 0, 0, 0);
        fbo.end();
    }
}

void RenderTarget::clear(ofColor color) {
    if (fbo.isAllocated()) {
        fbo.begin();
        ofClear(color);
        fbo.end();
    }
}

//--------------------------------------------------------------
// RenderEngine Implementation
//--------------------------------------------------------------

RenderEngine::RenderEngine()
    : renderWidth(1920)
    , renderHeight(1080)
    , qualityScale(1.0f)
    , cullingEnabled(true)
    , cacheEnabled(true)
    , sourceCachingEnabled_(true)
    , lodLevel_(1.0f)
    , debugMode_(false)
    , lastRenderTime(0.0f)
    , frameCount(0)
    , averageRenderTime(0.0f) {
    
    // Initialize render targets
    mainTarget.allocate(renderWidth, renderHeight);
    tempTarget.allocate(renderWidth, renderHeight);
    maskTarget.allocate(renderWidth, renderHeight);
    
    // Initialize blend shaders
    initializeBlendShaders();
    
    // Initialize LOD levels
    initializeLODLevels();
    
    // Initialize new Source-based statistics
    lastStats_.reset();
}

RenderEngine::~RenderEngine() {
}

void RenderEngine::setSize(int width, int height) {
    renderWidth = width;
    renderHeight = height;
    
    // Reallocate render targets
    mainTarget.allocate(width, height);
    tempTarget.allocate(width, height);
    maskTarget.allocate(width, height);
}

void RenderEngine::setQuality(float quality) {
    qualityScale = ofClamp(quality, 0.1f, 1.0f);
}

void RenderEngine::render(const Composition& comp, float currentTime) {
    frameCount++;
    float startTime = ofGetElapsedTimef();
    
    // Reset frame metrics
    frameMetrics = PerformanceMetrics();
    frameMetrics.totalRenderTime = startTime;
    
    // Clear main target
    mainTarget.clear();
    mainTarget.begin();
    
    setupRenderState();
    
    // Get layers and sort by index (background to foreground)
    auto layers = comp.getLayers();
    std::sort(layers.begin(), layers.end(), 
        [](const std::shared_ptr<Layer>& a, const std::shared_ptr<Layer>& b) {
            // Higher index = background (rendered first)
            return a->getInfo().in_point > b->getInfo().in_point; 
        });
    
    // Render each layer with performance optimizations
    for (const auto& layer : layers) {
    	if (layer && layer->isVisible()) {
    		// Convert time to frame time for layer calculations
    		float compositionFrameTime = currentTime * comp.getInfo().fps;
    		
    		// Use the new layer time management methods
    		if (layer->isActiveAtTime(compositionFrameTime)) {
    			// Calculate layer-relative time for proper keyframe handling
    			float layerRelativeTime = layer->getLayerTime(compositionFrameTime);
    			
    			// Update layer to the correct frame based on layer-relative time
    			layer->setCurrentFrame(static_cast<int>(layerRelativeTime + layer->getInfo().in_point));
    			
    			// Culling check
    			bool shouldCull = cullingEnabled && shouldCullLayer(*layer, currentTime);
    			if (shouldCull) {
    				frameMetrics.layersCulled++;
    				continue;
    			}
    			
    			// Cache check
    			if (cacheEnabled && isLayerCached(*layer, currentTime)) {
    				frameMetrics.layersCached++;
    				drawCachedLayer(*layer);
    			} else {
    				frameMetrics.layersRendered++;
    				renderLayer(*layer, currentTime);
    				
    				// Update cache if enabled
    				if (cacheEnabled) {
    					updateLayerCache(*layer, currentTime);
    				}
    			}
    		}
    	}
    }
    
    restoreRenderState();
    mainTarget.end();
    
    // Update performance metrics
    float endTime = ofGetElapsedTimef();
    lastRenderTime = endTime - startTime;
    frameMetrics.totalRenderTime = lastRenderTime;
    
    // Update running averages
    averageRenderTime = (averageRenderTime * 0.95f) + (lastRenderTime * 0.05f);
    frameMetrics.averageFPS = frameCount > 0 ? 1.0f / averageRenderTime : 0.0f;
    
    // Copy frame metrics to current metrics
    currentMetrics = frameMetrics;
}

void RenderEngine::renderLayer(const Layer& layer, float currentTime) {
    // Save current transform state
    ofPushMatrix();
    
    // Apply layer transform using the existing TransformNode system
    applyLayerTransform(layer, currentTime);
    
    // Calculate layer opacity
    float opacity = calculateLayerOpacity(layer, currentTime);
    
    // Early exit if completely transparent
    if (opacity <= 0.0f) {
        ofPopMatrix();
        return;
    }
    
    // Set up blending and opacity
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255, 255, 255, opacity * 255);
    
    // Determine layer type and render accordingly
    RenderLayerType type = getLayerRenderType(layer);
    
    // Render based on layer type
    switch (type) {
        case RenderLayerType::FOOTAGE:
            renderFootage(layer, currentTime);
            break;
        case RenderLayerType::SHAPE:
            renderShape(layer, currentTime);
            break;
        case RenderLayerType::TEXT:
            renderText(layer, currentTime);
            break;
        case RenderLayerType::SOLID:
            renderSolid(layer, currentTime);
            break;
        case RenderLayerType::COMPOSITION:
            renderComposition(layer, currentTime);
            break;
        case RenderLayerType::NULL_LAYER:
            renderNull(layer, currentTime);
            break;
    }
    
    // Restore transform state
    ofPopMatrix();
}

void RenderEngine::applyLayerTransform(const Layer& layer, float currentTime) {
    // Apply the transform matrix that includes keyframe interpolation
	layer.pushMatrix();
}

void RenderEngine::renderFootage(const Layer& layer, float currentTime) {
    // Get footage texture based on layer source
    std::string source = layer.getInfo().source;
    ofTexture* texture = getFootageTexture(source);
    
    if (texture && texture->isAllocated()) {
        // Calculate size based on quality
        float width = texture->getWidth() * qualityScale;
        float height = texture->getHeight() * qualityScale;
        
        // Center the texture (AE default behavior)
        texture->draw(-width/2, -height/2, width, height);
    } else {
        // Placeholder if texture not found
        ofSetColor(100, 100, 200, 128);
        ofDrawRectangle(-64, -64, 128, 128);
        ofSetColor(255);
    }
}

void RenderEngine::renderShape(const Layer& layer, float currentTime) {
    // Enhanced shape layer rendering implementation
    try {
        // Check if this is actually a shape layer
        if (layer.getInfo().type != Layer::SHAPE_LAYER) {
            ofLogWarning("RenderEngine") << "renderShape called on non-shape layer";
            return;
        }
        
        // Apply layer transform is already done in renderLayer()
        // Apply layer opacity is already done in renderLayer()
        
        // For now, we'll implement basic shape rendering
        // In full implementation, this would:
        // 1. Get ShapeSource from layer to access shape data
        // 2. Render each shape group with proper hierarchy
        // 3. Apply fill and stroke properties
        // 4. Handle shape animations
        
        // Basic shape rendering placeholder with enhanced visualization
        ofPushStyle();
        
        // Simulate multiple shapes in a layer
        ofSetColor(100, 150, 200, 180);
        ofDrawRectangle(-30, -30, 60, 60);
        
        ofSetColor(200, 100, 150, 180);
        ofDrawCircle(0, 0, 25);
        
        ofSetColor(150, 200, 100, 180);
        ofDrawTriangle(-20, 20, 20, 20, 0, -20);
        
        ofPopStyle();
        
        ofLogVerbose("RenderEngine") << "Rendered shape layer: " << layer.getInfo().name;
        
    } catch (const std::exception& e) {
        ofLogError("RenderEngine") << "Error rendering shape layer: " << e.what();
    }
}

void RenderEngine::renderText(const Layer& layer, float currentTime) {
    // Text layer rendering - placeholder
    // In a full implementation, this would handle fonts and text properties
    ofSetColor(255, 255, 255);
    // Note: Text positioning would need proper implementation
}

void RenderEngine::renderSolid(const Layer& layer, float currentTime) {
    // Solid color layer - fills the entire composition
    ofSetColor(128, 128, 128, 200); // Default gray
    ofDrawRectangle(-renderWidth/2, -renderHeight/2, renderWidth, renderHeight);
}

void RenderEngine::renderComposition(const Layer& layer, float currentTime) {
    // CompositionLayer specific rendering
    try {
        // Check if this is actually a composition layer
        if (layer.getInfo().type != Layer::COMPOSITION_LAYER) {
            ofLogWarning("RenderEngine") << "renderComposition called on non-composition layer";
            return;
        }
        
        // Get CompositionSource from the layer
        auto compositionSource = layer.getSource<CompositionSource>();
        if (!compositionSource) {
            ofLogError("RenderEngine") << "Layer does not have CompositionSource";
            return;
        }
        
        // Get child composition
        auto childComposition = compositionSource->getComposition();
        if (!childComposition) {
            ofLogWarning("RenderEngine") << "CompositionSource has no child composition";
            return;
        }
        
        // Calculate nested time using CompositionSource
        float nestedTime = currentTime; // Simple time mapping for now
        
        // CompositionSource handles its own FBO rendering in draw()
        // We just need to call the layer's draw method which will:
        // 1. Calculate nested time
        // 2. Render child composition to FBO
        // 3. Draw the FBO texture with layer transforms
        layer.draw(0, 0, layer.getWidth(), layer.getHeight());
        
        ofLogVerbose("RenderEngine") << "Rendered composition layer: " << layer.getInfo().name
                                     << " at nested time: " << nestedTime;
        
    } catch (const std::exception& e) {
        ofLogError("RenderEngine") << "Error rendering composition layer: " << e.what();
    }
}

void RenderEngine::renderNull(const Layer& layer, float currentTime) {
    // Null layers don't render anything - they're for transform hierarchy only
    // No-op
}

void RenderEngine::setupRenderState() {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255, 255, 255, 255);
}

void RenderEngine::restoreRenderState() {
    ofPopStyle();
}

float RenderEngine::calculateLayerOpacity(const Layer& layer, float currentTime) {
    // Get opacity from layer (includes keyframe interpolation)
    float opacity = layer.getOpacity();
    return ofClamp(opacity / 100.0f, 0.0f, 1.0f); // AE uses 0-100%, we need 0-1
}

RenderLayerType RenderEngine::getLayerRenderType(const Layer& layer) {
    // Map AE layer types to render types
    switch (layer.getInfo().type) {
        case Layer::AV_LAYER:
            return RenderLayerType::FOOTAGE;
        case Layer::VECTOR_LAYER:
            return RenderLayerType::SHAPE;
        case Layer::SHAPE_LAYER:
            return RenderLayerType::SHAPE;
        case Layer::COMPOSITION_LAYER:
            return RenderLayerType::COMPOSITION;
        default:
            return RenderLayerType::FOOTAGE;
    }
}

void RenderEngine::loadFootage(const std::string& path, const std::string& id) {
    ofImage image;
    if (image.load(path)) {
        footageTextures[id] = image.getTexture();
        textureLoadTimes[id] = ofGetElapsedTimef();
        ofLogNotice("RenderEngine") << "Loaded footage: " << id << " from " << path;
    } else {
        ofLogError("RenderEngine") << "Failed to load footage: " << path;
    }
}

ofTexture* RenderEngine::getFootageTexture(const std::string& id) {
    auto it = footageTextures.find(id);
    if (it != footageTextures.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool RenderEngine::shouldCullLayer(const Layer& layer, float currentTime) {
    if (!cullingEnabled) return false;
    
    // Basic culling - check if layer bounds intersect with screen
    ofRectangle bounds = getLayerBounds(layer, currentTime);
    ofRectangle screen(0, 0, renderWidth, renderHeight);
    
    return !bounds.intersects(screen);
}

ofRectangle RenderEngine::getLayerBounds(const Layer& layer, float currentTime) {
    // Simplified bounds calculation
    // In a full implementation, this would account for transforms and layer content
    return ofRectangle(-100, -100, 200, 200);
}

void RenderEngine::updateLayerCache(const Layer& layer, float currentTime) {
    if (!cacheEnabled) return;
    
    // Simple cache implementation
    int layerId = reinterpret_cast<intptr_t>(&layer); // Use layer address as ID
    LayerCache& cache = layerCache[layerId];
    
    // Mark cache as updated
    cache.lastUpdateTime = currentTime;
    cache.isDirty = false;
}

bool RenderEngine::isLayerCached(const Layer& layer, float currentTime) {
    if (!cacheEnabled) return false;
    
    int layerId = reinterpret_cast<intptr_t>(&layer);
    auto it = layerCache.find(layerId);
    
    if (it != layerCache.end()) {
        const LayerCache& cache = it->second;
        // Cache is valid for a short time
        return !cache.isDirty && (currentTime - cache.lastUpdateTime) < 0.1f;
    }
    
    return false;
}

void RenderEngine::drawCachedLayer(const Layer& layer) {
    int layerId = reinterpret_cast<intptr_t>(&layer);
    auto it = layerCache.find(layerId);
    
    if (it != layerCache.end()) {
        const LayerCache& cache = it->second;
        if (cache.texture.isAllocated()) {
            float opacity = calculateLayerOpacity(layer, 0);
            ofSetColor(255, 255, 255, opacity * 255);
            cache.texture.draw(0, 0);
        }
    }
}

void RenderEngine::renderWithMask(const Layer& layer, float currentTime) {
    // Placeholder for mask system
    renderLayer(layer, currentTime);
}

void RenderEngine::applyLayerMasks(const Layer& layer, float currentTime) {
    // Placeholder for mask application
}

void RenderEngine::applyBlendMode(BlendMode mode) {
    // Apply blend modes using OpenFrameworks blend modes
    switch (mode) {
        case BlendMode::NORMAL:
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            break;
        case BlendMode::MULTIPLY:
            ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
            break;
        case BlendMode::ADD:
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            break;
        case BlendMode::SUBTRACT:
            ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
            break;
        case BlendMode::SCREEN:
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);
            break;
        default:
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            break;
    }
}

void RenderEngine::setupBlendModeShader(BlendMode mode) {
    // Placeholder for blend mode shader setup
    // In a full implementation, this would load custom GLSL shaders
    // for accurate AE-compatible blending
}

void RenderEngine::initializeBlendShaders() {
    // Placeholder for blend shader initialization
    // Would load GLSL shaders for each blend mode
}

void RenderEngine::initializeLODLevels() {
    lodLevels.clear();
    
    // Define LOD levels based on distance/scale
    lodLevels.push_back({0.0f, 1.0f});    // Full quality
    lodLevels.push_back({100.0f, 0.75f}); // 75% quality
    lodLevels.push_back({200.0f, 0.5f});  // 50% quality
    lodLevels.push_back({400.0f, 0.25f}); // 25% quality
}

float RenderEngine::calculateLODQuality(const Layer& layer, float currentTime) {
    // Calculate distance-based LOD
    ofRectangle bounds = getLayerBounds(layer, currentTime);
    float distance = glm::distance(glm::vec2(bounds.getCenter().x, bounds.getCenter().y),
                                  glm::vec2(renderWidth/2, renderHeight/2));
    
    for (int i = lodLevels.size() - 1; i >= 0; i--) {
        if (distance >= lodLevels[i].distance) {
            return lodLevels[i].quality;
        }
    }
    
    return 1.0f; // Default to full quality
}

//--------------------------------------------------------------
// New Source-based rendering implementation
//--------------------------------------------------------------

void RenderEngine::renderLayer(const Layer& layer, const RenderContext& context) {
    float startTime = ofGetElapsedTimef();
    
    // Performance tracking reset for this render call
    if (debugMode_) {
        lastStats_.reset();
    }
    
    // Check if layer should be culled in the new system
    if (shouldCullLayer(layer, context)) {
        lastStats_.layersCulled++;
        if (debugMode_) {
            ofLogVerbose("RenderEngine") << "Culled layer: " << layer.getName();
        }
        return;
    }
    
    // Create layer-specific rendering context
    RenderContext layerContext = createLayerContext(layer, context);
    
    // Apply layer transforms and blending
    ofPushMatrix();
    ofPushStyle();
    
    applyLayerTransform(layer, layerContext);
    applyLayerBlending(layer, layerContext);
    
    // Get the layer's source for rendering
    auto source = layer.getSource();
    if (source) {
        // Delegate rendering to the Source
        source->draw(layerContext);
        lastStats_.sourcesRendered++;
        
        if (debugMode_) {
            ofLogVerbose("RenderEngine") << "Rendered layer via Source: " << layer.getName()
                                         << " SourceType: " << static_cast<int>(source->getSourceType());
        }
    } else {
        // Fallback to legacy rendering if no Source attached
        if (debugMode_) {
            ofLogWarning("RenderEngine") << "Layer has no Source, using legacy rendering: " << layer.getName();
        }
        renderLayer(layer, layerContext.currentTime); // Call legacy method
    }
    
    ofPopStyle();
    ofPopMatrix();
    
    lastStats_.layersRendered++;
    
    // Update performance metrics
    if (debugMode_) {
        lastStats_.renderTimeMs += (ofGetElapsedTimef() - startTime) * 1000.0f;
    }
}

void RenderEngine::renderComposition(const Composition& comp, const RenderContext& context) {
    float startTime = ofGetElapsedTimef();
    
    if (debugMode_) {
        lastStats_.reset();
        ofLogVerbose("RenderEngine") << "Starting Source-based composition render: " << "Composition";
    }
    
    // Setup composition rendering
    mainTarget.clear();
    mainTarget.begin();
    
    setupRenderState();
    
    // Get layers and sort by rendering order (background to foreground)
    auto layers = comp.getLayers();
    std::sort(layers.begin(), layers.end(),
        [](const std::shared_ptr<Layer>& a, const std::shared_ptr<Layer>& b) {
            // Higher index = background (rendered first)
            return a->getInfo().in_point > b->getInfo().in_point;
        });
    
    // Render each layer using Source-based pipeline
    for (const auto& layer : layers) {
        if (layer && layer->isVisible()) {
            // Check if layer is active at current time
            float compositionFrameTime = context.currentTime * comp.getInfo().fps;
            if (layer->isActiveAtTime(compositionFrameTime)) {
                // Create layer-specific context
                RenderContext layerContext = context;
                layerContext.currentTime = layer->getLayerTime(compositionFrameTime);
                
                // Render using new Source-based method
                renderLayer(*layer, layerContext);
            }
        }
    }
    
    restoreRenderState();
    mainTarget.end();
    
    // Update total performance metrics
    if (debugMode_) {
        lastStats_.renderTimeMs = (ofGetElapsedTimef() - startTime) * 1000.0f;
        ofLogNotice("RenderEngine") << "Composition render complete - Layers: " << lastStats_.layersRendered
                                    << ", Sources: " << lastStats_.sourcesRendered
                                    << ", Culled: " << lastStats_.layersCulled
                                    << ", Time: " << lastStats_.renderTimeMs << "ms";
    }
}

RenderContext RenderEngine::createLayerContext(const Layer& layer, const RenderContext& parentContext) {
    RenderContext layerContext = parentContext;
    
    // Apply layer-specific properties to context
    layerContext.opacity *= layer.getOpacity() / 100.0f;
    
    // Set layer blend mode (if supported by RenderContext)
    // layerContext.blendMode = layer.getBlendMode(); // Uncomment when RenderContext supports this
    
    // Adjust time for layer timing
    float compositionTime = parentContext.currentTime;
    layerContext.currentTime = layer.getLayerTime(compositionTime);
    
    // Apply layer transform matrix
    // The LayerSource will handle transform application during draw()
    
    if (debugMode_) {
        ofLogVerbose("RenderEngine") << "Created layer context for: " << layer.getName()
                                     << " Opacity: " << layerContext.opacity
                                     << " Time: " << layerContext.currentTime;
    }
    
    return layerContext;
}

void RenderEngine::applyLayerTransform(const Layer& layer, RenderContext& context) {
    // Apply layer transform using the existing TransformNode system
    // This maintains compatibility with the current Layer transform implementation
    layer.pushMatrix();
    
    if (debugMode_) {
        ofLogVerbose("RenderEngine") << "Applied transform for layer: " << layer.getName();
    }
}

void RenderEngine::applyLayerBlending(const Layer& layer, RenderContext& context) {
    // Apply layer opacity
    float totalOpacity = context.opacity * (layer.getOpacity() / 100.0f);
    ofSetColor(255, 255, 255, totalOpacity * 255);
    
    // Apply blend mode
    // For now, use basic alpha blending; could be enhanced with full AE blend mode support
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    if (debugMode_) {
        ofLogVerbose("RenderEngine") << "Applied blending for layer: " << layer.getName()
                                     << " Opacity: " << totalOpacity;
    }
}

bool RenderEngine::shouldCullLayer(const Layer& layer, const RenderContext& context) {
    // Time-based culling - check if layer is active
    if (!layer.isActiveAtTime(context.currentTime)) {
        return true;
    }
    
    // Opacity-based culling
    if (layer.getOpacity() <= 0.001f || context.opacity <= 0.001f) {
        return true;
    }
    
    // LOD-based spatial culling
    if (lodLevel_ > 0.0f) {
        // Get layer bounds for culling check
        ofRectangle layerBounds = getLayerBounds(layer, context.currentTime);
        ofRectangle viewportBounds(context.x, context.y, context.w, context.h);
        
        // Cull if layer is completely outside viewport
        if (!layerBounds.intersects(viewportBounds)) {
            return true;
        }
    }
    
    return false;
}

void RenderEngine::clearCache() {
    // Clear legacy cache
    layerCache.clear();
    
    // Clear Source-based cache if implemented
    if (sourceCachingEnabled_) {
        // Future implementation: clear Source-specific caches
        if (debugMode_) {
            ofLogNotice("RenderEngine") << "Cleared all caches";
        }
    }
}

} // namespace ae
} // namespace ofx
