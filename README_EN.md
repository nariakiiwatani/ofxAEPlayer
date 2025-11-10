# ofxAEPlayer

An openFrameworks addon for exporting and playing back After Effects compositions in real-time.

## Overview

ofxAEPlayer is an addon that exports After Effects compositions as JSON files and plays them back within openFrameworks applications. It uses an included ExtendScript tool to extract composition data and performs rendering on the C++ side.

## Key Features

- Export and playback of After Effects compositions
- Basic 2D transforms (position, scale, rotation, opacity)
- Shape layers (ellipse, rectangle, polygon, path, group)
- Image and video asset loading
- Pre-compositions (nested compositions)
- Mask functionality
- Track mattes (alpha, luma, inverted)
- Keyframe animation (linear, bezier, hold interpolation)
- Expression baking
- Time remapping
- Markers (composition, layer)
- Parent-child hierarchy

## System Requirements

- openFrameworks 0.12.0 or later
- After Effects (for composition export)
- C++17 compatible compiler

## Installation

1. Clone or place this repository in the openFrameworks `addons` directory

```
of_v0.12.0_osx_release/
  addons/
    ofxAEPlayer/
```

2. Add the following to your project's `addons.make`

```
ofxAEPlayer
```

## Usage

### 1. Exporting from After Effects

1. Run `tools/ExportComposition.jsx` from the After Effects script panel
2. Select the output folder
3. Select the target composition and execute
4. Composition data and assets will be exported in JSON format

### 2. Loading and Playing in openFrameworks

```cpp
#include "ofxAEPlayer.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        comp_ = std::make_shared<ofx::ae::Composition>();
        comp_->load("path/to/composition.json");
        
        timeline_ = 0;
        is_playing_ = true;
    }
    
    void update() {
        if(is_playing_ && comp_) {
            const auto& info = comp_->getInfo();
            if(++timeline_ >= info.end_frame) {
                timeline_ = info.start_frame;
            }
            if(comp_->setFrame(timeline_)) {
                comp_->update();
            }
        }
    }
    
    void draw() {
        if(comp_) {
            comp_->draw(0, 0);
        }
    }
    
private:
    std::shared_ptr<ofx::ae::Composition> comp_;
    int timeline_;
    bool is_playing_;
};
```

## File Structure

```
ofxAEPlayer/
├── src/                      # C++ source code
│   ├── ofxAEPlayer.h        # Main header
│   ├── core/                # Core functionality (Composition, Layer, Mask, etc.)
│   ├── data/                # Data structures (Enums, KeyframeData, PathData, etc.)
│   ├── prop/                # Properties (Transform, Shape, Mask, etc.)
│   ├── source/              # Sources (Shape, Solid, Still, Video, Composition, etc.)
│   └── utils/               # Utilities (BlendMode, TrackMatte, AssetManager, etc.)
├── tools/                   # After Effects export tools
│   └── ExportComposition.jsx
├── example/                 # Basic usage example
├── example-collision/       # Collision detection example
├── example-marker/          # Marker usage example
└── docs/                    # Documentation
```

## API Overview

### Composition

Class that manages the entire composition.

```cpp
namespace ofx::ae {

class Composition {
public:
    // Load composition
    bool load(const std::string& filepath);
    
    // Set frame and update
    bool setFrame(int frame);
    void update();
    
    // Draw
    void draw(float x, float y);
    
    // Get composition info
    const CompositionInfo& getInfo() const;
    
    // Layer operations
    std::vector<std::shared_ptr<Layer>> getLayers() const;
    
    // Get size
    float getWidth() const;
    float getHeight() const;
    
    // Get current time
    float getCurrentTime() const;
};

}
```

### Layer

Class representing a layer.

```cpp
namespace ofx::ae {

class Layer {
public:
    // Layer info
    std::string getName() const;
    SourceType getSourceType() const;
    
    // Size
    float getWidth() const;
    float getHeight() const;
    
    // Get source
    std::shared_ptr<LayerSource> getSource() const;
};

}
```

### SourceType

Layer source types.

```cpp
enum class SourceType {
    SHAPE,          // Shape layer
    SOLID,          // Solid layer
    STILL,          // Still image
    VIDEO,          // Video
    SEQUENCE,       // Image sequence
    COMPOSITION,    // Pre-composition
    CAMERA,         // Camera (not implemented)
    LIGHT,          // Light (not implemented)
    TEXT,           // Text (not implemented)
    ADJUSTMENT,     // Adjustment layer (not implemented)
    NULL_OBJECT     // Null object (not implemented)
};
```

## Feature Support Status

See [`docs/AE_FEATURE_SUPPORT_STATUS_EN.md`](docs/AE_FEATURE_SUPPORT_STATUS_EN.md) for detailed feature support information.

### Supported Features

- ✅ Basic 2D animation
- ✅ Shape layers (basic shapes)
- ✅ Image and video assets
- ✅ Pre-compositions
- ✅ Masks
- ✅ Track mattes
- ✅ Markers
- ✅ Parent-child relationships

### Unsupported Features

- ❌ 3D features (camera, light, 3D layers)
- ❌ Text layers
- ❌ Effects
- ❌ Layer styles
- ❌ Advanced blend modes (beyond basic 7 types)

## Limitations

1. **3D Features**: Camera, light, and 3D layers are not supported
2. **Effects**: All effects are unsupported (pre-rendering required)
3. **Text Layers**: Completely unsupported (shape conversion or rasterization required)
4. **Blend Modes**: Only 7 basic types supported (Normal, Add, Subtract, Multiply, Screen, Lighten, Darken)
5. **Expressions**: Baked to all frames during export
6. **Video Playback**: May be unstable depending on the environment

## Sample Projects

### example

Basic composition playback example.

```bash
cd example
make
./bin/example
```

### example-collision

Collision detection example using shape layers.

### example-marker

Event trigger example using markers.

## License

This project is released under the [MIT License](LICENSE).

## Contributing

Bug reports and feature requests are welcome via Issues.

## References

- [openFrameworks](https://openframeworks.cc/)
- [After Effects Scripting Guide](https://ae-scripting.docsforadobe.dev/)