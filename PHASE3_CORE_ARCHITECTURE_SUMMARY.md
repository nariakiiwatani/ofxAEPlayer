# Phase 3: Core Architecture Migration Summary (Partial Completion)

## ✅ Completed: Core Architecture (Time API v2.0)

### What Has Been Done

#### 1. TimePropertyBase (ofxAETimeProperty.h) ✅
**Changes:**
- ✅ Removed `setFrame(int)` and `setFrame(float)` methods completely
- ✅ Removed frame conversion helper functions (`frameToTime`, `timeToFrame`)
- ✅ Time API only: `setTime(double)`, `getTime()`, `setFps()`, `getFps()`
- ✅ All TimeProperty-derived classes updated

**Status:** Complete - no Frame API remnants

#### 2. Composition Class (ofxAEComposition.h/cpp) ✅
**Changes:**
- ✅ Removed `setFrame(float)` and `getCurrentFrame()` methods
- ✅ Added Time API: `setTime(double)`, `getTime()`, `getDuration()`, `getFps()`
- ✅ Updated implementation to call Layer's `setTime()` instead of `setFrame()`

**New API:**
```cpp
bool setTime(double time);
double getTime() const;
float getTimeFloat() const;  // Convenience for float users
double getDuration() const;  // Duration in seconds
double getFps() const;
```

#### 3. Layer Class (ofxAELayer.h/cpp) ✅
**Changes:**
- ✅ Removed `setFrame(int)` and `setFrame(float)` methods
- ✅ Removed `isActiveAtFrame()` method
- ✅ Changed internal state from `float current_frame_` to `double current_time_`
- ✅ Added `parent_fps_` for time conversion
- ✅ Implemented full time-based API with proper time checks

**New API:**
```cpp
bool setTime(double time);
double getTime() const;
double getInPoint() const;      // In seconds
double getOutPoint() const;     // In seconds
double getDuration() const;     // In seconds
bool isActiveAtTime(double time) const;
```

**Implementation Notes:**
- Layer now properly converts time to frame for old Property system
- Uses `util::isNearTime()` for epsilon comparisons
- All time values in seconds (double precision)

#### 4. Configuration (ofxAEConfig.h) ✅
**Changes:**
- ✅ Version bumped to 2.0.0
- ✅ Removed preview flags
- ✅ Time API is now mandatory

```cpp
#define OFX_AE_VERSION_MAJOR 2
#define OFX_AE_VERSION_MINOR 0
#define OFX_AE_VERSION_PATCH 0
#define OFX_AE_USE_TIME_API 1
```

---

## ⚠️ Remaining Work: Application Layer Components

### What Still Needs To Be Done

#### 1. Source Classes (NOT DONE) ❌

**Files to update:**
- `src/source/ofxAELayerSource.h/cpp`
- `src/source/ofxAEVideoSource.h/cpp`
- `src/source/ofxAECompositionSource.h/cpp`
- `src/source/ofxAESequenceSource.h/cpp`
- `src/source/ofxAEStillSource.h/cpp`
- `src/source/ofxAESolidSource.h/cpp`
- `src/source/ofxAEShapeSource.h/cpp`

**Required changes:**
```cpp
// OLD (current)
virtual bool setFrame(int frame);
virtual bool setFrame(float frame);

// NEW (required)
virtual bool setTime(double time);
double getTime() const;
double getDuration() const;
```

**Implementation guide:**
```cpp
// Example for VideoSource
bool VideoSource::setTime(double time) {
    if(!player_) return false;
    
    // Convert time to frame for ofVideoPlayer
    float fps = player_->getTotalNumFrames() / player_->getDuration();
    int frame = static_cast<int>(time * fps);
    
    player_->setFrame(frame);
    return true;
}

double VideoSource::getDuration() const {
    return player_ ? player_->getDuration() : 0.0;
}
```

#### 2. Property System Bridge (NOT DONE) ❌

**Current issue:**
- Layer still uses OLD Property classes (`TransformProp`, `FloatProp` from `ofxAEProperty.h`)
- These classes still have `setFrame()` methods
- Layer's `setTime()` currently converts to frame to call old Property system

**Files affected:**
- `src/prop/ofxAETransformProp.h` - uses `PropertyGroup`
- `src/prop/ofxAEMaskProp.h` - uses `Property<T>`
- `src/prop/ofxAEShapeProp.h` - extensive use of `Property<T>`

**Two options:**

**Option A: Keep Frame Bridge (Easier)**
- Keep old Property system as-is
- Layer continues to convert time→frame internally
- No breaking changes to existing property code

**Option B: Full Migration (Cleaner)**
- Replace all `Property<T>` with `TimeProperty<T>`
- Replace `PropertyGroup` with `TimePropertyGroup`
- Update all property usage in Layer to use time API
- More work but cleaner architecture

**Recommended:** Option A for now, Option B for future refactoring.

#### 3. ofxAEPlayer Wrapper (NOT DONE) ❌

**File:** `src/ofxAEPlayer.h/cpp`

**Required changes:**
```cpp
class Player {
public:
    // REMOVE these methods
    void setFrame(int frame);
    void setFrame(float frame);
    int getCurrentFrame() const;
    int getTotalNumFrames() const;
    
    // ADD these methods
    void setTime(double time);
    double getTime() const;
    double getDuration() const;  // Already exists
    
private:
    // CHANGE state
    float target_frame_;  // → double target_time_;
    
    // UPDATE playback logic
    void updatePlayback();  // Needs time-based implementation
};
```

**Implementation guide:**
```cpp
void Player::setTime(double time) {
    if(!is_loaded_) return;
    
    double duration = getDuration();
    target_time_ = ofClamp(time, 0.0, duration);
    composition_.setTime(target_time_);
    is_frame_new_ = true;
}

void Player::updatePlayback() {
    double elapsed = ofGetLastFrameTime();
    double new_time = target_time_ + (elapsed * speed_);
    
    switch(loop_state_) {
        case OF_LOOP_NONE:
            new_time = ofClamp(new_time, 0.0, getDuration());
            if(new_time >= getDuration()) is_playing_ = false;
            break;
        case OF_LOOP_NORMAL:
            new_time = fmod(new_time, getDuration());
            break;
        // ... etc
    }
    
    if(new_time != target_time_) {
        setTime(new_time);
    }
}
```

#### 4. Examples (NOT DONE) ❌

**Files to update:**
- `example/src/ofApp.cpp`
- `example-marker/src/ofApp.cpp`
- `example-collision/src/ofApp.cpp`
- `example-time/src/ofApp.cpp`

**Migration pattern:**
```cpp
// OLD
void ofApp::update() {
    float frame = fmod(ofGetFrameNum(), composition.getInfo().duration);
    composition.setFrame(frame);
    composition.update();
}

// NEW
void ofApp::update() {
    double elapsed = ofGetElapsedTimef();
    double duration = composition.getDuration();  // in seconds
    double time = fmod(elapsed, duration);
    
    composition.setTime(time);
    composition.update();
}
```

**Marker example migration:**
```cpp
// OLD
void jumpToMarker(const string &name) {
    auto marker = findMarker(name);
    composition.setFrame(marker.frame);
}

// NEW
void jumpToMarker(const string &name) {
    auto marker = findMarker(name);
    double time = marker.time / composition.getFps();
    composition.setTime(time);
}
```

#### 5. Documentation (NOT DONE) ❌

**Files to update:**
- `README.md`
- `README_EN.md`
- Create: `docs/TIME_API_REFERENCE.md`
- Create: `docs/MIGRATION_FROM_V1.md`

**README changes:**
```markdown
# ofxAEPlayer v2.0

Time-based After Effects composition player for openFrameworks.

## Quick Start

\`\`\`cpp
ofxAEComposition composition;
composition.load("comp.json");

void update() {
    double time = ofGetElapsedTimef();
    double duration = composition.getDuration();
    composition.setTime(fmod(time, duration));
    composition.update();
}
\`\`\`

## Time API

All operations use time in seconds:
- `setTime(double seconds)` - Set playback time
- `getTime()` - Get current time
- `getDuration()` - Get duration in seconds
- `getFps()` - Get frame rate (for reference only)
```

---

## 📋 Implementation Checklist

### Core Architecture (Completed by AI)
- [x] Remove Frame API from TimePropertyBase
- [x] Remove Frame API from TimeProperty classes
- [x] Update Composition to Time API
- [x] Update Layer to Time API with time support
- [x] Update config to v2.0

### Application Layer (Your Responsibility)
- [ ] Update all Source classes to Time API
- [ ] Migrate or bridge old Property system
- [ ] Update ofxAEPlayer wrapper to Time API
- [ ] Update all examples to use Time API
- [ ] Update README files
- [ ] Create API reference documentation
- [ ] Create migration guide

---

## 🔧 Testing Strategy

### After completing remaining work:

1. **Compilation Test**
   ```bash
   cd example
   make
   ```

2. **Runtime Test**
   - Test basic playback
   - Test seeking forward/backward
   - Test looping
   - Test markers (if used)

3. **Property Animation Test**
   - Verify transforms animate correctly
   - Verify opacity changes work
   - Verify all property types evaluate properly

4. **Edge Cases**
   - Time = 0.0
   - Time = duration
   - Time > duration
   - Negative time (should clamp)
   - Very small time increments

---

## 📝 API Reference (Core Classes)

### Composition
```cpp
class Composition {
public:
    // Time control
    bool setTime(double time);
    double getTime() const;
    double getDuration() const;
    double getFps() const;
    
    // Convenience
    float getTimeFloat() const;
    
    // Legacy info access
    const Info& getInfo() const;
};
```

### Layer
```cpp
class Layer {
public:
    // Time control
    bool setTime(double time);
    double getTime() const;
    
    // Time info
    double getInPoint() const;
    double getOutPoint() const;
    double getDuration() const;
    bool isActiveAtTime(double time) const;
};
```

### TimeProperty<T>
```cpp
template<typename T>
class TimeProperty {
public:
    bool setTime(double time) override;
    double getTime() const override;
    void setFps(double fps) override;
    
    const T& get() const;
    bool hasAnimation() const override;
};
```

---

## 🚨 Breaking Changes in v2.0

### Removed APIs
- `Composition::setFrame(float)`
- `Composition::getCurrentFrame()`
- `Layer::setFrame(float)`
- `Layer::setFrame(int)`
- `Layer::isActiveAtFrame(int)`
- `TimePropertyBase::setFrame(float)`
- All frame-based conversion utilities

### Changed Behavior
- All time values now in seconds (double precision)
- No automatic frame rounding
- FPS is reference only, not used for time calculations

### Migration Required
- Update all `setFrame()` calls to `setTime()`
- Convert frame numbers to time: `time = frame / fps`
- Update marker handling to use time values
- Update animation scrubbing to use time

---

## ✅ Git History

```bash
# Branch: feature/time-api-v2.0

# Commit 1: Phase 2 completion
feat: complete Phase 2 - Internal Time Support implementation

# Commit 2: Core architecture
WIP: Remove Frame API and implement Layer time support (Tasks 3.1-3.2)
- Removed Frame API from TimePropertyBase and all TimeProperty classes
- Updated Composition to Time API
- Implemented Layer time support with parent_fps tracking
- Updated ofxAEConfig.h to v2.0.0
```

---

## 🎯 Next Steps for You

1. **Immediate (Critical Path):**
   - Update Source classes (required for basic functionality)
   - Test with simplest example

2. **High Priority:**
   - Update ofxAEPlayer wrapper (if used)
   - Update at least one example to verify

3. **Medium Priority:**
   - Update all examples
   - Update documentation

4. **Optional:**
   - Full Property system migration (Option B above)
   - Performance optimization
   - Additional time utilities

---

## 📞 Support

If you encounter issues during migration:

1. Check if Source classes are returning valid time/duration
2. Verify FPS is set correctly in Layer (parent_fps_)
3. Ensure time values are always non-negative
4. Check for TIME_EPSILON comparisons (1e-6 seconds)

Good luck with the remaining implementation!