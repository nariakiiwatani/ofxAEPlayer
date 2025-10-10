#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include "ofGraphicsBaseTypes.h"
#include "ofJson.h"
#include "ofxAEMarker.h"
#include "ofxAEKeyframe.h"
#include "ofxAERenderContext.h"
#include "ofxAELayerSource.h"
#include "utils/TransformNode.h"
#include "utils/Hierarchical.h"

namespace ofx { namespace ae {

// Forward declarations
class LayerSource;

/**
 * Layer - Unified layer class supporting the new Source-based architecture
 *
 * This refactored Layer class replaces the inheritance-based system with
 * composition, allowing a single Layer class to handle different source types
 * (footage, shapes, compositions, etc.) through LayerSource implementations.
 *
 * Key features:
 * - Single Layer class handles all source types via composition
 * - Full backward compatibility with existing JSON formats
 * - Integration with TransformNode and Hierarchical systems
 * - Efficient rendering pipeline with RenderContext
 * - Time-based animation support with keyframes
 */
class Layer : public TransformNode
{
public:
    // Legacy enum maintained for backward compatibility
    enum LayerType {
        AV_LAYER,
        VECTOR_LAYER,
        SHAPE_LAYER,
        COMPOSITION_LAYER
    };
    
    // Legacy structure maintained for backward compatibility
    struct Info {
        std::string name;
        LayerType type;
        std::string source;
        std::string sourceType;
        int in_point;
        int out_point;
        std::string parent;
        std::vector<MarkerData> markers;
        
        Info() : type(AV_LAYER), in_point(0), out_point(0) {}
    };

    Layer();
    virtual ~Layer();
    
    // ========================================================================
    // Core Lifecycle Methods (Source-based implementation)
    // ========================================================================
    
	bool load(const std::string& layer_path);
    /**
     * Initialize the layer from JSON data exported from After Effects.
     * Creates appropriate LayerSource based on layer type and configures
     * both layer and source properties.
     *
     * @param json JSON object containing layer configuration
     * @return true if setup was successful, false otherwise
     */
    virtual bool setup(const ofJson& json, const std::filesystem::path &source_dir="");
    
    /**
     * Update the layer for the current time.
     * Handles time-dependent updates for both layer properties and source content.
     *
     * @param currentTime Current composition time in seconds
     */
    virtual void update(float currentTime);
    
    /**
     * Render the layer within the provided context.
     * Applies layer-level transformations and delegates rendering to the source.
     *
     * @param context Rendering context with transform, opacity, blend mode, etc.
     */
    virtual void draw(const RenderContext& context) const;
    
    // ========================================================================
    // Legacy Interface Support (maintained for compatibility)
    // ========================================================================
    
    /// Legacy draw method for ofBaseDraws compatibility
    virtual void draw(float x, float y, float w, float h) const;
    
    /// Legacy update method for ofBaseUpdates compatibility
    virtual void update();
    
    /// Get layer height from source bounds
    virtual float getHeight() const;
    
    /// Get layer width from source bounds
    virtual float getWidth() const;
    
    // ========================================================================
    // Source Management (new functionality)
    // ========================================================================
    
    /**
     * Set the layer source implementation
     * @param source Unique pointer to LayerSource implementation
     */
    void setSource(std::unique_ptr<LayerSource> source);
    
    /**
     * Get the current layer source
     * @return Pointer to source, or nullptr if not set
     */
    LayerSource* getSource() const { return source_.get(); }
    
    template<typename T>
    T* getSource() const {
        return dynamic_cast<T*>(source_.get());
    }
    
    /**
     * Get the source type from the attached source
     * @return Source type enum value
     */
    LayerSource::SourceType getSourceType() const;
    
    // ========================================================================
    // Layer Properties (existing functionality maintained)
    // ========================================================================
    
    /// Layer name
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }
    
    /// Time properties
    void setStartTime(float time) { startTime_ = time; }
    float getStartTime() const { return startTime_; }
    
    void setDuration(float duration) { duration_ = duration; }
    float getDuration() const { return duration_; }
    
    /// Visual properties
    void setOpacity(float opacity) { opacity_ = std::max(0.0f, std::min(1.0f, opacity)); }
    float getOpacity() const { return opacity_; }
    
    void setBlendMode(BlendMode mode) { blendMode_ = mode; }
    BlendMode getBlendMode() const { return blendMode_; }
    
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    
    // ========================================================================
    // Time Management (enhanced for source-based architecture)
    // ========================================================================
    
    /**
     * Check if layer is active at given time
     * @param time Composition time to check
     * @return true if layer should be active/visible
     */
    bool isActiveAtTime(float time) const;
    
    /**
     * Convert global composition time to layer-relative time
     * @param globalTime Composition time
     * @return Layer-relative time accounting for start time and speed
     */
    float getLocalTime(float globalTime) const;
    
    /**
     * Check if layer should be rendered at current time
     * @param currentTime Current composition time
     * @return true if layer should be rendered
     */
    bool shouldRender(float currentTime) const;
    
    /**
     * Prepare layer for rendering at given time
     * Updates keyframes and source state
     * @param currentTime Current composition time
     */
    void prepareForRendering(float currentTime);
    
    // ========================================================================
    // Legacy Support Methods (maintained for backward compatibility)
    // ========================================================================
    

    /// Legacy layer info access
    const Info& getInfo() const { return layer_info_; }
    
    /// Legacy keyframe support
    bool hasKeyframes() const;
    const ofJson& getKeyframes() const;
    void setCurrentFrame(int frame);
    int getCurrentFrame() const { return current_frame_; }
    
    /// Legacy time methods
    float getLayerTime(float compositionTime) const;
    void initializeAtInPoint();
    void handleOutPoint();
    
    /// Legacy parent support
    const std::string& getParentName() const { return layer_info_.parent; }
    
    /// Legacy source type methods
    const std::string& getLegacySourceType() const;
    bool isSourceTypeNone() const;
    bool isSourceTypeComposition() const;
    bool isSourceTypeStill() const;
    bool isSourceTypeVideo() const;
    bool isSourceTypeSequence() const;
    bool isSourceTypeFootage() const;
    bool isSourceTypeSolid() const;
    bool isSourceTypeUnknown() const;
    
    /// Legacy type conversion
    static LayerType stringToLayerType(const std::string& type_str);
    
    // ========================================================================
    // Debug and Diagnostics
    // ========================================================================
    
    /**
     * Get debug information about this layer
     * @return String containing layer and source debug info
     */
    std::string getDebugInfo() const;

private:
    // ========================================================================
    // Source-based Core Data
    // ========================================================================
    
    /// Main source implementation (replaces inheritance)
    std::unique_ptr<LayerSource> source_;
    
    // ========================================================================
    // Layer Properties
    // ========================================================================
    
    /// Layer identification
    std::string name_;
    
    /// Timing properties
    float startTime_;
    float duration_;
    
    /// Visual properties
    float opacity_;
    BlendMode blendMode_;
    bool visible_;
    
    // ========================================================================
    // Performance Optimization
    // ========================================================================
    
    /// Cached visibility state
    mutable bool isVisible_;
    
    /// Last update time for change detection
    mutable float lastUpdateTime_;
       
    // ========================================================================
    // Legacy Support Data (maintained for compatibility)
    // ========================================================================
    
    /// Legacy layer information structure
    Info layer_info_;
    
    /// Legacy keyframe data
    ofJson keyframes_;
    
    /// Legacy frame tracking
    int current_frame_;
    bool initialized_at_in_point_;
    
    // ========================================================================
    // JSON Parsing and Source Creation
    // ========================================================================
    
    /**
     * Parse layer-level properties from JSON
     * @param json JSON object to parse
     * @return true if parsing successful
     */
    bool parseLayerProperties(const ofJson& json);
    
    // ========================================================================
    // Legacy Support Methods (internal)
    // ========================================================================
    
    /// Legacy transform updates
    void updateTransformFromKeyframes();
    void parseTransformData(const ofJson& transform_data);
    ofJson getInterpolatedValue(const std::string& property, int frame) const;
    void applyTransformValue(const std::string& property, const ofJson& value);
    
    /// Legacy keyframe support
    void updateLegacyKeyframes(float currentTime);
};

}} // namespace ofx::ae
