# Frame-to-Time Conversion Summary

## Overview

This document summarizes the conversion of frame-based logic to time-based logic throughout the codebase. The goal was to eliminate unnecessary frame-to-time conversions and work directly with time values wherever possible.

## Converted Components

### 1. Player Internal State (`src/ofxAEPlayer.cpp`, `src/ofxAEPlayer.h`)

**Changed:**
- `target_frame_` → `target_time_` (from `float` frame counter to `double` time in seconds)
- `constrainFrame(int/float)` → `constrainTime(double)` (unified into single time-based constraint)

**Impact:**
All internal playback state now operates on seconds rather than frame numbers. Frame conversions only happen at the API boundary.

### 2. Loop Logic (`src/ofxAEPlayer.cpp::updatePlayback()`)

**Before (frame-based):**
```cpp
float frame_delta = elapsed * fps * speed_;
float new_frame = target_frame_ + frame_delta;

switch(loop_state_) {
    case OF_LOOP_NORMAL: {
        float range = duration * fps;
        new_frame = fmod(offset, range) + start_frame;
    }
    // ... more frame arithmetic
}

double time = new_frame / fps;
composition_.setTime(time);
```

**After (time-based):**
```cpp
double time_delta = elapsed * speed_;
double new_time = target_time_ + time_delta;

switch(loop_state_) {
    case OF_LOOP_NORMAL: {
        new_time = fmod(offset, duration) + start_time;
    }
    // ... direct time arithmetic
}

composition_.setTime(new_time);
```

**Benefits:**
- Eliminates frame-to-time conversion overhead
- More precise calculations (no frame quantization)
- Simpler code (fewer intermediate variables)

### 3. Position/Progress API (`src/ofxAEPlayer.cpp`)

**Changed:**
- `getPosition()`: Now calculates `target_time_ / duration` directly (no frame conversion)
- `setPosition()`: Now calculates `pct * duration` directly (no frame conversion)

**Benefits:**
- More accurate position tracking
- No rounding errors from frame conversions

### 4. Frame Stepping Methods (`src/ofxAEPlayer.cpp`)

**Changed:**
- `nextFrame()` / `previousFrame()`: Now calculate frame duration (`1.0 / fps`) and add/subtract time directly

**Before:**
```cpp
void Player::nextFrame() {
    setFrame(target_frame_ + 1);  // Frame-based increment
}
```

**After:**
```cpp
void Player::nextFrame() {
    double frame_duration = 1.0 / composition_.getFps();
    target_time_ = constrainTime(target_time_ + frame_duration);
    composition_.setTime(target_time_);
}
```

### 5. Layer In/Out Points (`src/core/ofxAELayer.h`, `src/core/ofxAELayer.cpp`)

**Changed:**
- `in_` / `out_` (int frames) → `in_time_` / `out_time_` (double seconds)
- `getInPoint()` / `getOutPoint()`: Now return stored time values directly
- `getDuration()`: Now calculates `out_time_ - in_time_` directly
- `isActiveAtTime()`: Now uses time values directly (no conversion)

**Benefits:**
- Consistent time-based representation throughout layer lifecycle
- Frame values only used when loading from JSON (one-time conversion)

## Retained Frame-Based Code

### 1. Public API Methods (Compatibility Layer)

**Kept:**
- `setFrame(int)` / `setFrame(float)` - Public API wrapper
- `getCurrentFrame()` - Returns `static_cast<int>(target_time_ * fps)`
- `getTotalNumFrames()` - Returns `static_cast<int>(duration * fps)`

**Reason:**
These are part of the public `ofBaseVideoPlayer` API and must remain for compatibility. They're now thin wrappers that convert to/from the internal time-based representation.

### 2. Image Sequence Frame Indexing (`src/source/ofxAESequenceSource.cpp`)

**Kept:**
```cpp
int new_index = static_cast<int>(time * fps_);
```

**Reason:**
Image sequences inherently require frame indices for file naming (e.g., `frame_001.png`, `frame_002.png`). The frame number is calculated from time only when needed for file lookup.

### 3. JSON Import (Layer In/Out)

**Kept:**
```cpp
int in_frame = 0;
int out_frame = 0;
json::extract(json, "in", in_frame);
json::extract(json, "out", out_frame);
in_time_ = static_cast<double>(in_frame) / parent_fps_;
out_time_ = static_cast<double>(out_frame) / parent_fps_;
```

**Reason:**
After Effects exports frame-based in/out points. Conversion happens once during load, then time values are used throughout.

## Performance & Accuracy Improvements

### 1. Fewer Conversions
- **Before:** Every update cycle converted `frame → time → setTime()`
- **After:** Direct time arithmetic, one `setTime()` call

### 2. No Frame Quantization
- **Before:** Time stored as frames lost sub-frame precision
- **After:** Full double-precision time representation

### 3. Simpler Code
- **Before:** Multiple conversion points, easy to introduce bugs
- **After:** Single source of truth (time), conversions only at API boundaries

## Migration Impact

### Breaking Changes
**None.** All public API methods remain unchanged. Internal refactoring only.

### Performance
- Slightly faster update loop (eliminated unnecessary multiplies/divides)
- More accurate timing (no cumulative rounding errors)

### Testing
All existing code using the public API should work identically. Internal time precision has improved.

## Architecture Summary

```
┌─────────────────────────────────────────────────────────┐
│  Public API (Frame-based for compatibility)             │
│  - setFrame(int/float)                                  │
│  - getCurrentFrame() → int                              │
│  - getTotalNumFrames() → int                            │
└─────────────────────────────────────────────────────────┘
                          ↓ ↑
              Conversion at API boundary
                          ↓ ↑
┌─────────────────────────────────────────────────────────┐
│  Internal State (Time-based)                            │
│  - target_time_: double                                 │
│  - updatePlayback(): pure time arithmetic               │
│  - Loop logic: time-based                               │
│  - Layer in/out: time-based                             │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│  Special Cases (Frame indexing only where needed)       │
│  - Image sequence: time → frame index → file lookup     │
│  - JSON import: frame → time (one-time conversion)      │
└─────────────────────────────────────────────────────────┘
```

## Conclusion

The codebase now operates primarily with time-based logic. Frame numbers are:
1. Used only at API boundaries for compatibility
2. Calculated on-demand from time values
3. Required only for specific use cases (file indexing)

This provides better precision, cleaner code, and a more maintainable architecture while maintaining full backward compatibility with existing code.