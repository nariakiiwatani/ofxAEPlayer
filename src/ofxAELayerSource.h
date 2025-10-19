#pragma once

#include "ofMain.h"
#include "ofJson.h"
#include "ofRectangle.h"
#include "ofxAERenderContext.h"
#include <string>
#include <memory>

namespace ofx { namespace ae {

class Visitor;

class LayerSource : public ofBaseDraws, public ofBaseUpdates
{
public:
    enum SourceType {
        SHAPE,        // Vector shapes and paths
        COMPOSITION,  // Nested compositions (pre-comps)
        SOLID,        // Solid color layers
        CAMERA,       // 3D camera layers
        LIGHT,        // 3D light layers
        ADJUSTMENT,   // Adjustment layers (effects only)
        TEXT,         // Text layers with typography
        NULL_OBJECT,  // Null objects for hierarchy/parenting
        STILL,        // Still images via TexturePlayer
        VIDEO,        // Video files via ofVideoPlayer
        SEQUENCE,     // Image sequences via SequencePlayer
		 UNKNOWN,
		 NUM_TYPES
    };

	virtual ~LayerSource() = default;

	virtual void accept(Visitor& visitor);

	virtual bool setup(const ofJson &json) { return false; }
	virtual bool load(const std::filesystem::path &filepath) { return setup(ofLoadJson(filepath)); }

	virtual void update() override {}
	virtual bool setFrame(int frame) { return false; }

	using ofBaseDraws::draw;
	virtual void draw(float x, float y, float w, float h) const override {}

	virtual SourceType getSourceType() const = 0;


	virtual std::string getDebugInfo() const { return "LayerSource"; }

    static std::string sourceTypeToString(SourceType type) {
        switch (type) {
            case SHAPE: return "shape";
            case COMPOSITION: return "composition";
            case SOLID: return "solid";
            case CAMERA: return "camera";
            case LIGHT: return "light";
            case ADJUSTMENT: return "adjustment";
            case TEXT: return "text";
            case NULL_OBJECT: return "null";
            case STILL: return "still";
            case VIDEO: return "video";
            case SEQUENCE: return "sequence";
            default: return "unknown";
        }
    }
	static SourceType typeNameToSourceType(std::string name) {
		for(int i = 0; i < NUM_TYPES; ++i) {
			auto type = SourceType(i);
			if(name == sourceTypeToString(type)) {
				return type;
			}
		}
		return UNKNOWN;
	}
	static std::unique_ptr<LayerSource> createSourceOfType(SourceType type);
	static std::unique_ptr<LayerSource> createSourceOfType(std::string type) {
		return createSourceOfType(typeNameToSourceType(type));
	}

};
}} // namespace ofx::ae
