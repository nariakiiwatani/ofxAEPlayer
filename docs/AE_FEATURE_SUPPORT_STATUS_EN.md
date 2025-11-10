# After Effects Feature Support Status

This document outlines which After Effects features are reproduced when a composition is exported and played back.

## Legend

- ✅ **Supported**: Fully reproduced
- ⚠️ **Partial Support**: Only some features are reproduced
- ❌ **Unsupported**: Not reproduced

---

## 1. Layers

### 1.1 Layer Types

| Property | Support Status | Notes |
|----------|---------------|-------|
| Null Object | ✅ | |
| Shape Layer | ✅ | |
| Solid | ✅ | |
| Still Image | ✅ | |
| Video | ⚠️ | Tends to have issues, not recommended |
| Image Sequence | ✅ | |
| Pre-composition | ✅ | |
| Text | ❌ | |
| Camera | ❌ | |
| Light | ❌ | |
| Adjustment Layer | ❌ | |

### 1.2 Layer Properties

#### Transform

| Property | Support Status | Notes |
|----------|---------------|-------|
| Anchor Point | ✅ | |
| Position | ✅ | |
| Scale | ✅ | |
| Rotation | ✅ | Z rotation only |
| Opacity | ✅ | |
| X Rotation | ❌ | 3D features not supported |
| Y Rotation | ❌ | 3D features not supported |
| Orientation | ❌ | 3D features not supported |

#### Layer Settings

| Feature | Support Status | Notes |
|---------|---------------|-------|
| Parent-Child | ✅ | |
| In/Out Points | ✅ | |
| Visibility | ✅ | |
| Layer Offset | ✅ | Position on timeline |
| 3D Layer | ❌ | |

#### Blend Modes (Layer)

| Blend Mode | Support Status |
|------------|---------------|
| Normal | ✅ |
| Add | ✅ |
| Subtract | ✅ |
| Multiply | ✅ |
| Screen | ✅ |
| Lighten | ✅ |
| Darken | ✅ |
| Dissolve | ❌ |
| Dancing Dissolve | ❌ |
| Color Burn | ❌ |
| Linear Burn | ❌ |
| Darker Color | ❌ |
| Color Dodge | ❌ |
| Linear Dodge (Add) | ❌ |
| Lighter Color | ❌ |
| Overlay | ❌ |
| Soft Light | ❌ |
| Hard Light | ❌ |
| Vivid Light | ❌ |
| Linear Light | ❌ |
| Pin Light | ❌ |
| Hard Mix | ❌ |
| Difference | ❌ |
| Exclusion | ❌ |
| Divide | ❌ |
| Hue | ❌ |
| Saturation | ❌ |
| Color | ❌ |
| Luminosity | ❌ |

#### Track Matte

| Matte Type | Support Status |
|------------|---------------|
| Alpha Matte | ✅ |
| Alpha Inverted Matte | ✅ |
| Luma Matte | ✅ |
| Luma Inverted Matte | ✅ |

---

## 2. Masks

| Feature | Support Status |
|---------|---------------|
| Inverted | ✅ |
| Mask Path | ✅ |
| Mask Feather | ❌ |
| Mask Opacity | ✅ |
| Mask Expansion | ❌ |

### Mask Modes

| Mode | Support Status | Notes |
|------|---------------|-------|
| Add | ✅ | |
| Subtract | ✅ | |
| Intersect | ⚠️ | Approximated with multiply |
| Lighten | ✅ | |
| Darken | ✅ | |
| Difference | ❌ | |

---

## 3. Shape Layers

### 3.1 Shape Types

| Shape | Support Status |
|-------|---------------|
| Group | ✅ |
| Rectangle | ✅ |
| Ellipse | ✅ |
| Polygon | ✅ |
| Path | ✅ |

### 3.2 Shape Properties

#### Ellipse

| Property | Support Status |
|----------|---------------|
| Size | ✅ |
| Position | ✅ |

#### Rectangle

| Property | Support Status |
|----------|---------------|
| Size | ✅ |
| Position | ✅ |
| Roundness | ❌ |

#### Polygon/Star

| Property | Support Status |
|----------|---------------|
| Type (Polygon/Star) | ✅ |
| Points | ✅ |
| Position | ✅ |
| Rotation | ✅ |
| Outer Radius | ✅ |
| Inner Radius | ✅ |
| Outer Roundness | ❌ |
| Inner Roundness | ❌ |

#### Path

| Property | Support Status |
|----------|---------------|
| Path Shape | ✅ |
| Bezier Handles | ✅ |

### 3.3 Fill and Stroke

#### Fill

| Property | Support Status | Notes |
|----------|---------------|-------|
| Blend Mode | ⚠️ | Basic modes only |
| Composite | ✅ | |
| Fill Rule | ✅ | |
| Color | ✅ | |
| Opacity | ✅ | |

#### Stroke

| Property | Support Status | Notes |
|----------|---------------|-------|
| Blend Mode | ⚠️ | Basic modes only |
| Composite | ✅ | |
| Color | ✅ | |
| Opacity | ✅ | |
| Stroke Width | ✅ | Limited values (7 or less?) |
| Line Cap | ❌ | |
| Line Join | ❌ | |
| Miter Limit | ❌ | |
| Dashes | ❌ | |
| Taper | ❌ |
| Wave | ❌ |

---

## 4. Animation

### 4.1 Keyframes

| Feature | Support Status | Notes |
|---------|---------------|-------|
| Linear Interpolation | ✅ | |
| Bezier Interpolation | ✅ | |
| Hold Interpolation | ✅ | |
| Easy Ease | ✅ | Processed as bezier |
| Spatial Interpolation | ⚠️ | Partially supported |
| Roving | ❌ | |
| Continuous Bezier | ❌ | |

### 4.2 Expressions

| Feature | Support Status | Notes |
|---------|---------------|-------|
| Expressions | ⚠️ | Baked to all frames during export |

**Important**: Expressions are evaluated during export and converted to keyframes. They are not calculated dynamically during playback.

### 4.3 Time Remapping

| Feature | Support Status |
|---------|---------------|
| Time Remapping | ✅ |

---

## 5. Markers

| Feature | Support Status | Notes |
|---------|---------------|-------|
| Composition Markers | ✅ | |
| Layer Markers | ✅ | |
| Marker Comments | ✅ | |
| Marker Duration | ✅ | |

---

## 6. Effects

| Feature | Support Status |
|---------|---------------|
| All Effects | ❌ |

**Note**: All After Effects effects are unsupported. If you need to use effects, pre-render them as video or image sequences.

---

## 7. Layer Styles

| Feature | Support Status |
|---------|---------------|
| All Layer Styles | ❌ |

---

## 8. Supported Footage File Formats

### 8.1 Images

| Format | Support Status | Notes |
|--------|---------------|-------|
| PNG | ✅ | |
| JPEG | ✅ | |
| PSD | ✅ | Converted to PNG |
| AI | ✅ | Converted to PNG |

### 8.2 Video

| Format | Support Status | Notes |
|--------|---------------|-------|
| Common video formats | ⚠️ | Tends to have issues, not recommended |
| Image Sequence | ✅ | |

---