# Time-Based API Example (Phase 2)

This example demonstrates the new time-based API introduced in Phase 2 of the Time-Based Architecture Migration.

## Features

- **Time API Demo**: Uses `setTime(double seconds)` instead of `setFrame(int frame)`
- **API Comparison**: Toggle between Time API and Frame API to compare behavior
- **Variable Playback Speed**: Control playback speed from 0.1x to 5.0x
- **Real-time Feedback**: Display current time (seconds) and frame simultaneously

## Key Differences from Frame API

### Frame API (Legacy)
```cpp
// Frame-based approach
float frame = getCurrentFrame();
composition.setFrame(frame);
```

### Time API (New - Phase 2)
```cpp
// Time-based approach (more precise)
double time = getElapsedTime();
composition.setTime(time);  // Direct time in seconds
```

## Controls

- **SPACE**: Play/Pause
- **R**: Reset to beginning
- **D**: Toggle debug info
- **T**: Toggle between Time API and Frame API
- **+/-**: Increase/Decrease playback speed
- **LEFT/RIGHT**: Seek -/+ 1 second

## Setup

1. Export a composition from After Effects using the time-based export mode (check "時間ベースでキーフレームを書き出し" in ExportComposition.jsx)
2. Place the exported JSON in `bin/data/`
3. Update the filename in `ofApp::setup()` if needed
4. Compile and run

## Implementation Notes

### Time-Based Update Loop
```cpp
void ofApp::update() {
    if(use_time_api_) {
        // Calculate time directly from elapsed time
        double comp_duration_seconds = static_cast<double>(info_.duration) / info_.fps;
        double elapsed = ofGetElapsedTimef();
        double comp_time = fmod(elapsed * playback_speed_, comp_duration_seconds);
        
        // Use the new time API
        composition.setTime(comp_time);
        composition.update();
    }
}
```

### Advantages of Time API

1. **Precision**: Uses `double` (microsecond precision) instead of `int` frames
2. **FPS Independence**: Time is absolute, not dependent on frame rate
3. **Smooth Interpolation**: Better for variable playback speeds
4. **Real-time Sync**: Easier to sync with audio or external clocks

## Compatibility

The Time API is **fully backward compatible**. The Frame API methods (`setFrame()`) are maintained as compatibility wrappers that internally convert to time using the composition's FPS.

## Phase 2 Status

✅ Implemented:
- Time-based internal storage
- Time API in Composition
- Frame API compatibility layer
- Dual JSON format support (frame-based and time-based)

🚧 Coming in Phase 3:
- Layer-level time support
- Advanced time manipulation features
- Time remapping improvements

## See Also

- [TIME_MIGRATION_ROADMAP.md](../../TIME_MIGRATION_ROADMAP.md) - Full migration plan
- [ofxAEComposition.h](../../src/core/ofxAEComposition.h) - Time API implementation
- [ofxAETimeProperty.h](../../src/prop/ofxAETimeProperty.h) - Time-based property system