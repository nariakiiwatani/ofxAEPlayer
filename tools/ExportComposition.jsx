var scriptFile = File($.fileName);
var scriptFolder = scriptFile.parent;
$.evalFile(File(scriptFolder.fullName + "/json2.js"));

// ===== EFFECT BAKING SYSTEM =====
var EFFECT_BAKING_EXCLUSIONS = {};
var bakedPrecomps = {};
var processedFootageItems = {};

// Object.keys polyfill for ExtendScript compatibility
if (!Object.keys) {
	Object.keys = function(obj) {
		var keys = [];
		for (var key in obj) {
			if (obj.hasOwnProperty(key)) {
				keys.push(key);
			}
		}
		return keys;
	};
}

// ===== PROPERTY MAPPING CONFIGURATION SYSTEM =====

// After Effectsプロパティのマッピング設定 - matchNameごとの完全な処理設定
var PROPERTY_MAPPING_CONFIG = {
    // Layers
    "ADBE AV Layer": {
    },
    "ADBE Vector Layer": {
    },
    "ADBE Text Layer": {
    },
    // Transform系
    "ADBE Transform Group": {
        wrapInObject: "transform"
    },
    "ADBE Anchor Point": {
        wrapInObject: "anchor"
    },
    "ADBE Position": {
        wrapInObject: "position"
    },
    "ADBE Position_0": {
        wrapInObject: "positionX",
        ignored: true
    },
    "ADBE Position_1": {
        wrapInObject: "positionY",
        ignored: true
    },
    "ADBE Position_2": {
        wrapInObject: "positionZ",
        ignored: true
    },
    "ADBE Scale": {
        wrapInObject: "scale"
    },
    "ADBE Orientation": {
        wrapInObject: "orientation",
        ignored: true
    },
    "ADBE Rotate X": {
        wrapInObject: "rotateX",
        ignored: true
    },
    "ADBE Rotate Y": {
        wrapInObject: "rotateY",
        ignored: true
    },
    "ADBE Rotate Z": {
        wrapInObject: "rotateZ"
    },
    "ADBE Opacity": {
        wrapInObject: "opacity"
    },

    "ADBE Mask Parade": {
        wrapInObject: "mask",
        preserveIndexes: true
    },
    "ADBE Mask Atom": {
        wrapInObject: "atom",
        customProcessor: "maskAtom"
    },
    "ADBE Mask Shape": {
        wrapInObject: "shape",
    },
    "ADBE Mask Feather": {
        wrapInObject: "feather",
    },
    "ADBE Mask Opacity": {
        wrapInObject: "opacity",
    },
    "ADBE Mask Offset": {
        wrapInObject: "offset",
    },

    // Vector Layer系
    "ADBE Root Vectors Group": {
        wrapInObject: "shape",
        preserveIndexes: true
    },
    "ADBE Vector Group": {
        merge: {shapeType: "group"},
        preserveIndexes: true
    },
    "ADBE Vector Transform Group": {
        wrapInObject: "transform"
    },
    "ADBE Vector Anchor": {
        wrapInObject: "anchor"
    },
    "ADBE Vector Position": {
        wrapInObject: "position"
    },
    "ADBE Vector Scale": {
        wrapInObject: "scale"
    },
    "ADBE Vector Rotation": {
        wrapInObject: "rotateZ"
    },
    "ADBE Vector Group Opacity": {
        wrapInObject: "opacity"
    },

    "ADBE Vectors Group": {
        wrapInObject: "shape",
        preserveIndexes: true
    },
    "ADBE Vector Shape - Ellipse": {
        merge: {shapeType: "ellipse"}
    },
    "ADBE Vector Shape - Rect": {
        merge: {shapeType: "rectangle"}
    },
    "ADBE Vector Shape - Star": {
        merge: {shapeType: "polygon"}
    },
    "ADBE Vector Shape - Group": {
        merge: {shapeType: "path"}
    },

    "ADBE Vector Shape": {
    },

    "ADBE Vector Stroke Dash 1":   { wrapInObject: "dash1" },
    "ADBE Vector Stroke Gap 1":    { wrapInObject: "gap1" },
    "ADBE Vector Stroke Dash 2":   { wrapInObject: "dash2" },
    "ADBE Vector Stroke Gap 2":    { wrapInObject: "gap2" },
    "ADBE Vector Stroke Dash 3":   { wrapInObject: "dash3" },
    "ADBE Vector Stroke Gap 3":    { wrapInObject: "gap3" },
    "ADBE Vector Stroke Offset":   { wrapInObject: "dashOffset" },

    // テーパー／ウェーブ詳細（Stroke Taper / Wave の子）
    "ADBE Vector Taper Length Units": { wrapInObject: "taperLengthUnits" },
    "ADBE Vector Taper Start Length": { wrapInObject: "taperStartLength" },
    "ADBE Vector Taper End Length":   { wrapInObject: "taperEndLength" },
    "ADBE Vector Taper StartWidthPx": { wrapInObject: "taperStartWidthPx" },
    "ADBE Vector Taper EndWidthPx":   { wrapInObject: "taperEndWidthPx" },
    "ADBE Vector Taper Start Width":  { wrapInObject: "taperStartWidth" },
    "ADBE Vector Taper End Width":    { wrapInObject: "taperEndWidth" },
    "ADBE Vector Taper Start Ease":   { wrapInObject: "taperStartEase" },
    "ADBE Vector Taper End Ease":     { wrapInObject: "taperEndEase" },

    "ADBE Vector Taper Wave Amount":  { wrapInObject: "waveAmount" },
    "ADBE Vector Taper Wave Units":   { wrapInObject: "waveUnits" },
    "ADBE Vector Taper Wavelength":   { wrapInObject: "wavelength" },
    "ADBE Vector Taper Wave Cycles":  { wrapInObject: "waveCycles" },
    "ADBE Vector Taper Wave Phase":   { wrapInObject: "wavePhase" },
    // Shape系
    "ADBE Vector Ellipse Size": {
        wrapInObject: "size"
    },
    "ADBE Vector Ellipse Position": {
        wrapInObject: "position"
    },
    "ADBE Vector Rect Size": {
        wrapInObject: "size"
    },
    "ADBE Vector Rect Position": {
        wrapInObject: "position"
    },
    "ADBE Vector Rect Roundness": {
        wrapInObject: "roundness"
    },
    "ADBE Vector Star Type": {
        wrapInObject: "type"
    },
    "ADBE Vector Star Points": {
        wrapInObject: "points"
    },
    "ADBE Vector Star Position": {
        wrapInObject: "position"
    },
    "ADBE Vector Star Outer Radius": {
        wrapInObject: "outerRadius"
    },
    "ADBE Vector Star Inner Radius": {
        wrapInObject: "innerRadius"
    },
    "ADBE Vector Star Outer Roundess": {
        wrapInObject: "outerRoundess"
    },
    "ADBE Vector Star Inner Roundess": {
        wrapInObject: "innerRoundess"
    },
    "ADBE Vector Star Rotation": {
        wrapInObject: "rotation"
    },
    
    "ADBE Vector Shape Direction": {
        wrapInObject: "direction",
        customProcessor: "windingDirection"
    },
    "ADBE Vector Composite Order": {
        wrapInObject: "compositeOrder"
    },

    // Fill/Stroke系
    "ADBE Vector Graphic - Fill": {
        merge: {shapeType: "fill"}
    },
    "ADBE Vector Fill Rule": {
        wrapInObject: "rule",
        customProcessor: "fillRule"
    },
    "ADBE Vector Fill Color": {
        wrapInObject: "color"
    },
    "ADBE Vector Fill Opacity": {
        wrapInObject: "opacity"
    },
    "ADBE Vector Graphic - Stroke": {
        merge: {shapeType: "stroke"}
    },
    "ADBE Vector Stroke Color": {
        wrapInObject: "color"
    },
    "ADBE Vector Stroke Opacity": {
        wrapInObject: "opacity"
    },
    "ADBE Vector Stroke Width": {
        wrapInObject: "width"
    },
    "ADBE Vector Stroke Line Cap": {
        wrapInObject: "lineCap"
    },
    "ADBE Vector Stroke Line Join": {
        wrapInObject: "lineJoin"
    },
    "ADBE Vector Stroke Miter Limit": {
        wrapInObject: "miterLimit"
    },
    "ADBE Vector Stroke Dashes": {
        wrapInObject: "dashes"
    },
    "ADBE Vector Stroke Taper": {
        wrapInObject: "taper"
    },
    "ADBE Vector Stroke Wave": {
        wrapInObject: "wave"
    },
    "ADBE Vector Filter - Trim": {
        merge: {shapeType: "trim"}
    },
    "ADBE Vector Filter - Trim Start": {
        wrapInObject: "start"
    },
    "ADBE Vector Filter - Trim End": {
        wrapInObject: "end"
    },
    "ADBE Vector Filter - Trim Offset": {
        wrapInObject: "offset"
    },
    "ADBE Vector Blend Mode": {
        wrapInObject: "blendMode",
        customProcessor: "vectorBlendMode"
    },



    // 不要なプロパティを無効化
    "ADBE Audio Group": { ignored: true },
    "ADBE Data Group": { ignored: true },
    "ADBE Layer Overrides": { ignored: true },
    "ADBE Layer Sets": { ignored: true },
    "ADBE Layer Styles": { ignored: true },
    "ADBE Envir Appear in Reflect": { ignored: true },
    "ADBE Plane Options Group": { ignored: true },
    "ADBE Extrsn Options Group": { ignored: true },
    "ADBE Material Options Group": { ignored: true },
    "ADBE Source Options Group": { ignored: true },
    "ADBE Marker": { ignored: true },
    "ADBE Effect Parade": { ignored: true },
    "ADBE Time Remapping": {
        wrapInObject: "timeRemap"
    },
    "ADBE MTrackers": { ignored: true },
    "ADBE Vector Materials Group": { ignored: true },
    "ADBE Vector Skew": { ignored: true },
    "ADBE Vector Skew Axis": { ignored: true }

};

// ===== BLEND MODE MAPPING SYSTEM =====

function createModeMapping(mappings, defaultValue) {
    var map = {};
    for (var i = 0; i < mappings.length; i++) {
        map[mappings[i].key] = mappings[i].value;
    }
    return function(mode) {
        return map.hasOwnProperty(mode) ? map[mode] : defaultValue;
    };
}

function blendingModeToString(mode) {
    var blendingModeMap = createModeMapping([
        {key: BlendingMode.NORMAL, value: "NORMAL"},
        {key: BlendingMode.DISSOLVE, value: "DISSOLVE"},
        {key: BlendingMode.DANCING_DISSOLVE, value: "DANCING_DISSOLVE"},
        {key: BlendingMode.DARKEN, value: "DARKEN"},
        {key: BlendingMode.MULTIPLY, value: "MULTIPLY"},
        {key: BlendingMode.COLOR_BURN, value: "COLOR_BURN"},
        {key: BlendingMode.LINEAR_BURN, value: "LINEAR_BURN"},
        {key: BlendingMode.DARKER_COLOR, value: "DARKER_COLOR"},
        {key: BlendingMode.LIGHTEN, value: "LIGHTEN"},
        {key: BlendingMode.SCREEN, value: "SCREEN"},
        {key: BlendingMode.COLOR_DODGE, value: "COLOR_DODGE"},
        {key: BlendingMode.LINEAR_DODGE, value: "LINEAR_DODGE"},
        {key: BlendingMode.LIGHTER_COLOR, value: "LIGHTER_COLOR"},
        {key: BlendingMode.OVERLAY, value: "OVERLAY"},
        {key: BlendingMode.SOFT_LIGHT, value: "SOFT_LIGHT"},
        {key: BlendingMode.HARD_LIGHT, value: "HARD_LIGHT"},
        {key: BlendingMode.VIVID_LIGHT, value: "VIVID_LIGHT"},
        {key: BlendingMode.LINEAR_LIGHT, value: "LINEAR_LIGHT"},
        {key: BlendingMode.PIN_LIGHT, value: "PIN_LIGHT"},
        {key: BlendingMode.HARD_MIX, value: "HARD_MIX"},
        {key: BlendingMode.DIFFERENCE, value: "DIFFERENCE"},
        {key: BlendingMode.EXCLUSION, value: "EXCLUSION"},
        {key: BlendingMode.SUBTRACT, value: "SUBTRACT"},
        {key: BlendingMode.DIVIDE, value: "DIVIDE"},
        {key: BlendingMode.HUE, value: "HUE"},
        {key: BlendingMode.SATURATION, value: "SATURATION"},
        {key: BlendingMode.COLOR, value: "COLOR"},
        {key: BlendingMode.LUMINOSITY, value: "LUMINOSITY"},
        {key: BlendingMode.ADD, value: "ADD"},
        {key: BlendingMode.CLASSIC_COLOR_DODGE, value: "CLASSIC_COLOR_DODGE"},
        {key: BlendingMode.CLASSIC_COLOR_BURN, value: "CLASSIC_COLOR_BURN"},
        {key: BlendingMode.LIGHTEN_COLOR_DODGE, value: "LIGHTEN_COLOR_DODGE"},
        {key: BlendingMode.LIGHTEN_COLOR_BURN, value: "LIGHTEN_COLOR_BURN"}
    ], "UNKNOWN");
    
    return blendingModeMap(mode);
}

function vectorBlendModeToString(mode) {
    var vectorBlendModeMap = createModeMapping([
        {key: 1, value: "NORMAL"},
        {key: 3, value: "DARKER"},
        {key: 4, value: "MULTIPLY"},
        {key: 5, value: "COLOR_BURN"},
        {key: 6, value: "LINEAR_BURN"},
        {key: 7, value: "DARKER_COLOR"},
        {key: 9, value: "LIGHTER"},
        {key: 10, value: "SCREEN"},
        {key: 11, value: "COLOR_DODGE"},
        {key: 12, value: "LINEAR_DODGE"},
        {key: 13, value: "LIGHTER_COLOR"},
        {key: 15, value: "OVERLAY"},
        {key: 16, value: "SOFT_LIGHT"},
        {key: 17, value: "HARD_LIGHT"},
        {key: 18, value: "LINEAR_LIGHT"},
        {key: 19, value: "VIVID_LIGHT"},
        {key: 20, value: "PIN_LIGHT"},
        {key: 21, value: "HARD_MIX"},
        {key: 23, value: "DIVIDE"},
        {key: 24, value: "EXCLUSION"},
        {key: 26, value: "HUE"},
        {key: 27, value: "SATURATION"},
        {key: 28, value: "COLOR"},
        {key: 29, value: "BRIGHTNESS"}
    ], "NORMAL");
    
    return vectorBlendModeMap(mode);
}

function windingDirectionToString(direction) {
    var map = {};
    map[1] = "DEFAULT";
    map[2] = "COUNTER_CLOCKWISE";
    map[3] = "CLOCKWISE";
    return map.hasOwnProperty(direction) ? map[direction] : "DEFAULT";
}

function fillRuleToString(rule) {
    var map = {};
    map[1] = "NON_ZERO";
    map[2] = "EVEN_ODD";
    return map.hasOwnProperty(rule) ? map[rule] : "NON_ZERO";
}

function maskModeToString(mode) {
    var map = {};
    map[MaskMode.NONE] = "NONE";
    map[MaskMode.ADD] = "ADD";
    map[MaskMode.SUBTRACT] = "SUBTRACT";
    map[MaskMode.INTERSECT] = "INTERSECT";
    map[MaskMode.LIGHTEN] = "LIGHTEN";
    map[MaskMode.DARKEN] = "DARKEN";
    map[MaskMode.DIFFERENCE] = "DIFFERENCE";
    return map.hasOwnProperty(mode) ? map[mode] : "ADD";
}

function keyframeInterpolationTypeToString(type) {
    var map = {};
    map[KeyframeInterpolationType.LINEAR] = "LINEAR";
    map[KeyframeInterpolationType.BEZIER] = "BEZIER";
    map[KeyframeInterpolationType.HOLD] = "HOLD";
    return map.hasOwnProperty(type) ? map[type] : "LINEAR";
}

    // var KEYFRAME_INTERPOLATION_TYPES = {
    //     6612: "LINEAR",        // KeyframeInterpolationType.LINEAR
    //     6613: "BEZIER",        // KeyframeInterpolationType.BEZIER
    //     6614: "HOLD"           // KeyframeInterpolationType.HOLD
    // };

// ===== COMMON UTILITY FUNCTIONS =====

function trackMatteTypeToString(t){
    var map = {};
    map[TrackMatteType.NO_TRACK_MATTE]   = "NO_TRACK_MATTE";
    map[TrackMatteType.ALPHA]            = "ALPHA";
    map[TrackMatteType.ALPHA_INVERTED]   = "ALPHA_INVERTED";
    map[TrackMatteType.LUMA]             = "LUMA";
    map[TrackMatteType.LUMA_INVERTED]    = "LUMA_INVERTED";
    return map.hasOwnProperty(t) ? map[t] : "UNKNOWN";
}

// CRC32 テーブルを生成
function makeCrc32Table() {
    var table = [];
    var poly = 0xEDB88320;
    for (var i = 0; i < 256; i++) {
        var c = i;
        for (var j = 0; j < 8; j++) {
            if (c & 1) {
                c = poly ^ (c >>> 1);
            } else {
                c = c >>> 1;
            }
        }
        table[i] = c >>> 0; // 符号なし32bitに
    }
    return table;
}

var CRC32_TABLE = makeCrc32Table();

(function(me){
    // ===== UTILITY FUNCTIONS (moved inside IIFE for debugLog access) =====
    
    function copyFileWithDateCheck(sourceFile, destFile, fileName, logContext) {
        try {
            var shouldCopy = true;
            if (destFile.exists) {
                var sourceModified = sourceFile.modified;
                var destModified = destFile.modified;
                shouldCopy = sourceModified > destModified;
                if (!shouldCopy) {
                    debugLog("FileCopy", "File already up to date, skipping: " + fileName, null, "verbose");
                    return true; // Success, but no copy needed
                }
            }
            
            if (shouldCopy) {
                sourceFile.copy(destFile);
                debugLog("FileCopy", "File copied: " + fileName, logContext, "verbose");
            }
            return true;
        } catch (e) {
            debugLog("FileCopy", "Error copying file: " + fileName + " - " + e.toString(), logContext, "error");
            return false;
        }
    }

    function safelyProcessLayerProperty(layer, propertyName, processor, errorLevel) {
        errorLevel = errorLevel || "error";
        try {
            return processor();
        } catch (error) {
            var layerName = layer ? layer.name : "unknown";
            var errorMessage = "ERROR: Failed to process " + propertyName + " for layer " + layerName + ": " + error.toString();
            debugLog("LayerProcessing", errorMessage, {
                layerName: layerName,
                propertyName: propertyName,
                error: error.message
            }, errorLevel);
            return null;
        }
    }

    // polyfills
    if (typeof String.prototype.trim !== 'function') {
        String.prototype.trim = function(){ return this.replace(/^\s+|\s+$/g, ''); };
    }
    if (typeof String.prototype.fsSanitized !== 'function') {
        String.prototype.fsSanitized = function(){ return this.replace(/[\\\/\:\*\?\"\<\>\|]/g, "_"); };
    }
    if (typeof Array.prototype.map !== 'function') {
        Array.prototype.map = function(callback, thisArg){
            var T,A,k;
            if (this==null) throw new TypeError(' this is null or not defined');
            var O = Object(this);
            var len = O.length >>> 0;
            if (typeof callback !== 'function') throw new TypeError(callback + ' is not a function');
            if (arguments.length>1) T=thisArg;
            A=new Array(len); k=0;
            while(k<len){
                var kValue, mappedValue;
                if (k in O){
                    kValue = O[k];
                    mappedValue = callback.call(T, kValue, k, O);
                    A[k]=mappedValue;
                }
                k++;
            }
            return A;
        };
    }
    if (typeof Array.prototype.filter !== 'function') {
        Array.prototype.filter = function(callback, thisArg) {
            if (this == null) throw new TypeError(' this is null or not defined');
            if (typeof callback !== 'function') throw new TypeError(callback + ' is not a function');

            var O = Object(this);
            var len = O.length >>> 0;
            var res = [];
            for (var i = 0; i < len; i++) {
                if (i in O) {
                    var val = O[i];
                    if (callback.call(thisArg, val, i, O)) {
                        res.push(val);
                    }
                }
            }
            return res;
        };
    }

    function getFootageFrameRate(footageItem) {
        try {
            if (!(footageItem instanceof FootageItem)) {
                return null;
            }
            
            var mainSource = footageItem.mainSource;
            
            if (mainSource instanceof FileSource) {
                return mainSource.conformFrameRate;
            }
            
            return null;
        } catch (e) {
            debugLog("getFootageFrameRate", "Error getting footage frame rate: " + e.toString(), null, "warning");
            return null;
        }
    }

    function getSourceType(layer) {
        try {
            // Check if layer exists and has a source property
            if (!layer || !layer.source) {
                return "none"; // ソースなし（シェイプレイヤーなど）
            }
            
            if (layer.source instanceof CompItem) {
                return "composition";
            }
            
            // Check for null layer before solid layer
            // Null layers are identified by the nullLayer property
            if (layer.nullLayer) {
                return "null";
            }
            
            if (layer.source.mainSource instanceof SolidSource) {
                return "solid";
            }
            
            if (layer.source.mainSource instanceof FileSource) {
                var fileSource = layer.source.mainSource;
                
                // ExtendScript APIの確実な静止画判定を使用
                if (fileSource.isStill) {
                    return "still";
                }
                
                // FootageSourceのプロパティを使用した確実な判定
                try {
                    var footageSource = layer.source;
                    
                    // 動画・音声の有無を確認
                    var hasVideo = false;
                    var hasAudio = false;
                    
                    try {
                        hasVideo = footageSource.hasVideo;
                        hasAudio = footageSource.hasAudio;
                    } catch (e) {
                        // プロパティにアクセスできない
                        debugLog("getSourceType", "Cannot access footage properties", null, "warning");
                    }
                    
                    var file = fileSource.file;
                    if (file) {
                        if (hasVideo && hasAudio) {
                            // 映像+音声は確実に動画
                            return "video";
                        } else if (!hasVideo && hasAudio) {
                            // 音声のみ
                            return "audio";
                        } else if (hasVideo && !hasAudio) {
                            // 映像のみの場合は拡張子で判定
                            var fileName = file.name;
                            var fileNameLower = fileName.toLowerCase();
                            
                            // 静止画拡張子の場合は画像シーケンスと判定
                            if (fileNameLower.match(/\.(jpg|jpeg|png|tiff|tga|exr|dpx|bmp|gif|psd)$/)) {
                                return "sequence";
                            }
                            
                            // その他の場合は動画
                            return "video";
                        } else {
                            debugLog("getSourceType", "Cannot access footage properties, falling back to filename analysis: " + e.toString(), null, "warning");
                        }
                    }
                } catch (e) {
                    debugLog("getSourceType", "Error in advanced source type detection: " + e.toString(), null, "warning");
                }
                
                return "footage"; // その他のフッテージ
            }
            
            return "unknown";
        } catch (e) {
            debugLog("getSourceType", "Error determining source type: " + e.toString(), {
                layerName: layer ? layer.name : "unknown",
                hasSource: !!(layer && layer.source)
            }, "error");
            return "unknown";
        }
    }

    var SETTINGS_SECTION = "MyScriptSettings";


    // ===== UI =====
    function createDebugPanel(parentGroup) {
        DEBUG_PANEL = parentGroup.add("panel", undefined, "Debug Log");
        DEBUG_PANEL.orientation = "column";
        DEBUG_PANEL.alignChildren = "fill";
        DEBUG_PANEL.alignment = "fill";
        DEBUG_PANEL.preferredSize.height = 300;
        DEBUG_PANEL.visible = false;
        
        // フィルタリンググループ
        var filterGroup = DEBUG_PANEL.add("group");
        filterGroup.orientation = "column";
        filterGroup.alignChildren = "fill";
        filterGroup.alignment = "fill";
        
        // ログレベルフィルタ
        var levelFilterGroup = filterGroup.add("group");
        levelFilterGroup.orientation = "row";
        levelFilterGroup.alignment = "fill";
        
        var levelLabel = levelFilterGroup.add("statictext", undefined, "レベル:");
        levelLabel.preferredSize.width = 50;
        
        DEBUG_LEVEL_FILTER_DROPDOWN = levelFilterGroup.add("dropdownlist", undefined, ["VERBOSE", "NOTICE", "WARNING", "ERROR", "FATAL"]);
        DEBUG_LEVEL_FILTER_DROPDOWN.selection = 0; // デフォルトはVERBOSE
        DEBUG_LEVEL_FILTER_DROPDOWN.preferredSize.width = 100;
        
        // テキストフィルタ
        var textFilterGroup = filterGroup.add("group");
        textFilterGroup.orientation = "row";
        textFilterGroup.alignment = "fill";
        
        var textLabel = textFilterGroup.add("statictext", undefined, "検索:");
        textLabel.preferredSize.width = 50;
        
        DEBUG_TEXT_FILTER_INPUT = textFilterGroup.add("edittext", undefined, "");
        DEBUG_TEXT_FILTER_INPUT.alignment = "fill";
        
        // ログリスト
        DEBUG_LIST = DEBUG_PANEL.add("listbox");
        DEBUG_LIST.alignment = "fill";
        DEBUG_LIST.preferredSize.height = 180;
        
        // ダブルクリックでログ詳細表示
        DEBUG_LIST.onDoubleClick = function() {
            var selectedIndex = this.selection ? this.selection.index : -1;
            if (selectedIndex >= 0) {
                // フィルタリングされたログから対応するエントリを取得
                var filteredLogs = DEBUG_LOGS.filter(function(logEntry) {
                    var currentLevelIndex = LOG_LEVELS[DEBUG_FILTER_LEVEL].index;
                    var logLevelIndex = LOG_LEVELS[logEntry.level].index;
                    if (logLevelIndex < currentLevelIndex) {
                        return false;
                    }
                    
                    if (DEBUG_FILTER_TEXT && DEBUG_FILTER_TEXT.trim() !== "") {
                        var searchText = DEBUG_FILTER_TEXT.toLowerCase();
                        var fullText = logEntry.fullText.toLowerCase();
                        if (fullText.indexOf(searchText) === -1) {
                            return false;
                        }
                    }
                    
                    return true;
                });
                
                // リストは時系列順（下が最新）なので、インデックスはそのまま使用
                var logIndex = selectedIndex;
                if (logIndex >= 0 && logIndex < filteredLogs.length) {
                    showLogDetailDialog(filteredLogs[logIndex]);
                }
            }
        };
        
        // 使用説明
        var instructionGroup = DEBUG_PANEL.add("group");
        instructionGroup.orientation = "row";
        instructionGroup.alignment = "center";
        
        var instructionText = instructionGroup.add("statictext", undefined, "ダブルクリックでログ詳細表示・コピー可能");
        instructionText.graphics.font = ScriptUI.newFont(instructionText.graphics.font.name, ScriptUI.FontStyle.ITALIC, 10);
        
        // コントロールボタン
        var debugControlGroup = DEBUG_PANEL.add("group");
        debugControlGroup.orientation = "row";
        debugControlGroup.alignment = "fill";
        
        var clearLogButton = debugControlGroup.add("button", undefined, "ログクリア");
        clearLogButton.alignment = "center";
        
        DEBUG_SAVE_BUTTON = debugControlGroup.add("button", undefined, "ファイルに保存");
        DEBUG_SAVE_BUTTON.alignment = "center";
        
        clearLogButton.onClick = function() {
            clearDebugLogs();
        };
        
        DEBUG_SAVE_BUTTON.onClick = function() {
            saveDebugLogs();
        };
        
        // フィルタ変更時の処理
        DEBUG_LEVEL_FILTER_DROPDOWN.onChange = function() {
            var selectedIndex = this.selection.index;
            var levels = ["verbose", "notice", "warning", "error", "fatal"];
            DEBUG_FILTER_LEVEL = levels[selectedIndex];
            updateDebugDisplay();
        };
        
        DEBUG_TEXT_FILTER_INPUT.onChange = function() {
            DEBUG_FILTER_TEXT = this.text;
            updateDebugDisplay();
        };
        
        return DEBUG_PANEL;
    }

    function createUI(myPanel){
        if (!myPanel){ alert("このスクリプトはスクリプトUIパネルとして実行してください。"); return; }

        myPanel.grp = myPanel.add("group");
        myPanel.grp.orientation = "column";
        myPanel.grp.alignChildren = "fill";
        myPanel.grp.alignment = "fill";
        myPanel.grp.spacing = 10;


        // 1. 書出しパス
        var outputLabelGroup = myPanel.grp.add("group");
        outputLabelGroup.orientation = "row";
        outputLabelGroup.alignment = "fill";
        
        var outputLabel = outputLabelGroup.add("statictext", undefined, "出力先:");
        outputLabel.alignment = "left";

        var outputGroup = myPanel.grp.add("group");
        outputGroup.orientation = "row";
        outputGroup.alignment = "fill";

        var outputPathText = outputGroup.add("edittext");
        outputPathText.name = "outputPathText";
        outputPathText.text = "";
        outputPathText.alignment = "fill";

        var selectOutputFolderButton = outputGroup.add("button", undefined, "選択");
        selectOutputFolderButton.alignment = "right";

        // 2. 共有アセット関連
        var assetGroup = myPanel.grp.add("group");
        assetGroup.orientation = "column";
        assetGroup.alignment = "fill";

        // 共有アセット使用チェックボックス
        var sharedAssetsGroup = assetGroup.add("group");
        sharedAssetsGroup.orientation = "row";
        sharedAssetsGroup.alignment = "fill";

        var sharedAssetsCheck = sharedAssetsGroup.add("checkbox", undefined, "共有アセットフォルダを使用");
        sharedAssetsCheck.name = "sharedAssetsCheck";
        sharedAssetsCheck.value = false;
        sharedAssetsCheck.alignment = "left";

        // 共有アセットパス設定（インデント）
        var sharedAssetsPathGroup = assetGroup.add("group");
        sharedAssetsPathGroup.orientation = "row";
        sharedAssetsPathGroup.alignment = "fill";

        var indentSpacer = sharedAssetsPathGroup.add("statictext", undefined, "");
        indentSpacer.preferredSize.width = 20; // インデント用スペース

        var sharedAssetsLabel = sharedAssetsPathGroup.add("statictext", undefined, "共有アセットパス:");
        sharedAssetsLabel.preferredSize.width = 100;

        var sharedAssetsPathText = sharedAssetsPathGroup.add("edittext");
        sharedAssetsPathText.name = "sharedAssetsPathText";
        sharedAssetsPathText.text = "shared_assets";
        sharedAssetsPathText.alignment = "fill";

        var selectSharedAssetsFolderButton = sharedAssetsPathGroup.add("button", undefined, "選択");
        selectSharedAssetsFolderButton.alignment = "right";

        // 3. 小数桁（左揃え）
        var decimalPlacesGroup = myPanel.grp.add("group");
        decimalPlacesGroup.orientation = "row";
        decimalPlacesGroup.alignment = "fill";

        var decLabel = decimalPlacesGroup.add("statictext", undefined, "小数桁:");
        var decimalPlacesText = decimalPlacesGroup.add("edittext", undefined, "4");
        decimalPlacesText.name = "decimalPlacesText";
        decimalPlacesText.preferredSize.width = 40;

        // 4. フルフレーム関連
        var optionGroup = myPanel.grp.add("group");
        optionGroup.orientation = "column";
        optionGroup.alignment = "fill";

        var useFullFrameAnimationCheck = optionGroup.add("checkbox", undefined, "フルフレームアニメーションで書き出し");
        useFullFrameAnimationCheck.name = "useFullFrameAnimationCheck";
        useFullFrameAnimationCheck.value = false;
        useFullFrameAnimationCheck.alignment = "left";

        // ボタングループ
        var buttonGroup = myPanel.grp.add("group");
        buttonGroup.orientation = "row";
        buttonGroup.alignment = "fill";

        var executeButton = buttonGroup.add("button", undefined, "実行");
        executeButton.alignment = "fill";

        
        var debugButton = buttonGroup.add("button", undefined, "Debug");
        debugButton.alignment = "fill";

        
        // デバッグパネルを作成
        var debugPanel = createDebugPanel(myPanel.grp);

        selectOutputFolderButton.onClick = function(){
            var selectedFolder = Folder.selectDialog("出力先のフォルダを選択してください");
            if (selectedFolder){ outputPathText.text = decodeURI(selectedFolder.fsName); saveSettings(myPanel.grp); }
        };
        // UI制御
        sharedAssetsCheck.onClick = function() {
            sharedAssetsPathGroup.enabled = this.value;
            saveSettings(myPanel.grp);
        };
        sharedAssetsPathGroup.enabled = false; // 初期状態は無効

        // 設定を読み込み（onClickハンドラー設定後）
        loadSettings(myPanel.grp);

        selectSharedAssetsFolderButton.onClick = function(){
            var selectedFolder = Folder.selectDialog("共有アセットの保存先フォルダを選択してください");
            if (selectedFolder){
                sharedAssetsPathText.text = decodeURI(selectedFolder.fsName);
                saveSettings(myPanel.grp);
            }
        };

        executeButton.onClick = function(){
            var undoOpen=false;
            try{
                // 実行ボタンを押すたびにログをクリアし、処理済みコンポジション履歴もリセット
                clearDebugLogs();
                resetBakedPrecomps();
                resetProcessedFootage();
                
                var outputFolderPath = outputPathText.text.trim();
                var useSharedAssets = sharedAssetsCheck.value;
                var sharedAssetsPath = sharedAssetsPathText.text.trim();

                var decPlacesText = decimalPlacesText.text;
                var decPlaces = Math.max(0, Math.min(10, parseInt(decPlacesText,10) || 4));

                if (outputFolderPath===''){ alert("出力先のフォルダを指定してください。"); return; }
                if (app.project.activeItem==null || !(app.project.activeItem instanceof CompItem)){ alert("コンポジションを選択してください。"); return; }


                debugLog("ExecuteSystem", "Starting property extraction process (time-based export)", {
                    outputFolder: outputFolderPath,
                    useSharedAssets: useSharedAssets,
                    sharedAssetsPath: sharedAssetsPath,
                    decimalPlaces: decPlaces
                }, "notice");

                app.beginUndoGroup("プロパティ抽出"); undoOpen=true;
                
                var options = {
                    outputFolderPath: outputFolderPath,
                    useSharedAssets: useSharedAssets,
                    sharedAssetsPath: sharedAssetsPath,
                    decimalPlaces: decPlaces,
                    useFullFrameAnimation: useFullFrameAnimationCheck.value
                };
                extractPropertiesForAllLayers(app.project.activeItem, options);

                saveSettings(myPanel.grp);
                debugLog("ExecuteSystem", "Property extraction completed successfully", null, "notice");
                alert("全てのレイヤーのプロパティを抽出し、指定されたフォルダに保存しました。");
            }catch(e){
                debugLog("ExecuteSystem", "Error during execution: " + e.message, e, "error");
                alert("エラーが発生しました: " + e.message);
            }finally{
                if (undoOpen) {
                    app.endUndoGroup();
                    app.executeCommand(16); // undo
                }
            }
        };

        
        debugButton.onClick = function(){
            if (debugPanel) {
                debugPanel.visible = !debugPanel.visible;
                myPanel.layout.layout(true);
            }
        };

        addChangeEventListeners(myPanel.grp);
        myPanel.onResizing = myPanel.onResize = function(){ this.layout.resize(); };
        myPanel.layout.layout(true);
        
        // デバッグシステムの初期化
        debugLog("UISystem", "User interface initialized successfully", null, "notice");
    }

    // ===== UI helpers =====

    // ===== settings helpers =====
    function traverseUIElements(parentGroup, callback) {
        function traverse(group) {
            for (var i = 0; i < group.children.length; i++) {
                var child = group.children[i];
                callback(child, parentGroup);
                if (child.children && child.children.length > 0) {
                    traverse(child);
                }
            }
        }
        traverse(parentGroup);
    }
    
    function saveSettings(parentGroup){
        traverseUIElements(parentGroup, function(child) {
            if (child.name){
                var value = null;
                if (child instanceof EditText) value = child.text;
                else if (child instanceof Checkbox) value = child.value;
                else if (child instanceof DropDownList) value = child.selection ? child.selection.index : -1;
                if (value !== null) app.settings.saveSetting(SETTINGS_SECTION, child.name, value.toString());
            }
        });
    }
    
    function loadSettings(parentGroup){
        var sharedAssetsCheck = null;
        var sharedAssetsPathGroup = null;
        
        // 設定値を読み込み
        traverseUIElements(parentGroup, function(child) {
            if (child.name && app.settings.haveSetting(SETTINGS_SECTION, child.name)){
                var value = app.settings.getSetting(SETTINGS_SECTION, child.name);
                if (child instanceof EditText) child.text = value;
                else if (child instanceof Checkbox) child.value = (value === 'true');
                else if (child instanceof DropDownList){
                    var index = parseInt(value, 10);
                    if (!isNaN(index) && index >= 0 && index < child.items.length) child.selection = index;
                }
            }
            
            // UI要素の参照を取得
            if (child.name === 'sharedAssetsCheck') {
                sharedAssetsCheck = child;
            }
        });
        
        // sharedAssetsPathGroupを探す
        traverseUIElements(parentGroup, function(child) {
            if (child.children && child.children.length > 0) {
                for (var i = 0; i < child.children.length; i++) {
                    var grandChild = child.children[i];
                    if (grandChild.name === 'sharedAssetsPathText') {
                        sharedAssetsPathGroup = child;
                        break;
                    }
                }
            }
        });
        
        // 設定読み込み後にUI連動を更新
        if (sharedAssetsCheck && sharedAssetsPathGroup) {
            sharedAssetsPathGroup.enabled = sharedAssetsCheck.value;
        }
    }
    
    function addChangeEventListeners(parentGroup){
        traverseUIElements(parentGroup, function(child, parentGroup) {
            if (child.name){
                if (child instanceof EditText) {
                    (function(c){ c.onChange = function(){ saveSettings(parentGroup); }; })(child);
                } else if (child instanceof Checkbox) {
                    // sharedAssetsCheckは既に専用のonClickハンドラーがあるのでスキップ
                    if (child.name !== 'sharedAssetsCheck') {
                        (function(c){ c.onClick = function(){ saveSettings(parentGroup); }; })(child);
                    }
                } else if (child instanceof DropDownList) {
                    (function(c){ c.onChange = function(){ saveSettings(parentGroup); }; })(child);
                }
            }
        });
    }
    

    // ===== path util =====
    function getRelativePath(fromPath, toPath){
        var fromFsName = fromPath.fsName.replace(/\\/g,'/');
        var toFsName   = toPath.fsName.replace(/\\/g,'/');
        var fromParts = fromFsName.split('/'), toParts = toFsName.split('/');
        // 大文字小文字の比較用にコピーを作成
        var fromPartsLower = [];
        var toPartsLower = [];
        for (var i=0;i<fromParts.length;i++) fromPartsLower[i]=fromParts[i].toLowerCase();
        for (var j=0;j<toParts.length;j++) toPartsLower[j]=toParts[j].toLowerCase();
        var length = Math.min(fromParts.length, toParts.length);
        var samePartsLength = length;
        for (var k=0;k<length;k++){ if (fromPartsLower[k]!=toPartsLower[k]){ samePartsLength=k; break; } }
        var outputParts=[];
        for (var a=samePartsLength; a<fromParts.length; a++) outputParts.push("..");
        // 元の大文字小文字を保持したパスを使用
        for (var b=samePartsLength; b<toParts.length; b++) outputParts.push(toParts[b]);
        return outputParts.join("/");
    }

    function layerUniqueName(layer){ return layer.name.fsSanitized() + "(ID_" + layer.id + ")"; }

    function determineAssetFolderPath(options, outputFolderPath, compName) {
        if (options.useSharedAssets) {
            var sharedPath = options.sharedAssetsPath || "shared_assets";
            
            // 絶対パス判定 (Windows: C:\ or C:/, Unix: /)
            if (sharedPath.match(/^([A-Za-z]:[\\\/]|[\\\/])/)) {
                return sharedPath;
            } else {
                return outputFolderPath + "/" + sharedPath;
            }
        } else {
            return outputFolderPath + "/" + compName + "/assets";
        }
    }

    // ===== VALUE PROCESSING SYSTEM =====
    
    // プロパティの有効状態を判定
    function isPropertyEnabled(config) {
        return (config.ignored !== undefined) ? !config.ignored : true;
    }
    
    // キーフレーム補間情報を抽出
    function extractKeyframeInterpolation(prop, keyIndex) {
        try {
            var interpolation = {
                inType: "LINEAR",
                outType: "LINEAR",
                temporalEase: null,
                roving: false,
                continuous: false
            };
            
            // 補間タイプを取得
            try {
                var inType = prop.keyInInterpolationType(keyIndex);
                var outType = prop.keyOutInterpolationType(keyIndex);
                interpolation.inType = keyframeInterpolationTypeToString(inType);
                interpolation.outType = keyframeInterpolationTypeToString(outType);
            } catch (e) {
                debugLog("extractKeyframeInterpolation", "Error getting interpolation type: " + e.toString(), null, "warning");
            }
            
            // 時間イージング情報を取得
            try {
                var inEase = prop.keyInTemporalEase(keyIndex);
                var outEase = prop.keyOutTemporalEase(keyIndex);
                if (inEase && outEase) {
                    interpolation.temporalEase = {
                        inEase: {
                            speed: inEase[0] ? inEase[0].speed : 0,
                            influence: inEase[0] ? inEase[0].influence : 0
                        },
                        outEase: {
                            speed: outEase[0] ? outEase[0].speed : 0,
                            influence: outEase[0] ? outEase[0].influence : 0
                        }
                    };
                }
            } catch (e) {
                debugLog("extractKeyframeInterpolation", "Error getting temporal ease: " + e.toString(), null, "warning");
            }
            
            // ロービングとコンティニュアス
            try {
                interpolation.roving = prop.keyRoving ? prop.keyRoving(keyIndex) : false;
                interpolation.continuous = prop.keyContinuous ? prop.keyContinuous(keyIndex) : false;
            } catch (e) {
                debugLog("extractKeyframeInterpolation", "Error getting roving/continuous: " + e.toString(), null, "warning");
            }
            
            return interpolation;
        } catch (e) {
            debugLog("extractKeyframeInterpolation", "Error in keyframe interpolation extraction: " + e.toString(), null, "error");
            return null;
        }
    }
    
    // 空間タンジェント情報を抽出（Position/Pathプロパティ用）
    function extractSpatialTangents(prop, keyIndex, decimalPlaces) {
        try {
            var tangents = null;
            
            // 空間補間が可能なプロパティかチェック
            if (prop.isSpatial) {
                try {
                    var inTangent = prop.keyInSpatialTangent(keyIndex);
                    var outTangent = prop.keyOutSpatialTangent(keyIndex);
                    
                    if (inTangent && outTangent) {
                        tangents = {
                            inTangent: [
                                parseFloat(Number(inTangent[0]).toFixed(decimalPlaces)),
                                parseFloat(Number(inTangent[1]).toFixed(decimalPlaces))
                            ],
                            outTangent: [
                                parseFloat(Number(outTangent[0]).toFixed(decimalPlaces)),
                                parseFloat(Number(outTangent[1]).toFixed(decimalPlaces))
                            ]
                        };
                    }
                } catch (e) {
                    debugLog("extractSpatialTangents", "Error getting spatial tangents: " + e.toString(), null, "warning");
                }
            }
            
            return tangents;
        } catch (e) {
            debugLog("extractSpatialTangents", "Error in spatial tangent extraction: " + e.toString(), null, "error");
            return null;
        }
    }
    
    // 型自動判定による値処理
    function extractValue(prop, time, decimalPlaces, customProcessor, fps) {
        try {
            if (!prop) return null;
            
            var value = prop.valueAtTime(time, false);
            
            if (customProcessor) {
                switch(customProcessor) {
                    case "vectorBlendMode":
                        return vectorBlendModeToString(value);
                    case "windingDirection":
                        return windingDirectionToString(value);
                    case "fillRule":
                        return fillRuleToString(value);
                    default:
                        break;
                }
            }
            
            // 型自動判定
            if (typeof value === 'number') {
                // Float値
                return parseFloat(Number(value).toFixed(decimalPlaces || 4));
            } else if (value instanceof Array) {
                // Array値
                return value.map(function(val) {
                    return parseFloat(Number(val).toFixed(decimalPlaces || 4));
                });
            } else if (value && value.vertices) {
                // PathValue
                return extractPathValue(value, decimalPlaces || 4);
            }
            
            return value;
        } catch (e) {
            debugLog("extractValue", "Error extracting value: " + e.toString(), { prop: prop, time: time }, "error");
            return null;
        }
    }
    
    function valuesAreEqual(a, b) {
        try {
            // 型に応じた比較
            if (typeof a === 'number' && typeof b === 'number') {
                return eqNum(a, b);
            } else if (a instanceof Array && b instanceof Array) {
                return arrayValuesAreEqual(a, b);
            } else if (a && b && a.vertices && b.vertices) {
                return pathValuesAreEqual(a, b);
            }
            
            return a === b;
        } catch (e) {
            debugLog("valuesAreEqualGeneric", "Error comparing values: " + e.toString(), { a: a, b: b }, "error");
            return false;
        }
    }
    
    // デバッグシステム
    var DEBUG_LOGS = [];
    var DEBUG_PANEL = null;
    var DEBUG_LIST = null;
    var DEBUG_FILTER_LEVEL = "verbose"; // verbose, notice, warning, error, fatal
    var DEBUG_FILTER_TEXT = "";
    var DEBUG_LEVEL_FILTER_DROPDOWN = null;
    var DEBUG_TEXT_FILTER_INPUT = null;
    var DEBUG_SAVE_BUTTON = null;
    
    // ログレベル定義
    var LOG_LEVELS = {
        "verbose": { index: 0, label: "VERBOSE", color: "#808080" },
        "notice": { index: 1, label: "NOTICE", color: "#0080FF" },
        "warning": { index: 2, label: "WARNING", color: "#FFA500" },
        "error": { index: 3, label: "ERROR", color: "#FF4500" },
        "fatal": { index: 4, label: "FATAL", color: "#FF0000" }
    };
    
    function debugLog(functionName, message, context, level) {
        level = level || "verbose";
        var timestamp = new Date().toLocaleTimeString();
        
        var debugMsg = {
            timestamp: timestamp,
            level: level,
            functionName: functionName,
            message: message,
            context: context ? describeValue(context) : null,
            fullText: "[" + timestamp + "][" + LOG_LEVELS[level].label + ":" + functionName + "] " + message +
                     (context ? "\nContext: " + describeValue(context) : "")
        };
        
        DEBUG_LOGS.push(debugMsg);
    }
    
    function updateDebugDisplay() {
        if (!DEBUG_LIST || !DEBUG_LOGS) return;
        
        try {
            // ListBoxの内容をクリア
            DEBUG_LIST.removeAll();
            
            // フィルタリング処理
            var filteredLogs = DEBUG_LOGS.filter(function(logEntry) {
                // レベルフィルタリング
                var currentLevelIndex = LOG_LEVELS[DEBUG_FILTER_LEVEL].index;
                var logLevelIndex = LOG_LEVELS[logEntry.level].index;
                if (logLevelIndex < currentLevelIndex) {
                    return false;
                }
                
                // テキストフィルタリング
                if (DEBUG_FILTER_TEXT && DEBUG_FILTER_TEXT.trim() !== "") {
                    var searchText = DEBUG_FILTER_TEXT.toLowerCase();
                    var fullText = logEntry.fullText.toLowerCase();
                    if (fullText.indexOf(searchText) === -1) {
                        return false;
                    }
                }
                
                return true;
            });
            
            // 行数制限を撤廃（全てのログを表示）
            for (var i = 0; i < filteredLogs.length; i++) {
                DEBUG_LIST.add("item", filteredLogs[i].fullText);
            }
            
            // 最新のログを選択状態にする
            if (DEBUG_LIST.items.length > 0) {
                DEBUG_LIST.selection = 0;
            }
            
            // パネルのレイアウトを更新
            if (DEBUG_PANEL && DEBUG_PANEL.layout) {
                DEBUG_PANEL.layout.layout(true);
            }
        } catch (e) {
            // エラーが発生した場合は従来のalertを使用
            alert("Debug display error: " + e.toString());
        }
    }
    
    function clearDebugLogs() {
        DEBUG_LOGS = [];
        if (DEBUG_LIST) {
            DEBUG_LIST.removeAll();
            // パネルのレイアウトを更新
            if (DEBUG_PANEL && DEBUG_PANEL.layout) {
                DEBUG_PANEL.layout.layout(true);
            }
        }
    }
    
    function saveDebugLogs() {
        if (!DEBUG_LOGS || DEBUG_LOGS.length === 0) {
            alert("コピーするログがありません");
            return;
        }
        
        try {
            // フィルタリングされたログのテキストを取得
            var filteredLogs = DEBUG_LOGS.filter(function(logEntry) {
                var currentLevelIndex = LOG_LEVELS[DEBUG_FILTER_LEVEL].index;
                var logLevelIndex = LOG_LEVELS[logEntry.level].index;
                if (logLevelIndex < currentLevelIndex) {
                    return false;
                }
                
                if (DEBUG_FILTER_TEXT && DEBUG_FILTER_TEXT.trim() !== "") {
                    var searchText = DEBUG_FILTER_TEXT.toLowerCase();
                    var fullText = logEntry.fullText.toLowerCase();
                    if (fullText.indexOf(searchText) === -1) {
                        return false;
                    }
                }
                
                return true;
            });
            
            var logText = "";
            for (var i = 0; i < filteredLogs.length; i++) {
                logText += filteredLogs[i].fullText + "\n";
            }
            
            // クリップボードにコピー（ExtendScriptの制限により、テキストファイルとして保存）
            var tempFile = new File(Folder.temp.fsName + "/debug_logs_" + new Date().getTime() + ".txt");
            tempFile.encoding = "UTF-8";
            tempFile.open("w");
            tempFile.write(logText);
            tempFile.close();
            
            alert("デバッグログをテキストファイルに保存しました:\n" + tempFile.fsName + "\n\nファイルを開いてコピーしてください。");
            tempFile.execute();
        } catch (e) {
            alert("ログのコピーでエラーが発生しました: " + e.toString());
        }
    }
    
    function showLogDetailDialog(logEntry) {
        try {
            // ログ詳細表示ダイアログを作成
            var detailDialog = new Window("dialog", "ログ詳細 - " + logEntry.level.toUpperCase());
            detailDialog.orientation = "column";
            detailDialog.alignChildren = "fill";
            detailDialog.preferredSize.width = 600;
            detailDialog.preferredSize.height = 400;
            
            // ログ情報ヘッダー
            var headerGroup = detailDialog.add("group");
            headerGroup.orientation = "column";
            headerGroup.alignChildren = "fill";
            
            var timeLabel = headerGroup.add("statictext", undefined, "時刻: " + logEntry.timestamp);
            var levelLabel = headerGroup.add("statictext", undefined, "レベル: " + logEntry.level.toUpperCase());
            var functionLabel = headerGroup.add("statictext", undefined, "関数: " + logEntry.functionName);
            
            // ログ内容表示エリア
            var contentGroup = detailDialog.add("group");
            contentGroup.orientation = "column";
            contentGroup.alignChildren = "fill";
            contentGroup.alignment = "fill";
            
            var contentLabel = contentGroup.add("statictext", undefined, "ログ内容:");
            
            // EditTextで表示（複数行、選択・コピー可能）
            var logText = contentGroup.add("edittext", undefined, logEntry.fullText, {multiline: true, scrolling: true});
            logText.alignment = "fill";
            logText.preferredSize.height = 250;
            logText.active = true; // フォーカスを設定
            
            // 使用方法の説明
            var instructionText = contentGroup.add("statictext", undefined, "テキストを選択してCtrl+C（Mac: Cmd+C）でコピーできます");
            instructionText.graphics.font = ScriptUI.newFont(instructionText.graphics.font.name, ScriptUI.FontStyle.ITALIC, 10);
            
            // ボタングループ
            var buttonGroup = detailDialog.add("group");
            buttonGroup.orientation = "row";
            buttonGroup.alignment = "center";
            
            var selectAllButton = buttonGroup.add("button", undefined, "全選択");
            var closeButton = buttonGroup.add("button", undefined, "閉じる");
            
            // イベントハンドラ
            selectAllButton.onClick = function() {
                logText.active = true;
                // ExtendScriptでは完全な全選択は制限があるが、できる限り選択
                logText.textselection = logText.text;
            };
            
            closeButton.onClick = function() {
                detailDialog.close();
            };
            
            // Enterキーで閉じる
            detailDialog.onShow = function() {
                logText.active = true;
            };
            
            // ダイアログを表示
            detailDialog.show();
            
        } catch (e) {
            alert("ログ詳細表示でエラーが発生しました: " + e.toString());
        }
    }
    
    function describeValue(value) {
        try {
            if (value === null) return "null";
            if (value === undefined) return "undefined";
            if (typeof value === 'number') return "number(" + value + ")";
            if (typeof value === 'string') return "string('" + value + "')";
            if (typeof value === 'boolean') return "boolean(" + value + ")";
            if (value instanceof Array) return "array[" + value.length + "](" + value.slice(0, 3).join(", ") + (value.length > 3 ? "..." : "") + ")";
            if (value && value.vertices) return "pathValue(vertices:" + value.vertices.length + ")";
            if (value && value.matchName) return "property(" + value.matchName + ")";
            if (value && value.name) return "object(" + value.name + ")";
            return typeof value + "(" + Object.prototype.toString.call(value) + ")";
        } catch (e) {
            return "error_describing_value";
        }
    }
    
    
    
    // Legacy numeric helpers - 既存コードとの互換性維持
    function eqNum(a,b){ return Math.abs(a-b) <= 1e-6; }
    
    function arrayValuesAreEqual(a, b) {
        if (a.length !== b.length) return false;
        for (var i = 0; i < a.length; i++) {
            if (a[i] instanceof Array || b[i] instanceof Array) {
                if (!valuesAreEqual(a[i], b[i])) return false;
            } else {
                if (!eqNum(a[i], b[i])) return false;
            }
        }
        return true;
    }

    function saveGroup(dataObj, startFrame, valuesArray){
        if (valuesArray.length>0) dataObj[String(startFrame)] = valuesArray.slice();
    }

    // ===== marker extractors =====
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
    // ===== shape extractors =====
    // Legacy functions removed - now using unified configuration-driven system

    function extractPathValue(pathValue, decimalPlaces) {
        try {
            if (!pathValue || !pathValue.vertices) {
                return null;
            }
            
            var vertices = [];
            var inTangents = [];
            var outTangents = [];
            
            for (var i = 0; i < pathValue.vertices.length; i++) {
                var vertex = pathValue.vertices[i];
                vertices.push([
                    parseFloat(Number(vertex[0]).toFixed(decimalPlaces)),
                    parseFloat(Number(vertex[1]).toFixed(decimalPlaces))
                ]);
                
                if (pathValue.inTangents && pathValue.inTangents[i]) {
                    var inTangent = pathValue.inTangents[i];
                    inTangents.push([
                        parseFloat(Number(inTangent[0]).toFixed(decimalPlaces)),
                        parseFloat(Number(inTangent[1]).toFixed(decimalPlaces))
                    ]);
                } else {
                    inTangents.push([0, 0]);
                }
                
                if (pathValue.outTangents && pathValue.outTangents[i]) {
                    var outTangent = pathValue.outTangents[i];
                    outTangents.push([
                        parseFloat(Number(outTangent[0]).toFixed(decimalPlaces)),
                        parseFloat(Number(outTangent[1]).toFixed(decimalPlaces))
                    ]);
                } else {
                    outTangents.push([0, 0]);
                }
            }
            
            return {
                vertices: vertices,
                inTangents: inTangents,
                outTangents: outTangents,
                closed: pathValue.closed || false
            };
        } catch (e) {
            return null;
        }
    }

    function pathValuesAreEqual(a, b) {
        if (!a || !b) return a === b;
        if (a.closed !== b.closed) return false;
        if (!valuesAreEqual(a.vertices, b.vertices)) return false;
        if (!valuesAreEqual(a.inTangents, b.inTangents)) return false;
        if (!valuesAreEqual(a.outTangents, b.outTangents)) return false;
        return true;
    }



    // 時間ベースのキーフレーム抽出関数（統一版）
    function extractKeyframeProperty(prop, offsetTime, fps, decimalPlaces, customProcessor) {
        try {
            if (!prop) {
                debugLog("extractKeyframeProperty", "prop is null", null, "verbose");
                return null;
            }
            
            debugLog("extractKeyframeProperty", "Processing property: " + (prop.matchName || prop.name || "unnamed"), null, "verbose");

            var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;

            if (nk === 0) {
                debugLog("extractKeyframeProperty", "No animation detected, returning initial value only", null, "verbose");
                return null;
            }

            // Extract keyframe data with time keys
            var keyframes = [];
            
            for (var i = 1; i <= nk; i++) {
                try {
                    var keyTime = prop.keyTime(i);
                    var adjustedTime = keyTime - offsetTime;
                    
                    // Format time with 6 decimal places for precision (microseconds)
                    var timeKey = Number(adjustedTime.toFixed(6));
                    
                    var keyValue = extractValue(prop, keyTime, decimalPlaces, customProcessor, fps);
                    
                    // キーフレーム情報を構築
                    var keyframeInfo = {
                        time: timeKey,
                        value: keyValue
                    };
                    
                    // 補間情報を追加
                    var interpolation = extractKeyframeInterpolation(prop, i);
                    if (interpolation) {
                        keyframeInfo.interpolation = interpolation;
                    }
                    
                    // 空間タンジェント情報を追加（該当する場合）
                    var spatialTangents = extractSpatialTangents(prop, i, decimalPlaces);
                    if (spatialTangents) {
                        keyframeInfo.spatialTangents = spatialTangents;
                    }
                    
                    keyframes.push(keyframeInfo);
                    
                } catch (e) {
                    debugLog("extractKeyframeProperty", "Error extracting keyframe " + i + ": " + e.toString(), null, "warning");
                }
            }
            
            debugLog("extractKeyframeProperty", "Keyframe extraction completed successfully", {
                keyframeCount: keyframes.length,
                fps: fps
            });
            return keyframes;
        } catch (e) {
            debugLog("extractKeyframeProperty", "Error in keyframe extraction: " + e.toString(), null, "error");
            return null;
        }
    }
    
    function extractAnimatedProperty(prop, offset, duration, fps, decimalPlaces, customProcessor) {
        try {
            if (!prop) {
                debugLog("extractAnimatedProperty", "prop is null", null, "verbose");
                return null;
            }
            
            debugLog("extractAnimatedProperty", "Processing property: " + (prop.matchName || prop.name || "unnamed"), null, "verbose");
            
            var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
            if (nk === 0) {
                // No animation - only return initial value info
                debugLog("extractAnimatedProperty", "No animation detected, returning initial value only", null, "verbose");
                return null;
            }
            

            function toTime(f) { return f / fps; }

            // Extract animation data (keyframes)
            var animationData = {};
            var prevValue = null;
            var groupStartFrame = null;
            var groupValues = [];
            
            for (var frame = 0; frame <= duration; frame++) {
                var time = toTime(frame + offset);
                var extractedValue = extractValue(prop, time, decimalPlaces, customProcessor, fps);
                                
                var hasChanged = (prevValue === null) || !valuesAreEqual(extractedValue, prevValue);
                
                if (hasChanged) {
                    if (groupStartFrame === null) {
                        groupStartFrame = frame;
                        groupValues = [];
                    } else if (frame !== groupStartFrame + groupValues.length) {
                        saveGroup(animationData, groupStartFrame, groupValues);
                        groupStartFrame = frame;
                        groupValues = [];
                    }
                    groupValues.push(extractedValue);
                } else {
                    if (groupStartFrame !== null) {
                        saveGroup(animationData, groupStartFrame, groupValues);
                        groupStartFrame = null;
                        groupValues = [];
                    }
                }
                
                prevValue = extractedValue;
            }
            
            if (groupStartFrame !== null) {
                saveGroup(animationData, groupStartFrame, groupValues);
            }
            
            debugLog("extractAnimatedProperty", "Extraction completed successfully", {
                frameCount: Object.keys(animationData).length
            });
            return animationData;
        } catch (e) {
            debugLog("extractAnimatedProperty", "Error in extraction: " + e.toString(), null, "error");
            return null;
        }
    }

    function extractPropertyValue(prop, options, layer, offsetTime, config) {
        var fps = layer.containingComp.frameRate;
        function toTime(f) { return f / fps; }
        function toFrame(t){ return Math.round(t * fps); }
        var DEC = options.decimalPlaces || 4;
        var customProcessor = config ? config.customProcessor : null;

        var result;
        if (options.keyframes) {
            // エクスプレッション検出: 有効なエクスプレッションが存在する場合は自動的にベイク
            var hasExpression = false;
            try {
                hasExpression = prop.canSetExpression &&
                              prop.expressionEnabled &&
                              prop.expression &&
                              prop.expression.trim() !== "";
            } catch (e) {
            }

            var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
            
            if (hasExpression) {
                var offset = toFrame(offsetTime);
                var layerInPoint = toFrame(layer.inPoint);
                var layerOutPoint = toFrame(layer.outPoint);
                var duration = layerOutPoint - layerInPoint;
                
                debugLog("ExpressionDetection",
                         "Expression without keyframes, baking layer duration: " + duration + " frames",
                         {
                             propertyName: prop.name,
                             layerInPoint: layerInPoint,
                             layerOutPoint: layerOutPoint
                         },
                         "notice");
                
                result = extractAnimatedProperty(prop, offset, duration, fps, DEC, customProcessor);
            } else if (nk > 0) {
                var start = toFrame(prop.keyTime(1));
                var end = toFrame(prop.keyTime(nk));
                var duration = end - start;
                var shouldBake = options.useFullFrameAnimation || hasExpression;
                if (shouldBake) {
                    var offset = toFrame(offsetTime);
                    result = extractAnimatedProperty(prop, offset, duration, fps, DEC, customProcessor);
                } else {
                    // Always use time-based extraction (time-only export mode)
                    result = extractKeyframeProperty(prop, offsetTime, fps, DEC, customProcessor);
                }
            } else {
                return null;
            }
        } else {
            result = extractValue(prop, offsetTime, DEC, customProcessor, fps);
        }
        return result;
    }
    
    // ===== 統一処理関数 =====
    function extractPropertiesRecursive(property, options, layer, offsetTime) {
        try {
            if (!property) {
                debugLog("extractPropertiesRecursive", "property is null", null, "verbose");
                return null;
            }
            
            // if (!property.enabled) {
            //     debugLog("extractPropertiesRecursive", "Property not enabled: " + property.matchName, null, "verbose");
            //     return null;
            // }

            var config = PROPERTY_MAPPING_CONFIG[property.matchName];
            if (!config) {
                debugLog("extractPropertiesRecursive", "No config found for matchName: " + property.matchName, null, "verbose");
                return null;
            }
            
            if (!isPropertyEnabled(config)) {
                debugLog("extractPropertiesRecursive", "Property disabled: " + property.matchName, null, "verbose");
                return null;
            }
            
            debugLog("extractPropertiesRecursive", "Processing property: " + property.matchName, null, "verbose");
            
            var result = null;

            var propType = property.propertyType;
            switch(propType) {
                case PropertyType.PROPERTY:
                    result = extractPropertyValue(property, options, layer, offsetTime, config);
                    break;
                case PropertyType.INDEXED_GROUP:
                    result = [];
                    var preserveIndexes = config.preserveIndexes || false;
                    
                    for (var i = 1; i <= property.numProperties; i++) {
                        var childProp = property.property(i);
                        var child = extractPropertiesRecursive(childProp, options, layer, offsetTime);
                        
                        if (preserveIndexes) {
                            // インデックスを維持（nullも配列に追加）
                            result.push(child);
                        } else {
                            // 従来の動作（nullは除外）
                            if (child) {
                                result.push(child);
                            }
                        }
                    }
                    
                    if (preserveIndexes) {
                        // 全てがnullの場合のみresult自体をnullにする
                        var hasValidChild = false;
                        for (var j = 0; j < result.length; j++) {
                            if (result[j] !== null) {
                                hasValidChild = true;
                                break;
                            }
                        }
                        if (!hasValidChild) result = null;
                    } else {
                        // 従来の動作
                        if(result.length === 0) result = null;
                    }
                    break;
                case PropertyType.NAMED_GROUP:
                    result = {};
                    for (var i = 1; i <= property.numProperties; i++) {
                        var childProp = property.property(i);
                        var child = extractPropertiesRecursive(childProp, options, layer, offsetTime);
                        if (child) {
                            if(child instanceof Array) {
                                // WARNING: たぶん正しくない。NAMED_GROUPをキーフレームで展開する場合の処理はどう書く？
                                result = child;
                            }
                            else {
                                for (var key in child) {
                                    if (child.hasOwnProperty(key)) {
                                        result[key] = child[key];
                                    }
                                }
                            }
                        }
                    }
                    if(config.customProcessor === "maskAtom") {
                        result.inverted = property.inverted;
                        result.mode = maskModeToString(property.maskMode);
                    }
                    if(Object.keys(result).length === 0) result = null;
                    break;
            }
            if(result === null) {
                debugLog("extractPropertiesRecursive", "No result extracted for property: " + property.matchName, null, "verbose");
                return null;
            }
            
            // Add visibility state for shape elements and property groups
            // Check if the property has an enabled property (shape elements have this)
            if (property.enabled !== undefined && result !== null) {
                // Add visibility information to the result
                if (result instanceof Array) {
                    // For keyframe arrays, add visible to each keyframe's value
                    result = result.map(function(item) {
                        if (item !== null && item.value !== null && typeof item.value === 'object') {
                            item.value.visible = property.enabled;
                        }
                        return item;
                    });
                } else if (typeof result === 'object') {
                    // For object results (static values or property groups), add visible property
                    result.visible = property.enabled;
                }
            }
            
            if (config.merge) {
                for (var key in config.merge) {
                    if (config.merge.hasOwnProperty(key)) {
                        if(result instanceof Array) {
                            // WARNING: 多分正しくない。キーフレームデータ以外にも配列で返ってくるやつがいたらどうする？
                            result = result.map(function(item){
                                if (item !== null && item.value !== null) {
                                    item.value[key] = config.merge[key];
                                }
                                return item;
                            });
                        }
                        else {
                            result[key] = config.merge[key];
                        }
                    }
                }
            }
            if (config.wrapInObject){
                var wrapped = {};
                wrapped[config.wrapInObject] = result;
                result = wrapped;
            }
            debugLog("extractPropertiesRecursive", "Processing completed for: " + property.matchName, null, "verbose");
            return result;
        } catch (e) {
            debugLog("extractPropertiesRecursive", "Error in processing: " + e.toString(), null, "error");
            return null;
        }
    }
    
    // ===== LAYER FILTERING SYSTEM =====
    
    /**
     * レイヤー依存関係を解決し、必要なレイヤーのセットを構築する
     * @param {CompItem} comp - コンポジション
     * @return {Object} レイヤーIDをキーとした必要なレイヤーの辞書
     */
    function buildRequiredLayersSet(comp) {
        var requiredLayers = {};
        
        /**
         * レイヤーとその依存関係を再帰的に追加
         * @param {Layer} layer - 追加するレイヤー
         */
        function addLayerWithDependencies(layer) {
            if (!layer || requiredLayers[layer.id]) {
                return; // 既に処理済み
            }
            
            // このレイヤーを必要なセットに追加
            requiredLayers[layer.id] = true;
            debugLog("LayerFiltering", "Added layer to required set: " + layer.name + " (ID: " + layer.id + ")", null, "verbose");
            
            // 親レイヤーへの依存を解決
            if (layer.parent) {
                debugLog("LayerFiltering", "Layer " + layer.name + " depends on parent: " + layer.parent.name, null, "verbose");
                addLayerWithDependencies(layer.parent);
            }
            
            // トラックマットへの依存を解決
            if (layer.trackMatteLayer) {
                debugLog("LayerFiltering", "Layer " + layer.name + " depends on track matte: " + layer.trackMatteLayer.name, null, "verbose");
                addLayerWithDependencies(layer.trackMatteLayer);
            }
        }
        
        // 可視レイヤーから開始して依存関係を構築
        for (var i = 1; i <= comp.numLayers; i++) {
            var layer = comp.layer(i);
            if (layer.enabled) {
                debugLog("LayerFiltering", "Processing visible layer: " + layer.name + " (ID: " + layer.id + ")", null, "verbose");
                addLayerWithDependencies(layer);
            }
        }
        
        return requiredLayers;
    }
    
    /**
     * レイヤーが処理対象かどうかを判定
     * @param {Layer} layer - 判定するレイヤー
     * @param {Object} requiredLayers - 必要なレイヤーのセット
     * @return {boolean} 処理対象の場合はtrue
     */
    function shouldProcessLayer(layer, requiredLayers) {
        var isRequired = requiredLayers[layer.id] === true;
        if (!isRequired) {
            debugLog("LayerFiltering", "Skipping layer (not required): " + layer.name + " (ID: " + layer.id + ")", null, "notice");
        }
        return isRequired;
    }
    
    // ===== main extractor =====
    
    // ===== EFFECT BAKING UTILITY FUNCTIONS =====
    
    function resetBakedPrecomps() {
        bakedPrecomps = {};
        debugLog("resetBakedPrecomps", "Baked precomps history cleared", null, "notice");
    }
    
    function resetProcessedFootage() {
        processedFootageItems = {};
        debugLog("resetProcessedFootage", "Processed footage history cleared", null, "notice");
    }
    
    function shouldBakeEffect(effect) {
        try {
            if (!effect || !effect.enabled) {
                return false;
            }
            var matchName = effect.matchName;
            if (EFFECT_BAKING_EXCLUSIONS[matchName]) {
                return false;
            }
            return true;
        } catch (e) {
            debugLog("shouldBakeEffect", "Error checking effect: " + e.toString(), null, "warning");
            return false;
        }
    }
    
    function hasEffectsThatRequireBaking(layer) {
        try {
            if (!layer || !layer.property) {
                return false;
            }
            var effectsGroup = layer.property("ADBE Effect Parade");
            if (!effectsGroup || effectsGroup.numProperties === 0) {
                return false;
            }
            for (var i = 1; i <= effectsGroup.numProperties; i++) {
                var effect = effectsGroup.property(i);
                if (shouldBakeEffect(effect)) {
                    return true;
                }
            }
            return false;
        } catch (e) {
            debugLog("hasEffectsThatRequireBaking", "Error checking layer effects: " + e.toString(), null, "warning");
            return false;
        }
    }
    
    function getBakingEffectInfo(layer) {
        try {
            var effectInfo = [];
            var effectsGroup = layer.property("ADBE Effect Parade");
            if (!effectsGroup) {
                return effectInfo;
            }
            for (var i = 1; i <= effectsGroup.numProperties; i++) {
                var effect = effectsGroup.property(i);
                if (shouldBakeEffect(effect)) {
                    effectInfo.push({
                        name: effect.name,
                        matchName: effect.matchName
                    });
                }
            }
            return effectInfo;
        } catch (e) {
            debugLog("getBakingEffectInfo", "Error getting effect info: " + e.toString(), null, "error");
            return [];
        }
    }
    
    function calculateFileHash(file) {
        try {
            file.encoding = "BINARY";
            file.open("r");
                
            var hash = file.length;
            
            // PNG形式検証
            var signature = file.read(8);
            if (signature.charCodeAt(0) !== 0x89 || 
                signature.charCodeAt(1) !== 0x50 ||
                signature.charCodeAt(2) !== 0x4E ||
                signature.charCodeAt(3) !== 0x47) {
                file.close();
                debugLog("calculateFileHash", "Not a PNG file, using fallback hash", null, "warning");
                return hash.toString(16); // Fallback
            }

            // 1バイト読み出して0–255の数値に変換
            function readByte() {
                if (file.eof) {
                    throw new Error("Unexpected EOF");
                }
                var ch = file.readch();     // 1文字の文字列
                if (ch === "") {
                    throw new Error("Unexpected EOF (empty read)");
                }
                return ch.charCodeAt(0) & 0xFF;
            }

            // 4バイトをビッグエンディアンのUint32として読む
            function readUint32BE() {
                var b0 = readByte();
                var b1 = readByte();
                var b2 = readByte();
                var b3 = readByte();
                // >>>0 で符号なし32bitにそろえる
                return (((b0 << 24) | (b1 << 16) | (b2 << 8) | b3) >>> 0);
            }

            while (!file.eof) {
                // length (4バイト)
                var length = readUint32BE();

                // type (4バイト → ASCII)
                var type = "";
                for (i = 0; i < 4; i++) {
                    type += String.fromCharCode(readByte());
                }
                // IEND で終了
                if (type === "IEND") {
                    break;
                }

                // data 部分をスキップ
                file.seek(length, 1);

                // crc (4バイト)
                for (var i = 0; i < 4; i++) {
                    var ch = readByte();
                    var idx = (hash ^ ch) & 0xFF;
                    hash = (hash >>> 8) ^ CRC32_TABLE[idx];
                }

            }
            file.close();
            
            return hash.toString(16);
        } catch (e) {
            debugLog("calculateFileHash", "Error calculating hash: " + e.toString(), null, "error");
            return null;
        }
    }

    function makeSequenceMetadata(files, deleteDuplicates) {
        var uniqueFileList = [];
        var uniqueFrames = {};
        var frameMapping = [];
        for (var file_index = 0; file_index < files.length; file_index++) {
            var file = files[file_index];
            // ファイルが存在し、サイズが0より大きいことを確認
            var maxRetries = 100;
            var retryCount = 0;
            while (retryCount < maxRetries && (!file.exists || file.length === 0)) {
                $.sleep(10);
                retryCount++;
            }

            var hash = calculateFileHash(file);
            
            if (uniqueFrames[hash] !== undefined) {
                if (deleteDuplicates) {
                    file.remove();
                }
                frameMapping.push(uniqueFrames[hash]);
            } else {
                var actualFrame = uniqueFileList.length;
                uniqueFileList.push(decodeURI(file.name));
                uniqueFrames[hash] = actualFrame;
                frameMapping.push(actualFrame);
            }
        }
        return {
            uniqueFileList: uniqueFileList,
            frameMapping: frameMapping
        };
    }

    function renderPrecompAsSequence(precomp, outputFolder, baseName, options) {
        try {
            var fps = precomp.frameRate;
            var duration = precomp.duration;
            var frameCount = Math.ceil(duration * fps);
            
            debugLog("renderPrecompAsSequence", "Rendering precomp as PNG sequence", {
                precompName: precomp.name,
                fps: fps,
                frameCount: frameCount,
                duration: duration
            }, "notice");
            
            var sequenceFolder = new Folder(outputFolder.fsName + "/" + baseName);
            if (!sequenceFolder.exists) {
                sequenceFolder.create();
            }
            
            precomp.resolutionFactor = [1, 1];

            var fileList = [];

            for (var frame = 0; frame < frameCount; frame++) {
                var time = frame / fps;
                var paddedFrame = ("0000" + frame).slice(-4);
                var fileName = baseName + "_" + paddedFrame + ".png";
                var outputFile = new File(sequenceFolder.fsName + "/" + fileName);
                
                precomp.saveFrameToPng(time, outputFile);
                fileList.push(outputFile);
            }

            debugLog("renderPrecompAsSequence", "PNG sequence rendering completed", {
                frameCount: frameCount,
                outputFolder: sequenceFolder.fsName
            }, "notice");
            
            var sequenceMetadata = makeSequenceMetadata(fileList, true);

            return {
                frameCount: frameCount,
                fps: fps,
                directory: sequenceFolder,
                fileList: sequenceMetadata.uniqueFileList,
                frameMapping: sequenceMetadata.frameMapping
            };
        } catch (e) {
            debugLog("renderPrecompAsSequence", "Error rendering PNG sequence: " + e.toString(), null, "error");
            throw e;
        }
    }
    
    function precomposeAndRenderLayer(layer, comp, assetFolder, options) {
        
        try {
            var layerName = layer.name;
            var layerIndex = layer.index;
            var effectInfo = getBakingEffectInfo(layer);
            
            debugLog("precomposeAndRenderLayer", "Starting precompose and render for layer with effects", {
                layerName: layerName,
                effectCount: effectInfo.length,
                effects: effectInfo
            }, "notice");
            
            // Store layer duration BEFORE precompose (layer reference becomes invalid after)
            var layerInPoint = layer.inPoint;
            var layerOutPoint = layer.outPoint;
            var layerDuration = layerOutPoint - layerInPoint;
            var uniqueName = layerUniqueName(layer);
            
            debugLog("precomposeAndRenderLayer", "Layer timing before precompose", {
                inPoint: layerInPoint,
                outPoint: layerOutPoint,
                duration: layerDuration
            }, "verbose");
            
            var precompName = "Baked_" + layerName + "_" + new Date().getTime();
            var precompLayerIndices = [layerIndex];
            
            var precomp = comp.layers.precompose(
                precompLayerIndices,
                precompName,
                true  // moveAllAttributes = true (エフェクトとトランスフォームを全て移動)
            );

            for (var i = 1; i <= precomp.numLayers; i++) {
                precomp.layer(i).startTime = -layerInPoint;
            }
            // Adjust precomp duration to match the layer's duration
            precomp.duration = layerDuration;
                        
            debugLog("precomposeAndRenderLayer", "Set precomp work area", {
                workAreaStart: precomp.workAreaStart,
                workAreaDuration: precomp.workAreaDuration
            }, "verbose");
            
            debugLog("precomposeAndRenderLayer", "Adjusted precomp duration to match layer", {
                precompDuration: precomp.duration,
                layerDuration: layerDuration
            }, "verbose");
            
            // Get the new precomp layer reference and adjust its timing
            var newLayer = comp.layer(layerIndex);
            newLayer.startTime = layerInPoint;
            newLayer.inPoint = layerInPoint;
            newLayer.outPoint = layerOutPoint;

            debugLog("precomposeAndRenderLayer", "Precomp created successfully", {
                precompName: precompName,
                precompId: precomp.id
            }, "notice");
            
            var baseName = uniqueName;
            var renderResult = renderPrecompAsSequence(precomp, assetFolder, baseName, options);
            
            var metadata = {
                fps: renderResult.fps,
                directory: "./" + decodeURI(renderResult.directory.name),
                frames: {
                    list: renderResult.fileList,
                    indices: renderResult.frameMapping
                }
            };

            return {
                precomp: precomp,
                baseName: baseName,
                metadata: metadata,
                renderResult: renderResult
            };
        } catch (e) {
            debugLog("precomposeAndRenderLayer", "Error in precompose and render: " + e.toString(), null, "error");
            throw e;
        }
    }
    
    function extractPropertiesForAllLayers(comp, options){

        debugLog("extractPropertiesForAllLayers", "Starting composition processing: " + comp.name, {
            compId: comp.id,
            compName: comp.name
        }, "notice");
        var compName = comp.name.fsSanitized();
        var outputFolderPath = options.outputFolderPath;

        // フラット構造
        var outputFolder = new Folder(outputFolderPath);
        var layerFolder = new Folder(outputFolderPath + "/" + compName + "/layers");
        
        // アセットフォルダパス決定
        var assetFolderPath = determineAssetFolderPath(options, outputFolderPath, compName);
        var assetFolder = new Folder(assetFolderPath);

        var DEC = options.decimalPlaces || 4;

        if (!outputFolder.exists && !outputFolder.create()){ alert("指定された出力先フォルダを作成できませんでした。" + outputFolder.fsName); return; }
        if (!assetFolder.exists && !assetFolder.create()){ alert("指定されたアセット保存先フォルダを作成できませんでした。" + assetFolder.fsName); return; }
        if (!layerFolder.exists) layerFolder.create();

        var fps = comp.frameRate;

        var compInfo = {};
        compInfo["version"]       = "2.0";
        compInfo["exportMode"]    = "time";
        compInfo["duration"]      = comp.duration;
        compInfo["fps"]           = fps;
        compInfo["width"]         = comp.width;
        compInfo["height"]        = comp.height;
        compInfo["startTime"]     = comp.workAreaStart;
        compInfo["endTime"]       = comp.workAreaStart + comp.workAreaDuration;
        compInfo["layers"]        = [];
        
        debugLog("extractPropertiesForAllLayers", "Using time-based export (v2.0)", {
            fps: fps,
            duration: comp.duration
        }, "notice");
        
        // コンポジションマーカーを追加
        if (comp.markerProperty.numKeys > 0) {
            compInfo["markers"] = extractMarkers(comp.markerProperty, comp.workAreaStart);
        }

        debugLog("extractPropertiesForAllLayers", "Total layers to process: " + comp.numLayers);
        
        // レイヤーフィルタリング: 必要なレイヤーのセットを構築
        debugLog("extractPropertiesForAllLayers", "Building required layers set...", null, "notice");
        var requiredLayers = buildRequiredLayersSet(comp);
        var requiredLayerCount = 0;
        for (var layerId in requiredLayers) {
            if (requiredLayers.hasOwnProperty(layerId)) {
                requiredLayerCount++;
            }
        }
        debugLog("extractPropertiesForAllLayers", "Total layers in composition: " + comp.numLayers + ", Required layers: " + requiredLayerCount, null, "notice");
        
        for (var i=1;i<=comp.numLayers;i++){
            try {
                debugLog("LayerProcessing", "=== Starting layer " + i + " of " + comp.numLayers + " ===");
                
                var layer = null;
                try {
                    layer = comp.layer(i);
                    debugLog("LayerProcessing", "Retrieved layer object for index " + i);
                } catch (layerRetrievalError) {
                    debugLog("LayerProcessing", "ERROR: Failed to retrieve layer at index " + i + ": " + layerRetrievalError.toString());
                    continue;
                }
                
                // Check if layer is null or undefined
                if (!layer) {
                    debugLog("LayerProcessing", "ERROR: Layer at index " + i + " is null or undefined");
                    continue;
                }
                
                // レイヤーフィルタリング: 必要なレイヤーかどうかをチェック
                if (!shouldProcessLayer(layer, requiredLayers)) {
                    debugLog("LayerProcessing", "Skipping layer (not in required set): " + layer.name);
                    continue;
                }

                var bakedAsSequence = null;
                try {
                    // Check if this is a text layer
                    function isTextLayer(layer) {
                        return layer.matchName === "ADBE Text Layer";
                    }
                    
                    // Check for text layer baking first (before effect baking)
                    if (isTextLayer(layer) || hasEffectsThatRequireBaking(layer)) {
                        debugLog("LayerProcessing", "Layer requires sequence baking", {
                            layerName: layer.name
                        }, "notice");
                        
                        try {
                            bakedAsSequence = precomposeAndRenderLayer(layer, comp, assetFolder, options);
                            layer = comp.layer(i);
                            debugLog("LayerProcessing", "Layer successfully pre-rendered as sequence", {
                                layerName: layer.name,
                                source: bakedAsSequence.baseName
                            }, "notice");
                                                        
                        } catch (bakingError) {
                            debugLog("LayerProcessing", "Error during layer baking: " + bakingError.toString(), {
                                layerName: layer.name
                            }, "error");
                        }
                    }
                } catch (effectCheckError) {
                    debugLog("LayerProcessing", "Error checking for effects or text layer: " + effectCheckError.toString(), {
                        layerName: layer.name
                    }, "warning");
                }

                var layerInfo = {};
                layerInfo.name = layer.name;
                var layerNameForFile = layerUniqueName(layer);
                layerInfo.uniqueName = layerNameForFile;
                layerInfo.file = getRelativePath(outputFolder, layerFolder) + "/" + layerInfo.uniqueName + ".json";
                layerInfo.visible = layer.enabled;
                layerInfo.startTime = layer.startTime;

                if (layer.parent) {
                    layerInfo.parent = layerUniqueName(comp.layer(layer.parent.index));
                }
                if (layer.trackMatteLayer) {
                    layerInfo.trackMatte = {
                        layer: layerUniqueName(layer.trackMatteLayer),
                        type: trackMatteTypeToString(layer.trackMatteType)
                    };
                }
                compInfo["layers"].push(layerInfo);

                var resultData = {};
                
                // Safely extract basic layer properties using common error handling
                safelyProcessLayerProperty(layer, "name", function() {
                    resultData["name"] = layer.name;
                    debugLog("LayerProcessing", "Set result data name");
                    return layer.name;
                });
                
                var layerType = safelyProcessLayerProperty(layer, "layerType", function() {
                    var type = layer.matchName || "Unknown";
                    debugLog("LayerProcessing", "Layer type: " + type);
                    return type;
                });
                resultData["layerType"] = layerType || "Unknown";
                
                var blendMode = safelyProcessLayerProperty(layer, "blendingMode", function() {
                    var mode = blendingModeToString(layer.blendingMode);
                    debugLog("LayerProcessing", "Blending mode: " + mode);
                    return mode;
                });
                resultData["blendingMode"] = blendMode || "NORMAL";
                
                safelyProcessLayerProperty(layer, "adjustment", function() {
                    resultData["adjustment"] = layer.adjustmentLayer;
                });

                var timing = safelyProcessLayerProperty(layer, "timing", function() {
                    var inPoint = Number((layer.inPoint - layer.startTime).toFixed(6));
                    var outPoint = Number((layer.outPoint - layer.startTime).toFixed(6));
                    var stretch = layer.stretch;  // Time Stretch/Reverse value
                    debugLog("LayerProcessing", "Layer timing - in: " + inPoint + ", out: " + outPoint + ", stretch: " + stretch);
                    return {"in": inPoint, "out": outPoint, "stretch": stretch};
                });
                if (timing) {
                    resultData["in"] = timing.in;
                    resultData["out"] = timing.out;
                    resultData["stretch"] = timing.stretch;
                } else {
                    resultData["in"] = 0;
                    resultData["out"] = 0;
                    resultData["stretch"] = 100.0;  // Default value
                }

                // Process source if it exists
                debugLog("LayerProcessing", "Checking layer source...");
                var sourceType = safelyProcessLayerProperty(layer, "sourceType", function() {
                    var type = getSourceType(layer);
                    debugLog("LayerProcessing", "Source type determined: " + type);
                    return type;
                }) || "unknown";
                
                try {
                    if(bakedAsSequence) {
                        var sourcePath = getRelativePath(layerFolder, assetFolder) + "/" + bakedAsSequence.baseName + ".json";
                        resultData["source"] = sourcePath;
                        sourceType = "sequence";
                        resultData["sourceType"] = sourceType;
                        debugLog("LayerProcessing", "Source type override: " + sourceType + " for baked layer: " + layer.name);
                    }
                    else if (layer.source) {
                        debugLog("LayerProcessing", "Layer has source, processing source for layer: " + layer.name);
                        resultData["sourceType"] = sourceType;
                        var source = null;
                        var sourcePath = null;
                        
                        switch(sourceType) {
                            case "composition":
                                try {
                                    source = layer.source.name + ".json";
                                    // Composition files are saved in the output folder root, not in assets
                                    sourcePath = getRelativePath(layerFolder, outputFolder) + "/" + source;
                                    debugLog("LayerProcessing", "Composition source path: " + sourcePath);
                                } catch (compSourceError) {
                                    debugLog("LayerProcessing", "ERROR: Failed to process composition source: " + compSourceError.toString());
                                }
                                break;
                            case "sequence":
                                try {
                                    // For sequences, we'll create a JSON metadata file
                                    // The source will point to the JSON file, not the directory
                                    source = layer.source.name + ".json";
                                    sourcePath = getRelativePath(layerFolder, assetFolder) + "/" + source;
                                    debugLog("LayerProcessing", "Sequence source path (metadata): " + sourcePath);
                                } catch (seqSourceError) {
                                    debugLog("LayerProcessing", "ERROR: Failed to process sequence source: " + seqSourceError.toString());
                                }
                                break;
                            case "solid":
                                try {
                                    source = layer.source.name + ".json";
                                    sourcePath = getRelativePath(layerFolder, assetFolder) + "/" + source;
                                    debugLog("LayerProcessing", "Solid source path: " + sourcePath);
                                } catch (solidSourceError) {
                                    debugLog("LayerProcessing", "ERROR: Failed to process solid source: " + solidSourceError.toString());
                                }
                                break;
                            case "still":
                            case "video":
                            case "audio":
                                try {
                                    var fileName = decodeURI(layer.source.mainSource.file.name);
                                    var fileNameLower = fileName.toLowerCase();
                                    
                                    source = fileName;
                                    sourcePath = getRelativePath(layerFolder, assetFolder) + "/" + source;
                                    debugLog("LayerProcessing", "Media source path: " + sourcePath);
                                } catch (mediaSourceError) {
                                    debugLog("LayerProcessing", "ERROR: Failed to process media source: " + mediaSourceError.toString());
                                }
                                break;
                            default:
                                debugLog("LayerProcessing", "Unknown source type for layer: " + layer.name + " (type: " + sourceType + ")", null, "warning");
                                break;
                        }
                        
                        // For non-PSD/AI files, set the source path here
                        // For PSD/AI files, the source path will be set during the file copy process below
                        if(sourcePath) {
                            resultData["source"] = sourcePath;
                            try {
                                if (sourceType === "still") {
                                    var fileName = decodeURI(layer.source.mainSource.file.name).toLowerCase();
                                    if (fileName.match(/\.(psd|ai)$/)) {
                                        var baseName = fileName.replace(/\.[^.]+$/, "");
                                        var exportFolderName = assetFolder.fsName + "/" + baseName;
                                        var footageItemName = layer.source.name;
                                        var slashIndex = footageItemName.indexOf('/');
                                        if (slashIndex !== -1) {
                                            footageItemName = footageItemName.substring(0, slashIndex);
                                        }
                                        var outputFileName = footageItemName + ".png";

                                        resultData["source"] = getRelativePath(layerFolder, new Folder(exportFolderName)) + "/" + outputFileName;
                                    }
                                }
                            } catch (sourcePathError) {
                                debugLog("LayerProcessing", "ERROR: Failed to set source path: " + sourcePathError.toString());
                            }
                        }
                    } else {
                        debugLog("LayerProcessing", "Layer has no source: " + layer.name);
                    }
                } catch (sourceError) {
                    debugLog("LayerProcessing", "ERROR: Failed to process source for layer " + layer.name + ": " + sourceError.toString());
                }
                if(bakedAsSequence) {
                    try {                        
                        var metadataFile = new File(assetFolder.fsName + "/" + bakedAsSequence.baseName + ".json");
                        metadataFile.encoding = "UTF-8";
                        metadataFile.open("w");
                        metadataFile.write(JSON.stringify(bakedAsSequence.metadata, null, 4));
                        metadataFile.close();
                        
                        debugLog("SequenceExport", "Created sequence metadata JSON: " + metadataFile.fsName, "notice");
                    } catch(e) {
                        debugLog("SequenceExport", "Error creating sequence metadata: " + e.toString(), null, "error");
                        alert("シーケンスメタデータの作成中にエラー: " + e.message);
                    }
                }
                else {
                    // Check for duplicate footage processing (exclude compositions)
                    var footageId = (layer.source && !layer.nullLayer) ? layer.source.id : null;
                    var alreadyProcessed = footageId ? processedFootageItems[footageId] : false;
                    
                    if (alreadyProcessed) {
                        debugLog("FootageProcessing", "Footage already processed, skipping file operations", {
                            footageName: layer.source.name,
                            footageId: footageId,
                            sourceType: sourceType
                        }, "notice");
                    } else if (footageId) {
                        processedFootageItems[footageId] = true;
                    }
                    
                    if (!alreadyProcessed) {
                        switch(sourceType) {
                        case "composition":
                            try {
                                // Check if layer.source exists and is a valid composition
                                if (layer && layer.source && layer.source instanceof CompItem) {
                                    var nestedOptions = {
                                        outputFolderPath: outputFolderPath,
                                        useSharedAssets: options.useSharedAssets,
                                        sharedAssetsPath: options.sharedAssetsPath,
                                        decimalPlaces: DEC,
                                        useFullFrameAnimation: options.useFullFrameAnimation
                                    };
                                    extractPropertiesForAllLayers(layer.source, nestedOptions);
                                } else {
                                    debugLog("extractPropertiesForAllLayers", "Invalid or null composition source for nested composition processing", {
                                        layerName: layer ? layer.name : "unknown",
                                        sourceType: sourceType,
                                        hasSource: !!(layer && layer.source)
                                    }, "warning");
                                }
                            } catch (e) {
                                debugLog("extractPropertiesForAllLayers", "Error processing nested composition: " + e.toString(), {
                                    layerName: layer ? layer.name : "unknown",
                                    error: e.message
                                }, "error");
                            }
                            break;
                        case "solid":
                            var solidInfo = {
                                name: layer.source.name,
                                width: layer.source.width,
                                height: layer.source.height,
                                color: layer.source.mainSource.color
                            };
                            try {
                                var solidFile = new File(assetFolder.fsName + "/" + layer.source.name + ".json");
                                solidFile.encoding = "UTF-8";
                                solidFile.open("w");
                                solidFile.write(JSON.stringify(solidInfo, null, 4));
                                solidFile.close();
                            } catch(e) {
                                alert("ソリッド情報の保存中にエラーが発生しました: " + e.message);
                            }
                            break;
                        case "still":
                        case "video":
                        case "audio":
                            var fileName = decodeURI(layer.source.mainSource.file.name);
                            var fileNameLower = fileName.toLowerCase();
                            
                            // Check if it's a PSD or AI file
                            if (fileNameLower.match(/\.(psd|ai)$/)) {
                                try {
                                    debugLog("FileCopy", "Processing PSD/AI file using saveFrameToPng: " + fileName, null, "notice");
                                    
                                    // Get the original filename without extension for the folder name
                                    var baseName = fileName.replace(/\.[^.]+$/, "");
                                    
                                    // Create folder structure: [asset export folder]/[original filename]/
                                    var exportFolder = new Folder(assetFolder.fsName + "/" + baseName);
                                    if (!exportFolder.exists) {
                                        exportFolder.create();
                                    }
                                    
                                    // Get the footage item name (before slash if present)
                                    var footageItemName = layer.source.name;
                                    var slashIndex = footageItemName.indexOf('/');
                                    if (slashIndex !== -1) {
                                        footageItemName = footageItemName.substring(0, slashIndex);
                                    }
                                    
                                    // Create output filename as PNG
                                    var outputFileName = footageItemName + ".png";
                                    var outputFile = new File(exportFolder.fsName + "/" + outputFileName);
                                    
                                    // Check if PNG already exists and source hasn't been modified
                                    var shouldExport = true;
                                    if (outputFile.exists) {
                                        var sourceModified = layer.source.mainSource.file.modified;
                                        var outputModified = outputFile.modified;
                                        shouldExport = sourceModified > outputModified;
                                        if (!shouldExport) {
                                            debugLog("FileCopy", "PSD/AI PNG already up to date, skipping: " + outputFileName, null, "verbose");
                                        }
                                    }
                                    
                                    if (shouldExport) {
                                        // Create temporary composition from PSD/AI footage
                                        var tempCompName = "TempComp_" + layer.source.name + "_" + new Date().getTime();
                                        var newComp = app.project.items.addComp(
                                            tempCompName,
                                            layer.source.width,
                                            layer.source.height,
                                            layer.source.pixelAspect,
                                            1/comp.frameRate, // Single frame duration
                                            comp.frameRate
                                        );
                                        
                                        // Add footage to composition
                                        var newLayer = newComp.layers.add(layer.source);
                                        
                                        // Set composition time to frame 0
                                        newComp.time = 0;
                                        newComp.resolutionFactor = [1, 1];
                                        // Export using saveFrameToPng with proper parameters
                                        newComp.saveFrameToPng(0, outputFile);
                                        
                                        // Clean up temporary composition
                                        newComp.remove();
                                        
                                        debugLog("FileCopy", "PSD/AI exported as PNG: " + outputFileName, null, "notice");
                                    }
                                    
                                    debugLog("FileCopy", "Successfully processed PSD/AI file as PNG: " + outputFile.fsName, null, "notice");
                                    
                                } catch(e) {
                                    debugLog("FileCopy", "Error processing PSD/AI file with saveFrameToPng: " + e.message, null, "error");
                                    alert("PSD/AIファイルのPNG変換中にエラー: " + e.message);
                                }
                            } else {
                                // Regular file handling for non-PSD/AI files
                                var destFile = new File(assetFolder.fsName + "/" + fileName);
                                if (!copyFileWithDateCheck(layer.source.mainSource.file, destFile, fileName, {layerName: layer.name})) {
                                    alert("ファイルのコピー中にエラー: " + fileName);
                                }
                            }
                            break;
                        case "sequence":
                            var extMatch = (""+decodeURI(layer.source.mainSource.file.name)).match(/(\.[^.]+)$/);
                            var ext = extMatch ? extMatch[1].toLowerCase() : "";
                            var sequenceFolder = new Folder(assetFolder.fsName + "/" + layer.source.name);
                            if (!sequenceFolder.exists) sequenceFolder.create();
                            var sequenceFiles = layer.source.mainSource.file.parent.getFiles(function(f){
                                return f instanceof File && ((""+f.name).toLowerCase().indexOf(ext)>=0);
                            }).sort(function(a, b){
                                var A = a.name.toLowerCase();
                                var B = b.name.toLowerCase();
                                return (A < B) ? -1 : (A > B ? 1 : 0);
                            });
                            var fileList = [];
                            for (var s=0; s<sequenceFiles.length; s++){
                                var sequenceFile = sequenceFiles[s];
                                var dest = new File(sequenceFolder.fsName + "/" + sequenceFile.name);
                                if (!copyFileWithDateCheck(sequenceFile, dest, sequenceFile.name, {layerName: layer.name, sequenceIndex: s})) {
                                    alert("ファイルのコピー中にエラー: " + sequenceFile.name);
                                }
                                fileList.push(dest);
                            }
                            
                            try {
                                var metadata = makeSequenceMetadata(fileList, true);
                                var sequenceMetadata = {
                                    fps: getFootageFrameRate(layer.source) || comp.frameRate,
                                    directory: "./" + layer.source.name,
                                    frames: {
                                        list: metadata.uniqueFileList,
                                        indices: metadata.frameMapping
                                    }
                                };

                                var metadataFile = new File(assetFolder.fsName + "/" + layer.source.name + ".json");
                                metadataFile.encoding = "UTF-8";
                                metadataFile.open("w");
                                metadataFile.write(JSON.stringify(sequenceMetadata, null, 4));
                                metadataFile.close();
                                
                                debugLog("SequenceExport", "Created sequence metadata JSON: " + metadataFile.fsName, "notice");
                            } catch(e) {
                                debugLog("SequenceExport", "Error creating sequence metadata: " + e.toString(), null, "error");
                                alert("シーケンスメタデータの作成中にエラー: " + e.message);
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
                
                // Process markers
                debugLog("LayerProcessing", "Checking layer markers...");
                safelyProcessLayerProperty(layer, "markers", function() {
                    if (layer.marker && layer.marker.numKeys > 0) {
                        debugLog("LayerProcessing", "Processing markers for layer: " + layer.name + " (marker count: " + layer.marker.numKeys + ")");
                        resultData["markers"] = extractMarkers(layer.marker, layer.startTime);
                        debugLog("LayerProcessing", "Successfully processed markers for layer: " + layer.name);
                    } else {
                        debugLog("LayerProcessing", "Layer has no markers: " + layer.name);
                    }
                    return true;
                });
                
                // Process properties (non-keyframe)
                debugLog("LayerProcessing", "Processing properties (non-keyframe)...");
                safelyProcessLayerProperty(layer, "properties", function() {
                    options.keyframes = false;
                    var properties = extractPropertiesRecursive(layer, options, layer, layer.startTime);
                    if(properties) {
                        debugLog("LayerProcessing", "Processing properties for layer: " + layer.name);
                        for (var key in properties) {
                            if (properties.hasOwnProperty(key)) {
                                resultData[key] = properties[key];
                            }
                        }
                        debugLog("LayerProcessing", "Successfully processed properties for layer: " + layer.name);
                    } else {
                        debugLog("LayerProcessing", "No properties found for layer: " + layer.name);
                    }
                    return properties;
                });

                // Process keyframes
                debugLog("LayerProcessing", "Processing keyframes...");
                safelyProcessLayerProperty(layer, "keyframes", function() {
                    options.keyframes = true;
                    var keyframes = extractPropertiesRecursive(layer, options, layer, layer.startTime);
                    if(keyframes) {
                        debugLog("LayerProcessing", "Processing keyframes for layer: " + layer.name);
                        resultData["keyframes"] = keyframes;
                        debugLog("LayerProcessing", "Successfully processed keyframes for layer: " + layer.name);
                    } else {
                        debugLog("LayerProcessing", "No keyframes found for layer: " + layer.name);
                    }
                    return keyframes;
                });
    
                if (!safelyProcessLayerProperty(layer, "fileSaving", function() {
                    var jsonString = JSON.stringify(resultData, null, 4);
                    var saveFile = new File(layerFolder.fsName + "/" + layerNameForFile + ".json");
                    saveFile.encoding = "UTF-8";
                    saveFile.open("w");
                    saveFile.write(jsonString);
                    saveFile.close();
                    debugLog("LayerProcessing", "Successfully saved layer file: " + layerNameForFile + ".json");
                    return true;
                }, "error")) {
                    alert("ファイルの保存中にエラーが発生しました: " + layerNameForFile + ".json");
                }
                
                debugLog("LayerProcessing", "=== Completed processing layer " + i + ": " + layer.name + " ===");
                
            } catch (e) {
                debugLog("LayerProcessing", "ERROR: Critical error processing layer " + i + " (" + (layer ? layer.name : "unknown") + "): " + e.toString());
                // Continue with next layer even if this one fails
                continue;
            }
        }

        debugLog("extractPropertiesForAllLayers", "Finished processing " + compInfo["layers"].length + " layers for composition: " + compName);
        
        try {
            var compInfoFile = new File(outputFolderPath + "/" + compName + ".json");
            compInfoFile.encoding = "UTF-8";
            compInfoFile.open("w");
            compInfoFile.write(JSON.stringify(compInfo, null, 4));
            compInfoFile.close();
        } catch(e) {
            alert("コンポジション情報の保存中にエラーが発生しました: " + e.message);
        } finally {
            if (DEBUG_PANEL && DEBUG_PANEL.visible) {
                updateDebugDisplay();
            }
        }
    }

    createUI(me);
})(this);
