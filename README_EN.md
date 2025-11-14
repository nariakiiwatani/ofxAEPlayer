# ofxAEPlayer

An openFrameworks addon for exporting and playing back After Effects compositions in real-time.

## Overview

ofxAEPlayer is an addon that exports After Effects compositions as JSON files and plays them back within openFrameworks applications. It uses an included ExtendScript tool to extract composition data and performs rendering on the C++ side.

## Key Features

- Export and playback of After Effects compositions
- Basic 2D transforms (position, scale, rotation, opacity)
- Shape layers (ellipse, rectangle, polygon, path, group, advanced stroke properties)
- Image and video asset loading
- Pre-compositions (nested compositions)
- Mask functionality
- Track mattes (alpha, luma, inverted)
- Keyframe animation (linear, bezier, hold interpolation)
- Expression baking
- Time remapping
- Markers (composition, layer)
- Parent-child hierarchy
- Automatic baking of effects and text (to PNG sequences)
- Efficient asset management with shared asset folders
- Automatic PNG conversion for PSD/AI files
- Duplicate frame reduction for image sequences

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

#### Export Options

The export tool provides the following options:

##### **Basic Settings**
- **Output Path**: Specify the output folder
- **Decimal Places**: Number of decimal places for property values (default: 4)

##### **Shared Assets**
- **Use Shared Assets Folder**: Use a common asset folder across multiple compositions
- **Shared Assets Path**: Location for shared assets (relative or absolute path)

##### **Export Mode**
- **Export as Full-Frame Animation**: Export all frames instead of keyframe interpolation
- **Process Nested Compositions**: Recursively export pre-compositions

##### **Automatic Baking**
- **Bake Effect Layers to Sequence**: Auto-detect effects and convert to PNG sequence
- **Bake Text Layers to Sequence**: Convert text layers to PNG sequence
- **Deduplicate Sequence Frames**: Automatically detect and remove duplicate frames

### 2. Loading and Playing in openFrameworks

```cpp
#include "ofxAEPlayer.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        comp_ = std::make_shared<ofx::ae::Composition>();
        comp_->load("path/to/composition.json");
        
        playback_time_ = 0.0;
        is_playing_ = true;
    }
    
    void update() {
        if(is_playing_ && comp_) {
            double duration = comp_->getDuration();
            playback_time_ += ofGetLastFrameTime();
            if(playback_time_ >= duration) {
                playback_time_ = 0.0;
            }
            comp_->setTime(playback_time_);
            comp_->update();
        }
    }
    
    void draw() {
        if(comp_) {
            comp_->draw(0, 0);
        }
    }
    
private:
    std::shared_ptr<ofx::ae::Composition> comp_;
    double playback_time_;
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
    
    // Set time and update
    void setTime(double seconds);
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
    double getTime() const;
    
    // Get duration
    double getDuration() const;
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
- ⚠️ Text layers (practically usable via automatic baking)
- ⚠️ Effects (practically usable via automatic baking)
- ❌ Layer styles
- ❌ Advanced blend modes (beyond basic 7 types)

## Advanced Features

### Automatic Baking System

The export tool automatically handles compositions with effects or text layers:

1. **Effect Auto-Baking**: Detects layers with effects, creates pre-compositions, and renders as PNG sequences
2. **Text Auto-Baking**: Automatically renders text layers as PNG sequences
3. **Expression Auto-Baking**: Bakes expressions to all frames during export

These features eliminate the need for manual pre-rendering.

### Layer Filtering

During export, only visible layers and their dependencies (parent layers, track mattes) are automatically extracted. Benefits include:

- Reduced export file size
- Improved loading and playback performance
- Exclusion of unnecessary layers

### Shared Asset Management

When multiple compositions use the same assets (images, videos, etc.), using a shared asset folder provides:

- Disk space savings
- Simplified asset management
- Reduced export time

## Limitations

1. **3D Features**: Camera, light, and 3D layers are not supported
2. **Effects**: Not directly supported (practically usable via automatic baking)
3. **Text Layers**: Not directly supported (practically usable via automatic baking)
4. **Blend Modes**: Only 7 basic types supported (Normal, Add, Subtract, Multiply, Screen, Lighten, Darken)
5. **Expressions**: Baked to all frames during export
6. **Video Playback**: May be unstable depending on the environment
7. **When Using Auto-Baking**: Effect and text parameters cannot be controlled in real-time

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