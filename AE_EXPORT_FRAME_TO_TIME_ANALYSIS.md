# AE Export Script: Frame-to-Time Conversion Analysis

## Overview
Complete analysis of all frame-based conversions in `tools/ExportComposition.jsx` that need to be converted to time-based exports.

---

## 1. EXPORT MODE CONFIGURATION (Lines 18-27)

### Current Code:
```javascript
var EXPORT_MODE = {
    FRAME_BASED: "frame",
    TIME_BASED: "time"
};
var currentExportMode = EXPORT_MODE.FRAME_BASED;
```

### Action Required:
**REMOVE ENTIRELY** - No longer need export mode selection, time-only export.

---

## 2. UI CHECKBOX (Lines 813-821)

### Current Code:
```javascript
var timeBasedExportCheck = exportModeGroup.add("checkbox", undefined, "時間ベースでキーフレームを書き出し（Phase 2新機能）");
timeBasedExportCheck.name = "timeBasedExportCheck";
timeBasedExportCheck.value = false;
```

### Action Required:
**REMOVE ENTIRELY** - No UI selection needed, always time-based.

---

## 3. EXPORT MODE ASSIGNMENT (Line 889)

### Current Code:
```javascript
currentExportMode = timeBasedExportCheck.value ? EXPORT_MODE.TIME_BASED : EXPORT_MODE.FRAME_BASED;
```

### Action Required:
**REMOVE** - No mode selection logic needed.

---

## 4. timeRemapToFrames FUNCTION (Lines 420-425)

### Current Code:
```javascript
function timeRemapToFrames(value, fps) {
    if (typeof value === 'number') {
        return Math.round(value * fps);
    }
    return value;
}
```

### Action Required:
**CONVERT TO TIME** - Remove frame conversion:
```javascript
function timeRemapValue(value) {
    // Time Remap is already in seconds, just return it
    if (typeof value === 'number') {
        return value;
    }
    return value;
}
```

### Update References:
- Line 1177: `case "timeRemapToFrames":` → `case "timeRemapValue":`
- Property config line 302: `customProcessor: "timeRemapToFrames"` → `customProcessor: "timeRemapValue"`

---

## 5. extractMarkers FUNCTION (Lines 1470-1490)

### Current Code:
```javascript
function extractMarkers(markers, fps, offsetFrame){
    function toFrame(t){ return Math.round(t * fps); }
    var markerData = [];
    for (var i = 1; i <= markers.numKeys; i++) {
        var time = markers.keyTime(i);
        var markerValue = markers.keyValue(i);
        var frame = toFrame(time) - offsetFrame;
        var lengthFrames = toFrame(markerValue.duration);
        var comment = markerValue.comment ? markerValue.comment.replace(/[\r\n]+/g, '\n') : "";
        
        var markerInfo = {
            frame: frame,
            comment: comment,
            length: lengthFrames
        };
        
        markerData.push(markerInfo);
    }
    return markerData;
}
```

### Action Required:
**CONVERT TO TIME** - Export time directly:
```javascript
function extractMarkers(markers, offsetTime){
    var markerData = [];
    for (var i = 1; i <= markers.numKeys; i++) {
        var time = markers.keyTime(i);
        var markerValue = markers.keyValue(i);
        var adjustedTime = time - offsetTime;
        var duration = markerValue.duration;
        var comment = markerValue.comment ? markerValue.comment.replace(/[\r\n]+/g, '\n') : "";
        
        var markerInfo = {
            time: Number(adjustedTime.toFixed(6)),
            comment: comment,
            duration: Number(duration.toFixed(6))
        };
        
        markerData.push(markerInfo);
    }
    return markerData;
}
```

### Update Call Sites:
- Line 2012: `extractMarkers(comp.markerProperty, fps, startFrame)` → `extractMarkers(comp.markerProperty, comp.workAreaStart)`
- Line 2192: `extractMarkers(layer.marker, fps, offset)` → `extractMarkers(layer.marker, layer.startTime)`

---

## 6. extractKeyframeBasedProperty FUNCTION (Lines 1554-1618)

### Current Code:
```javascript
function extractKeyframeBasedProperty(prop, offset, fps, decimalPlaces, customProcessor) {
    function toFrame(t){ return Math.round(t * fps); }
    var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
    if (nk === 0) return null;

    var keyframes = [];
    for (var i = 1; i <= nk; i++) {
        var keyTime = prop.keyTime(i);
        var keyFrame = toFrame(keyTime) - offset;  // FRAME CONVERSION
        var keyValue = extractValue(prop, keyTime, decimalPlaces, customProcessor, fps);
        
        var keyframeInfo = {
            frame: keyFrame,  // FRAME KEY
            value: keyValue
        };
        // ... interpolation & spatial tangents ...
        keyframes.push(keyframeInfo);
    }
    return keyframes;
}
```

### Action Required:
**REMOVE ENTIRELY** - Replace with time-based version only (keep extractTimeBasedKeyframeProperty).

---

## 7. extractTimeBasedKeyframeProperty FUNCTION (Lines 1620-1684)

### Current Code:
```javascript
function extractTimeBasedKeyframeProperty(prop, offsetTime, fps, decimalPlaces, customProcessor) {
    // Already correct - uses time keys
    var keyframeInfo = {
        time: timeKey,
        value: keyValue
    };
}
```

### Action Required:
**KEEP AS-IS** - This is already correct, just rename to `extractKeyframeProperty` (drop "TimeBased" prefix since it's the only mode).

---

## 8. extractPropertyValue FUNCTION (Lines 1752-1811)

### Current Code:
```javascript
function extractPropertyValue(prop, options, layer, offset, config) {
    var fps = layer.containingComp.frameRate;
    function toFrame(t){ return Math.round(t * fps); }
    // ...
    if (hasExpression) {
        var layerInPoint = toFrame(layer.inPoint);  // FRAME CONVERSION
        var layerOutPoint = toFrame(layer.outPoint); // FRAME CONVERSION
        var duration = layerOutPoint - layerInPoint; // FRAME DURATION
        result = extractAnimatedProperty(prop, offset, duration, fps, DEC, customProcessor);
    } else if (nk > 0) {
        var start = toFrame(prop.keyTime(1));  // FRAME CONVERSION
        var end = toFrame(prop.keyTime(nk));   // FRAME CONVERSION
        var duration = end - start;            // FRAME DURATION
        var shouldBake = options.useFullFrameAnimation || hasExpression;
        if (shouldBake) {
            result = extractAnimatedProperty(prop, offset, duration, fps, DEC, customProcessor);
        } else {
            // Mode selection
            if (currentExportMode === EXPORT_MODE.TIME_BASED) {
                var offsetTime = layer.startTime;
                result = extractTimeBasedKeyframeProperty(prop, offsetTime, fps, DEC, customProcessor);
            } else {
                result = extractKeyframeBasedProperty(prop, offset, fps, DEC, customProcessor);
            }
        }
    }
}
```

### Action Required:
**CONVERT TO TIME** - Remove all frame conversions and mode selection:
```javascript
function extractPropertyValue(prop, options, layer, config) {
    var fps = layer.containingComp.frameRate;
    var DEC = options.decimalPlaces || 4;
    var customProcessor = config ? config.customProcessor : null;
    var offsetTime = layer.startTime;

    var result;
    if (options.keyframes) {
        var hasExpression = /* ... expression check ... */;
        var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
        
        if (hasExpression || options.useFullFrameAnimation) {
            // Baked animation (expression or full frame mode)
            var duration = layer.outPoint - layer.inPoint; // TIME DURATION
            result = extractAnimatedProperty(prop, offsetTime, duration, fps, DEC, customProcessor);
        } else if (nk > 0) {
            // Keyframe-based (time keys)
            result = extractKeyframeProperty(prop, offsetTime, fps, DEC, customProcessor);
        } else {
            return null;
        }
    } else {
        result = extractValue(prop, offsetTime, DEC, customProcessor, fps);
    }
    return result;
}
```

---

## 9. extractAnimatedProperty FUNCTION (Lines 1686-1750)

### Current Code:
```javascript
function extractAnimatedProperty(prop, offset, duration, fps, decimalPlaces, customProcessor) {
    function toTime(f) { return f / fps; }
    
    var animationData = {};
    for (var frame = 0; frame <= duration; frame++) {  // FRAME LOOP
        var time = toTime(frame + offset);              // FRAME TO TIME
        var extractedValue = extractValue(prop, time, decimalPlaces, customProcessor, fps);
        // ...
        saveGroup(animationData, groupStartFrame, groupValues); // FRAME KEY
    }
}
```

### Action Required:
**CONVERT TO TIME** - Loop by time, use time keys:
```javascript
function extractAnimatedProperty(prop, offsetTime, duration, fps, decimalPlaces, customProcessor) {
    var animationData = {};
    var frameInterval = 1.0 / fps;
    var prevValue = null;
    var groupStartTime = null;
    var groupValues = [];
    
    // Loop through time instead of frames
    for (var t = 0; t <= duration; t += frameInterval) {
        var time = t + offsetTime;
        var extractedValue = extractValue(prop, time, decimalPlaces, customProcessor, fps);
        
        var hasChanged = (prevValue === null) || !valuesAreEqual(extractedValue, prevValue);
        
        if (hasChanged) {
            if (groupStartTime === null) {
                groupStartTime = Number(t.toFixed(6));
                groupValues = [];
            } else if (Math.abs(t - (groupStartTime + groupValues.length * frameInterval)) > 1e-6) {
                saveGroup(animationData, groupStartTime, groupValues);
                groupStartTime = Number(t.toFixed(6));
                groupValues = [];
            }
            groupValues.push(extractedValue);
        } else {
            if (groupStartTime !== null) {
                saveGroup(animationData, groupStartTime, groupValues);
                groupStartTime = null;
                groupValues = [];
            }
        }
        prevValue = extractedValue;
    }
    
    if (groupStartTime !== null) {
        saveGroup(animationData, groupStartTime, groupValues);
    }
    
    return animationData;
}
```

---

## 10. saveGroup FUNCTION (Lines 1465-1467)

### Current Code:
```javascript
function saveGroup(dataObj, startFrame, valuesArray){
    if (valuesArray.length>0) dataObj[String(startFrame)] = valuesArray.slice();
}
```

### Action Required:
**KEEP AS-IS** - Works for both frame and time keys (just changes semantic meaning).
Could rename for clarity:
```javascript
function saveGroup(dataObj, startKey, valuesArray){
    if (valuesArray.length>0) dataObj[String(startKey)] = valuesArray.slice();
}
```

---

## 11. COMPOSITION METADATA (Lines 1988-2008)

### Current Code:
```javascript
function toFrame(t, round){ return round ? Math.round(t * fps) : t * fps; }

var compInfo = {};
compInfo["duration"]      = toFrame(comp.duration, true);        // FRAME COUNT
compInfo["fps"]           = fps;
compInfo["width"]         = comp.width;
compInfo["height"]        = comp.height;
var startFrame = toFrame(comp.workAreaStart, true);              // FRAME NUMBER
compInfo["startFrame"]    = startFrame;                          // FRAME NUMBER
compInfo["endFrame"]      = toFrame((comp.workAreaStart + comp.workAreaDuration), true); // FRAME NUMBER
compInfo["layers"]        = [];

// Export mode metadata
if (currentExportMode === EXPORT_MODE.TIME_BASED) {
    compInfo["exportMode"] = "time";
} else {
    compInfo["exportMode"] = "frame";
}
```

### Action Required:
**CONVERT TO TIME** - Export time values:
```javascript
var compInfo = {};
compInfo["version"]       = "2.0";                    // Version 2.0 for time-based API
compInfo["exportMode"]    = "time";                   // Always time-based
compInfo["duration"]      = comp.duration;            // TIME (seconds)
compInfo["fps"]           = fps;                      // Keep for reference
compInfo["width"]         = comp.width;
compInfo["height"]        = comp.height;
compInfo["startTime"]     = comp.workAreaStart;       // TIME (seconds)
compInfo["endTime"]       = comp.workAreaStart + comp.workAreaDuration; // TIME (seconds)
compInfo["layers"]        = [];
```

---

## 12. LAYER TIMING METADATA (Lines 2042-2093)

### Current Code:
```javascript
var offset = toFrame(layer.startTime, true);  // FRAME OFFSET
layerInfo.offset = offset;

// Later in timing extraction:
var inPoint = toFrame(layer.inPoint, true) - offset;   // FRAME NUMBER
var outPoint = toFrame(layer.outPoint, true) - offset; // FRAME NUMBER
var stretch = layer.stretch;
resultData["in"] = inPoint;
resultData["out"] = outPoint;
resultData["stretch"] = stretch;
```

### Action Required:
**CONVERT TO TIME** - Export time values:
```javascript
layerInfo.startTime = layer.startTime;  // TIME (seconds)

// Later in timing extraction:
var inPoint = layer.inPoint - layer.startTime;   // TIME (relative to layer start)
var outPoint = layer.outPoint - layer.startTime; // TIME (relative to layer start)
var stretch = layer.stretch;
resultData["in"] = Number(inPoint.toFixed(6));
resultData["out"] = Number(outPoint.toFixed(6));
resultData["stretch"] = stretch;
```

---

## 13. EXTRACTPROPERTIESRECURSIVE FUNCTION CALLS

### Update Call Signatures:

#### Line 2204 (extractPropertiesRecursive call):
**Before:**
```javascript
var properties = extractPropertiesRecursive(layer, options, layer, offset);
```

**After:**
```javascript
var properties = extractPropertiesRecursive(layer, options, layer);
```

#### Line 2223 (extractPropertiesRecursive call):
**Before:**
```javascript
var keyframes = extractPropertiesRecursive(layer, options, layer, offset);
```

**After:**
```javascript
var keyframes = extractPropertiesRecursive(layer, options, layer);
```

#### Function Signature (Line 1814):
**Before:**
```javascript
function extractPropertiesRecursive(property, options, layer, offset) {
```

**After:**
```javascript
function extractPropertiesRecursive(property, options, layer) {
```

#### Internal Recursive Calls (Lines 1852, 1884):
**Before:**
```javascript
var child = extractPropertiesRecursive(childProp, options, layer, offset);
```

**After:**
```javascript
var child = extractPropertiesRecursive(childProp, options, layer);
```

#### extractPropertyValue Call (Line 1844):
**Before:**
```javascript
result = extractPropertyValue(property, options, layer, offset, config);
```

**After:**
```javascript
result = extractPropertyValue(property, options, layer, config);
```

---

## Summary of Changes

### Files to Modify:
1. **tools/ExportComposition.jsx** - All changes in this file

### Functions to Remove:
1. `EXPORT_MODE` enum (lines 21-24)
2. `currentExportMode` variable (line 27)
3. `timeRemapToFrames` function (lines 420-425)
4. `extractKeyframeBasedProperty` function (lines 1554-1618)
5. Time-based export checkbox UI code (lines 813-821)
6. Export mode assignment logic (line 889)

### Functions to Modify:
1. `extractMarkers` - Convert to time-based (lines 1470-1490)
2. `extractTimeBasedKeyframeProperty` - Rename to `extractKeyframeProperty` (lines 1620-1684)
3. `extractAnimatedProperty` - Convert frame loop to time loop (lines 1686-1750)
4. `extractPropertyValue` - Remove mode selection, always use time (lines 1752-1811)
5. `extractPropertiesRecursive` - Remove offset parameter (line 1814)
6. Composition metadata export - Convert to time values (lines 1988-2008)
7. Layer timing export - Convert to time values (lines 2042-2093)

### Key Conversions:
- **Frame numbers** → **Time in seconds** (6 decimal places)
- **Frame count (duration)** → **Time duration in seconds**
- **Frame offset** → **Time offset in seconds**
- **Frame keys in JSON** → **Time keys in JSON**
- **"frame" field** → **"time" field** in keyframe objects
- **"startFrame"/"endFrame"** → **"startTime"/"endTime"** in composition
- **"offset" (frames)** → **"startTime" (seconds)** in layers

### Metadata Changes:
- Add `"version": "2.0"`
- Always set `"exportMode": "time"`
- Change field names: `duration`→keep as time, `startFrame`→`startTime`, `endFrame`→`endTime`
- Layer fields: `offset`→`startTime`, marker `frame`→`time`, `length`→`duration`

---

## Testing Checklist

After implementing changes, verify:
1. ✅ Composition exports with time-based metadata
2. ✅ Layer timing exported as seconds
3. ✅ Keyframes exported with time keys (not frame numbers)
4. ✅ Markers exported with time values
5. ✅ Expression baking still works (time-based)
6. ✅ Nested compositions handled correctly
7. ✅ All JSON fields use consistent time units
8. ✅ FPS still included for reference
9. ✅ Time Remap property exported as time
10. ✅ No frame-based code paths remaining

---

## Git Commit Message

```
feat(export): convert AE exporter to time-only export

Remove all frame-based conversions from AE export script.
Export all timing values as seconds (double precision):
- Keyframe times (6 decimal places)
- Layer in/out points
- Composition duration and work area
- Marker times and durations
- All property keyframes

Removed:
- EXPORT_MODE enum and currentExportMode variable
- Frame-based export mode UI checkbox
- extractKeyframeBasedProperty function (frame-based)
- timeRemapToFrames function
- All toFrame() conversions in export paths

Changed:
- extractMarkers: Export time instead of frames
- extractAnimatedProperty: Loop by time instead of frames
- extractPropertyValue: Always use time-based extraction
- Composition metadata: startTime/endTime instead of startFrame/endFrame
- Layer metadata: startTime instead of offset
- Keyframe data: "time" field instead of "frame" field

Version bumped to 2.0 (time-only API).
FPS kept in JSON for reference only.

BREAKING CHANGE: All exported JSON files now use time-based format.
Frame-based export mode no longer supported.