var scriptFile = File($.fileName);
var scriptFolder = scriptFile.parent;
$.evalFile(File(scriptFolder.fullName + "/json2.js"));

// ===== PROPERTY MAPPING CONFIGURATION SYSTEM =====

// After Effectsプロパティのマッピング設定 - matchNameごとの完全な処理設定
var PROPERTY_MAPPING_CONFIG = {
    // Layers
    "ADBE AV Layer": {
        assignTo: "layer"
    },
    "ADBE Vector Layer": {
        assignTo: "layer"
    },
    // Transform系
    "ADBE Transform Group": {
        assignTo: "transform"
    },
    "ADBE Anchor Point": {
        assignTo: "anchor"
    },
    "ADBE Position": {
        assignTo: "position"
    },
    "ADBE Position_0": {
        assignTo: "positionX",
        enabled: false
    },
    "ADBE Position_1": {
        assignTo: "positionY",
        enabled: false
    },
    "ADBE Position_2": {
        assignTo: "positionZ",
        enabled: false
    },
    "ADBE Scale": {
        assignTo: "scale"
    },
    "ADBE Orientation": {
        assignTo: "orientation",
        enabled: false
    },
    "ADBE Rotate X": {
        assignTo: "rotateX",
        enabled: false
    },
    "ADBE Rotate Y": {
        assignTo: "rotateY",
        enabled: false
    },
    "ADBE Rotate Z": {
        assignTo: "rotateZ"
    },
    "ADBE Opacity": {
        assignTo: "opacity"
    },

    // Vector Layer系
    "ADBE Root Vectors Group": {
        asArray: true,
        asSourceFile: true
    },
    "ADBE Vector Group": {
        type: "group",
        assignTo: "properties",
        asArray: true
    },
    "ADBE Vector Shape - Ellipse": {
        type: "ellipse",
        inline: true
    },
    "ADBE Vector Shape - Rect": {
        type: "rectangle",
        inline: true
    },
    "ADBE Vector Shape - Star": {
        type: "polygon",
        inline: true
    },
    "ADBE Vector Shape - Group": {
        type: "path",
        inline: true
    },
    
    // Shape系
    "ADBE Vector Ellipse Size": {
        assignTo: "size"
    },
    "ADBE Vector Ellipse Position": {
        assignTo: "position"
    },
    "ADBE Vector Rect Size": {
        assignTo: "size"
    },
    "ADBE Vector Rect Position": {
        assignTo: "position"
    },
    "ADBE Vector Rect Roundness": {
        assignTo: "roundness"
    },
    "ADBE Vector Star Type": {
        assignTo: "type"
    },
    "ADBE Vector Star Points": {
        assignTo: "points"
    },
    "ADBE Vector Star Position": {
        assignTo: "position"
    },
    "ADBE Vector Star Outer Radius": {
        assignTo: "outerRadius"
    },
    "ADBE Vector Star Inner Radius": {
        assignTo: "innerRadius"
    },
    "ADBE Vector Star Outer Roundess": {
        assignTo: "outerRoundess"
    },
    "ADBE Vector Star Inner Roundess": {
        assignTo: "innerRoundess"
    },
    "ADBE Vector Star Rotation": {
        assignTo: "rotation"
    },
    
    "ADBE Vector Shape Direction": {
        assignTo: "direction"
    },
    "ADBE Vector Blend Mode": {
        assignTo: "blendMode"
    },
    "ADBE Vector Composite Order": {
        assignTo: "compositeOrder"
    },

    // Fill/Stroke系
    "ADBE Vector Graphic - Fill": {
        assignTo: "fill",
        inline: true
    },
    "ADBE Vector Fill Rule": {
        assignTo: "rule"
    },
    "ADBE Vector Fill Color": {
        assignTo: "color"
    },
    "ADBE Vector Fill Opacity": {
        assignTo: "opacity"
    },
    "ADBE Vector Graphic - Stroke": {
        assignTo: "stroke",
        inline: true
    },
    "ADBE Vector Stroke Color": {
        assignTo: "color"
    },
    "ADBE Vector Stroke Opacity": {
        assignTo: "opacity"
    },
    "ADBE Vector Stroke Width": {
        assignTo: "width"
    },
    "ADBE Vector Stroke Line Cap": {
        assignTo: "lineCap"
    },
    "ADBE Vector Stroke Line Join": {
        assignTo: "lineJoin"
    },
    "ADBE Vector Stroke Miter Limit": {
        assignTo: "miterLimit"
    },
    "ADBE Vector Stroke Dashes": {
        assignTo: "dashes"
    },
    "ADBE Vector Stroke Taper": {
        assignTo: "taper"
    },
    "ADBE Vector Stroke Wave": {
        assignTo: "wave"
    },
    "ADBE Vector Filter - Trim": {
        assignTo: "properties"
    },
    "ADBE Vector Filter - Trim Start": {
        assignTo: "start"
    },
    "ADBE Vector Filter - Trim End": {
        assignTo: "end"
    },
    "ADBE Vector Filter - Trim Offset": {
        assignTo: "offset"
    },

    // 不要なプロパティを無効化
    "ADBE Audio Group": { enabled: false },
    "ADBE Data Group": { enabled: false },
    "ADBE Layer Overrides": { enabled: false },
    "ADBE Layer Sets": { enabled: false },
    "ADBE Envir Appear in Reflect": { enabled: false },
    "ADBE Layer Styles": { enabled: false },
    "ADBE Plane Options Group": { enabled: false },
    "ADBE Extrsn Options Group": { enabled: false },
    "ADBE Material Options Group": { enabled: false },
    "ADBE Source Options Group": { enabled: false },
    "ADBE Marker": { enabled: false },
    "ADBE Mask Parade": { enabled: false },
    "ADBE Effect Parade": { enabled: false },
    "ADBE Time Remapping": { enabled: false },
    "ADBE MTrackers": { enabled: false }
};


(function(me){
    // polyfills
    if (typeof String.prototype.trim !== 'function') {
        String.prototype.trim = function(){ return this.replace(/^\s+|\s+$/g, ''); };
    }
    if (typeof String.prototype.fsSanitized !== 'function') {
        String.prototype.fsSanitized = function(){ return this.replace(/[\\\/\:\*\?\"\<\>\|]/g, "_"); };
    }
    if (typeof Object.prototype.keys !== 'function') {
        Object.prototype.keys = function(){
            var keys = [];
            for (var k in this){ if (this.hasOwnProperty(k)) keys.push(k); }
            return keys;
        };
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
    if (typeof Array.isArray !== 'function') {
        Array.isArray = function(arg){ return Object.prototype.toString.call(arg) === '[object Array]'; };
    }

    function getSourceType(layer) {
        switch(layer.matchName) {
            case "ADBE Vector Layer":
                return "shape"; // シェイプレイヤー
        }
        if (!layer.source) {
            return "none"; // ソースなし（シェイプレイヤーなど）
        }
        
        // コンポジション
        if (layer.source instanceof CompItem) {
            return "composition";
        }
        
        // フッテージ（ファイルベース）
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
        
        // 平面
        if (layer.source.mainSource instanceof SolidSource) {
            return "solid";
        }
        
        return "unknown";
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


        // オプショングループ
        var optionGroup = myPanel.grp.add("group");
        optionGroup.orientation = "column";
        optionGroup.alignment = "fill";

        var topRow = optionGroup.add("group");
        topRow.orientation = "row";
        topRow.alignment = "fill";

        var exportNestedCompCheck = topRow.add("checkbox", undefined, "ネストされたコンポジションを再帰的に出力");
        exportNestedCompCheck.name = "exportNestedCompCheck";
        exportNestedCompCheck.value = false;
        exportNestedCompCheck.alignment = "left";

        var decimalPlacesGroup = topRow.add("group");
        decimalPlacesGroup.orientation = "row";
        decimalPlacesGroup.alignment = "right";

        var decLabel = decimalPlacesGroup.add("statictext", undefined, "小数桁:");
        var decimalPlacesText = decimalPlacesGroup.add("edittext", undefined, "4");
        decimalPlacesText.name = "decimalPlacesText";
        decimalPlacesText.preferredSize.width = 40;

        var recordUnsupportedPropsCheck = optionGroup.add("checkbox", undefined, "未対応プロパティを記録");
        recordUnsupportedPropsCheck.name = "recordUnsupportedPropsCheck";
        recordUnsupportedPropsCheck.value = false;
        recordUnsupportedPropsCheck.alignment = "left";

        var useKeyframeExtractionCheck = optionGroup.add("checkbox", undefined, "キーフレーム抽出を使用（推奨：高速で詳細な補間情報付き）");
        useKeyframeExtractionCheck.name = "useKeyframeExtractionCheck";
        useKeyframeExtractionCheck.value = true;
        useKeyframeExtractionCheck.alignment = "left";

        // 保存グループ
        var saveGroup = myPanel.grp.add("group");
        saveGroup.orientation = "column";
        saveGroup.alignment = "fill";

        var outputGroup = saveGroup.add("group");
        outputGroup.orientation = "row";
        outputGroup.alignment = "fill";

        var outputPathText = outputGroup.add("edittext");
        outputPathText.name = "outputPathText";
        outputPathText.text = "";
        outputPathText.alignment = "fill";

        var selectOutputFolderButton = outputGroup.add("button", undefined, "選択");
        selectOutputFolderButton.alignment = "right";

        var footageGroup = saveGroup.add("group");
        footageGroup.orientation = "row";
        footageGroup.alignment = "fill";

        var footageLabelText = footageGroup.add("statictext", undefined, "フッテージパス（default: 出力先/comp名/footages）");
        footageLabelText.alignment = "left";

        var footagePathText = footageGroup.add("edittext");
        footagePathText.name = "footagePathText";
        footagePathText.text = "";
        footagePathText.alignment = "fill";

        var selectFootageFolderButton = footageGroup.add("button", undefined, "選択");
        selectFootageFolderButton.alignment = "right";

        // ボタングループ
        var buttonGroup = myPanel.grp.add("group");
        buttonGroup.orientation = "row";
        buttonGroup.alignment = "fill";

        var executeButton = buttonGroup.add("button", undefined, "実行");
        executeButton.alignment = "fill";

        var resetButton = buttonGroup.add("button", undefined, "デフォルトに戻す");
        resetButton.alignment = "fill";
        
        var debugButton = buttonGroup.add("button", undefined, "Debug");
        debugButton.alignment = "fill";

        
        // デバッグパネルを作成
        var debugPanel = createDebugPanel(myPanel.grp);

        // 設定を読み込み
        loadSettings(myPanel.grp);

        selectOutputFolderButton.onClick = function(){
            var selectedFolder = Folder.selectDialog("出力先のフォルダを選択してください");
            if (selectedFolder){ outputPathText.text = decodeURI(selectedFolder.fsName); saveSettings(myPanel.grp); }
        };
        selectFootageFolderButton.onClick = function(){
            var selectedFolder = Folder.selectDialog("フッテージの保存先フォルダを選択してください");
            if (selectedFolder){ footagePathText.text = decodeURI(selectedFolder.fsName); saveSettings(myPanel.grp); }
        };

        executeButton.onClick = function(){
            var undoOpen=false;
            try{
                // 実行ボタンを押すたびにログをクリア
                clearDebugLogs();
                
                var outputFolderPath  = outputPathText.text.trim();
                var footageFolderPath = footagePathText.text.trim();
                
                var decPlacesText = decimalPlacesText.text;
                var decPlaces = Math.max(0, Math.min(10, parseInt(decPlacesText,10) || 4));

                if (outputFolderPath===''){ alert("出力先のフォルダを指定してください。"); return; }
                if (app.project.activeItem==null || !(app.project.activeItem instanceof CompItem)){ alert("コンポジションを選択してください。"); return; }

                var procNestedComp = exportNestedCompCheck.value;

                debugLog("ExecuteSystem", "Starting property extraction process", {
                    outputFolder: outputFolderPath,
                    footageFolder: footageFolderPath,
                    nestedComp: procNestedComp,
                    decimalPlaces: decPlaces
                }, "notice");

                app.beginUndoGroup("プロパティ抽出"); undoOpen=true;
                var options = {
                    outputFolderPath: outputFolderPath,
                    footageFolderPath: footageFolderPath,
                    procNestedComp: procNestedComp,
                    decimalPlaces: decPlaces,
                    recordUnsupportedProps: recordUnsupportedPropsCheck.value,
                    useKeyframeExtraction: useKeyframeExtractionCheck.value
                };
                extractPropertiesForAllLayers(app.project.activeItem, options);

                saveSettings(myPanel.grp);
                debugLog("ExecuteSystem", "Property extraction completed successfully", null, "notice");
                alert("全てのレイヤーのプロパティを抽出し、指定されたフォルダに保存しました。");
            }catch(e){
                debugLog("ExecuteSystem", "Error during execution: " + e.message, e, "error");
                alert("エラーが発生しました: " + e.message);
            }finally{
                if (undoOpen) app.endUndoGroup();
            }
        };

        resetButton.onClick = function(){
            if (confirm("設定をデフォルトに戻しますか？\nこの操作は取り消せません。")) resetSettings(myPanel.grp);
        };
        
        debugButton.onClick = function(){
            if (debugPanel) {
                debugPanel.visible = !debugPanel.visible;
                DEBUG_ENABLED = debugPanel.visible;
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
        });
    }
    
    function addChangeEventListeners(parentGroup){
        traverseUIElements(parentGroup, function(child, parentGroup) {
            if (child.name){
                if (child instanceof EditText) (function(c){ c.onChange = function(){ saveSettings(parentGroup); }; })(child);
                else if (child instanceof Checkbox) (function(c){ c.onClick = function(){ saveSettings(parentGroup); }; })(child);
                else if (child instanceof DropDownList) (function(c){ c.onChange = function(){ saveSettings(parentGroup); }; })(child);
            }
        });
    }
    
    function resetSettings(parentGroup){
        traverseUIElements(parentGroup, function(child) {
            if (child.name){
                if (child instanceof EditText){
                    if (child.name === 'decimalPlacesText') child.text = '4';
                }else if (child instanceof Checkbox){
                    child.value = false;
                }else if (child instanceof DropDownList){
                    child.selection = null;
                }
            }
        });
        saveSettings(parentGroup);
        parentGroup.layout.layout(true);
    }

    // ===== path util =====
    function getRelativePath(fromPath, toPath){
        var fromFsName = decodeURI(fromPath.fsName).replace(/\\/g,'/');
        var toFsName   = decodeURI(toPath.fsName).replace(/\\/g,'/');
        var fromParts = fromFsName.split('/'), toParts = toFsName.split('/');
        for (var i=0;i<fromParts.length;i++) fromParts[i]=fromParts[i].toLowerCase();
        for (var j=0;j<toParts.length;j++) toParts[j]=toParts[j].toLowerCase();
        var length = Math.min(fromParts.length, toParts.length);
        var samePartsLength = length;
        for (var k=0;k<length;k++){ if (fromParts[k]!=toParts[k]){ samePartsLength=k; break; } }
        var outputParts=[];
        for (var a=samePartsLength; a<fromParts.length; a++) outputParts.push("..");
        for (var b=samePartsLength; b<toParts.length; b++) outputParts.push(toParts[b]);
        return outputParts.join("/");
    }

    function layerUniqueName(layer){ return layer.name.fsSanitized() + "(ID_" + layer.id + ")"; }

    // ===== VALUE PROCESSING SYSTEM =====
    
    // キーフレーム補間タイプ定数
    var KEYFRAME_INTERPOLATION_TYPES = {
        6612: "LINEAR",        // KeyframeInterpolationType.LINEAR
        6613: "BEZIER",        // KeyframeInterpolationType.BEZIER
        6614: "HOLD"           // KeyframeInterpolationType.HOLD
    };
    
    // プロパティの有効状態を判定
    function isPropertyEnabled(config) {
        return (config.enabled !== undefined) ? config.enabled : true;
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
                interpolation.inType = KEYFRAME_INTERPOLATION_TYPES[inType] || "LINEAR";
                interpolation.outType = KEYFRAME_INTERPOLATION_TYPES[outType] || "LINEAR";
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
    function extractValue(prop, time, decimalPlaces) {
        try {
            if (!prop) return null;
            
            var value = prop.valueAtTime(time, false);
            
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
    var DEBUG_ENABLED = false;
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
    function extractMarkers(markers, fps, offsetFrame){
        var markerData = [];
        for (var i = 1; i <= markers.numKeys; i++) {
            var time = markers.keyTime(i);
            var markerValue = markers.keyValue(i);
            var frame = Math.floor(time * fps) - offsetFrame;
            var lengthFrames = Math.floor(markerValue.duration * fps);
            
            var markerInfo = {
                frame: frame,
                comment: markerValue.comment || "",
                length: lengthFrames
            };
            
            markerData.push(markerInfo);
        }
        
        return markerData;
    }
    function extractCompMarkers(comp){
        var markers = comp.markerProperty;
        var workAreaStartFrame = Math.floor(comp.workAreaStart * comp.frameRate);
        return extractMarkers(markers, comp.frameRate, workAreaStartFrame);
    }

    function extractLayerMarkers(layer){
        var markers = layer.marker;
        var inPointFrame = Math.floor(layer.inPoint * layer.containingComp.frameRate);
        return extractMarkers(markers, layer.containingComp.frameRate, inPointFrame);
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



    // キーフレームベースの抽出関数
    function extractKeyframeBasedProperty(prop, inPoint, duration, fps, decimalPlaces, valueCount) {
        try {
            if (!prop) {
                debugLog("extractKeyframeBasedProperty", "prop is null");
                return null;
            }
            
            debugLog("extractKeyframeBasedProperty", "Processing property: " + (prop.matchName || prop.name || "unnamed"));
            
            function toTime(f) { return f / fps; }
            function toFrame(t) { return Math.floor(t * fps); }
            
            if (!valueCount || valueCount === 0) {
                var sampleValue = prop.valueAtTime(toTime(inPoint), false);
                valueCount = (sampleValue instanceof Array) ? sampleValue.length : 1;
                debugLog("extractKeyframeBasedProperty", "Auto-detected valueCount: " + valueCount);
            }
            
            var isSingleValue = (valueCount === 1);
            var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
            
            // Get initial value
            var initialValue = extractValue(prop, toTime(inPoint), decimalPlaces);
            
            if (initialValue instanceof Array) {
                initialValue = initialValue.slice(0, valueCount);
                while (initialValue.length < valueCount) initialValue.push(null);
            } else if (valueCount > 1) {
                var temp = [initialValue];
                while (temp.length < valueCount) temp.push(null);
                initialValue = temp;
            }
            
            if (isSingleValue && initialValue instanceof Array) {
                initialValue = initialValue[0];
            }
            
            // Return structured data with initial value and keyframe data
            var result = {
                initialValue: initialValue,
                hasAnimation: (nk > 0),
                extractionMethod: "keyframe"
            };
            
            if (nk === 0) {
                // No animation - only return initial value info
                debugLog("extractKeyframeBasedProperty", "No animation detected, returning initial value only");
                return result;
            }
            
            // Extract keyframe data
            var keyframes = [];
            
            for (var i = 1; i <= nk; i++) {
                try {
                    var keyTime = prop.keyTime(i);
                    var keyFrame = toFrame(keyTime) - inPoint;
                    
                    // キーフレームがレイヤーの有効範囲内かチェック
                    // if (keyFrame < 0 || keyFrame > duration) {
                    //     continue;
                    // }
                    
                    var keyValue = extractValue(prop, keyTime, decimalPlaces);
                    
                    // 値の正規化
                    if (keyValue instanceof Array) {
                        keyValue = keyValue.slice(0, valueCount);
                        while (keyValue.length < valueCount) keyValue.push(null);
                    } else if (valueCount > 1) {
                        var temp = [keyValue];
                        while (temp.length < valueCount) temp.push(null);
                        keyValue = temp;
                    }
                    
                    if (isSingleValue && keyValue instanceof Array) {
                        keyValue = keyValue[0];
                    }
                    
                    // キーフレーム情報を構築
                    var keyframeInfo = {
                        frame: keyFrame,
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
                    debugLog("extractKeyframeBasedProperty", "Error extracting keyframe " + i + ": " + e.toString(), null, "warning");
                }
            }
            
            result.keyframes = keyframes;
            
            debugLog("extractKeyframeBasedProperty", "Keyframe extraction completed successfully", {
                keyframeCount: keyframes.length,
                hasAnimation: result.hasAnimation
            });
            return result;
        } catch (e) {
            debugLog("extractKeyframeBasedProperty", "Error in keyframe extraction: " + e.toString(), null, "error");
            return null;
        }
    }
    
    function extractAnimatedProperty(prop, inPoint, duration, fps, decimalPlaces, valueCount) {
        try {
            if (!prop) {
                debugLog("extractAnimatedProperty", "prop is null");
                return null;
            }
            
            debugLog("extractAnimatedProperty", "Processing property: " + (prop.matchName || prop.name || "unnamed"));
            
            function toTime(f) { return f / fps; }
            
            if (!valueCount || valueCount === 0) {
                var sampleValue = prop.valueAtTime(toTime(inPoint), false);
                valueCount = (sampleValue instanceof Array) ? sampleValue.length : 1;
                debugLog("extractAnimatedProperty", "Auto-detected valueCount: " + valueCount);
            }
            
            var isSingleValue = (valueCount === 1);
            var nk = (typeof prop.numKeys === 'number') ? prop.numKeys : 0;
            
            // Get initial value
            var initialValue = extractValue(prop, toTime(inPoint), decimalPlaces);
            
            if (initialValue instanceof Array) {
                initialValue = initialValue.slice(0, valueCount);
                while (initialValue.length < valueCount) initialValue.push(null);
            } else if (valueCount > 1) {
                var temp = [initialValue];
                while (temp.length < valueCount) temp.push(null);
                initialValue = temp;
            }
            
            if (isSingleValue && initialValue instanceof Array) {
                initialValue = initialValue[0];
            }
            
            // Return structured data with initial value and animation data
            var result = {
                initialValue: initialValue,
                hasAnimation: (nk > 0)
            };
            
            if (nk === 0) {
                // No animation - only return initial value info
                debugLog("extractAnimatedProperty", "No animation detected, returning initial value only");
                return result;
            }
            
            // Extract animation data (keyframes)
            var animationData = {};
            var prevValue = null;
            var groupStartFrame = null;
            var groupValues = [];
            
            for (var frame = 0; frame <= duration; frame++) {
                var time = toTime(frame + inPoint);
                var extractedValue = extractValue(prop, time, decimalPlaces);
                
                if (extractedValue instanceof Array) {
                    extractedValue = extractedValue.slice(0, valueCount);
                    while (extractedValue.length < valueCount) extractedValue.push(null);
                } else if (valueCount > 1) {
                    var temp = [extractedValue];
                    while (temp.length < valueCount) temp.push(null);
                    extractedValue = temp;
                }
                
                if (isSingleValue && extractedValue instanceof Array) {
                    extractedValue = extractedValue[0];
                }
                
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
            
            result.animationData = animationData;
            
            debugLog("extractAnimatedProperty", "Extraction completed successfully", {
                frameCount: animationData.keys ? animationData.keys().length : 0,
                hasAnimation: result.hasAnimation
            });
            return result;
        } catch (e) {
            debugLog("extractAnimatedProperty", "Error in extraction: " + e.toString(), null, "error");
            return null;
        }
    }
    
    // ===== 統一処理関数 =====
    function extractPropertiesRecursive(property, options, layer) {
        try {
            if (!property) {
                debugLog("extractPropertiesRecursive", "property is null", null, "warning");
                return null;
            }
            
            var config = PROPERTY_MAPPING_CONFIG[property.matchName];
            if (!config) {
                debugLog("extractPropertiesRecursive", "No config found for matchName: " + property.matchName, null, "warning");
                return null;
            }
            
            if (!isPropertyEnabled(config)) {
                debugLog("extractPropertiesRecursive", "Property disabled: " + property.matchName, null, "verbose");
                return null;
            }
            
            debugLog("extractPropertiesRecursive", "Processing property: " + property.matchName);
            
            var fps = layer.containingComp.frameRate;
            var inPoint = Math.floor(layer.inPoint * fps);
            var outPoint = Math.floor(layer.outPoint * fps);
            var duration = outPoint - inPoint;
            var DEC = options.decimalPlaces || 4;
            
            var result = {};
            var keyframes = {}; // Collect keyframes for animated properties
            
            if (config.type) {
                result.type = config.type;
            }
            
            if (property.valueAtTime && typeof property.valueAtTime === 'function') {
                var extractedData;
                
                // Choose extraction method based on options
                if (options.useKeyframeExtraction) {
                    // Use keyframe-based extraction
                    extractedData = extractKeyframeBasedProperty(property, inPoint, duration, fps, DEC, 0);
                    if (extractedData && config.assignTo) {
                        if (extractedData.hasAnimation && extractedData.keyframes) {
                            // Property has animation - store initial value directly and keyframes
                            result[config.assignTo] = extractedData.initialValue;
                            
                            // Transform系プロパティの場合は階層構造を作る
                            var parentMatchName = property.parentProperty ? property.parentProperty.matchName : "";
                            if (parentMatchName === "ADBE Transform Group") {
                                if (!keyframes["transform"]) {
                                    keyframes["transform"] = {};
                                }
                                keyframes["transform"][config.assignTo] = extractedData.keyframes;
                            } else {
                                // その他のプロパティは直接配置
                                keyframes[config.assignTo] = extractedData.keyframes;
                            }
                        } else {
                            // Property has no animation - store initial value directly
                            result[config.assignTo] = extractedData.initialValue;
                        }
                    }
                } else {
                    // Use full-frame extraction (legacy)
                    extractedData = extractAnimatedProperty(property, inPoint, duration, fps, DEC, 0);
                    if (extractedData && config.assignTo) {
                        if (extractedData.hasAnimation && extractedData.animationData) {
                            // Property has animation - store initial value directly and animation in keyframes
                            result[config.assignTo] = extractedData.initialValue;
                            
                            // Transform系プロパティの場合は階層構造を作る
                            var parentMatchName = property.parentProperty ? property.parentProperty.matchName : "";
                            if (parentMatchName === "ADBE Transform Group") {
                                if (!keyframes["transform"]) {
                                    keyframes["transform"] = {};
                                }
                                keyframes["transform"][config.assignTo] = extractedData.animationData;
                            } else {
                                // その他のプロパティは直接配置
                                keyframes[config.assignTo] = extractedData.animationData;
                            }
                        } else {
                            // Property has no animation - store initial value directly
                            result[config.assignTo] = extractedData.initialValue;
                        }
                    }
                }
            }
            
            if (property.numProperties && property.numProperties > 0) {
                var childResults = null;
                var as_array = config.asArray;
                if (as_array) {
                    childResults = [];
                    
                    for (var i = 1; i <= property.numProperties; i++) {
                        var childProp = property.property(i);
                        if (childProp) {
                            var childConfig = PROPERTY_MAPPING_CONFIG[childProp.matchName];
                            var childResult = extractPropertiesRecursive(childProp, options, layer);
                            if (childResult) {
                                var arrayItem = {};
                                
                                if (childConfig && childConfig.type) {
                                    arrayItem.type = childConfig.type;
                                }
                                
                                if (childConfig && childConfig.assignTo) {
                                    arrayItem[childConfig.assignTo] = childResult[childConfig.assignTo] || childResult;
                                } else {
                                    for (var key in childResult) {
                                        if (childResult.hasOwnProperty(key) && key !== "keyframes") {
                                            arrayItem[key] = childResult[key];
                                        }
                                    }
                                }
                                
                                // Merge child keyframes
                                if (childResult.keyframes) {
                                    for (var kfKey in childResult.keyframes) {
                                        if (childResult.keyframes.hasOwnProperty(kfKey)) {
                                            keyframes[kfKey] = childResult.keyframes[kfKey];
                                        }
                                    }
                                }
                                
                                childResults.push(arrayItem);
                            }
                        }
                    }
                } else {
                    childResults = {};
                    
                    for (var i = 1; i <= property.numProperties; i++) {
                        var childProp = property.property(i);
                        if (childProp) {
                            var childResult = extractPropertiesRecursive(childProp, options, layer);
                            if (childResult) {
                                for (var key in childResult) {
                                    if (childResult.hasOwnProperty(key) && key !== "keyframes") {
                                        childResults[key] = childResult[key];
                                    }
                                }
                                
                                // Merge child keyframes
                                if (childResult.keyframes) {
                                    for (var kfKey in childResult.keyframes) {
                                        if (childResult.keyframes.hasOwnProperty(kfKey)) {
                                            keyframes[kfKey] = childResult.keyframes[kfKey];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if(config.assignTo) {
                    result[config.assignTo] = childResults;
                }
                else if (config.inline) {
                    if(as_array) {
                        debugLog("extractPropertiesRecursive", "Cannot inline array into non-array result", null, "warning");
                    }
                    else {
                        var keys = childResults.keys();
                        for (var i = 0; i < keys.length; i++) {
                            var key = keys[i];
                            result[key] = childResults[key];
                        }
                    }
                }
                if(config.asSourceFile) {
                    var outputFolderPath  = options.outputFolderPath;
                    var outputFolder  = new Folder(outputFolderPath + "/" + layer.containingComp.name.fsSanitized());
                    var footageFolderPath = options.footageFolderPath || (outputFolder.fsName + "/footages");
                    var footageFolder = new Folder(footageFolderPath);
                    var fileName = layerUniqueName(layer) + ".json";
                    var jsonString = JSON.stringify(childResults, null, 4);
                    var saveFile = new File(footageFolder.fsName + "/" + fileName);
                    saveFile.encoding = "UTF-8";
                    saveFile.open("w");
                    saveFile.write(jsonString);
                    saveFile.close();
                }
            }

            // Add keyframes object if there are any animated properties
            if (keyframes.keys().length > 0) {
                result.keyframes = keyframes;
            }

            debugLog("extractPropertiesRecursive", "Processing completed for: " + property.matchName);
            return result;
        } catch (e) {
            debugLog("extractPropertiesRecursive", "Error in processing: " + e.toString(), null, "error");
            return null;
        }
    }
    
    // ===== main extractor =====
    function extractPropertiesForAllLayers(comp, options){
        var compName = comp.name.fsSanitized();

        var outputFolderPath  = options.outputFolderPath;
        var outputFolder  = new Folder(outputFolderPath + "/" + compName);
        var layerFolder   = new Folder(outputFolder.fsName + "/layers");
        var footageFolderPath = options.footageFolderPath || (outputFolder.fsName + "/footages");
        var footageFolder = new Folder(footageFolderPath);

        var procNestedComp    = options.procNestedComp;
        var DEC               = options.decimalPlaces || 4;


        if (!outputFolder.exists && !outputFolder.create()){ alert("指定された出力先フォルダを作成できませんでした。" + outputFolder.fsName); return; }
        if (!footageFolder.exists && !footageFolder.create()){ alert("指定されたフッテージ保存先フォルダを作成できませんでした。" + footageFolder.fsName); return; }
        if (!layerFolder.exists) layerFolder.create();

        var fps = comp.frameRate;
        function toFrame(t, floor){ return floor ? Math.floor(t * fps) : t * fps; }

        var compInfo = {};
        compInfo["duration"]      = toFrame(comp.duration, true);
        compInfo["fps"]           = fps;
        compInfo["width"]         = comp.width;
        compInfo["height"]        = comp.height;
        compInfo["startFrame"]    = toFrame(comp.workAreaStart, true);
        compInfo["endFrame"]      = toFrame((comp.workAreaStart + comp.workAreaDuration), true);
        compInfo["layers"]        = [];
        // compInfo["layersDirectory"] = getRelativePath(outputFolder, layerFolder);
        // compInfo["footageDirectory"] = getRelativePath(outputFolder, footageFolder);
        
        // コンポジションマーカーを追加
        var compMarkers = extractCompMarkers(comp);
        if (compMarkers.length > 0) {
            compInfo["markers"] = compMarkers;
        }

        for (var i=1;i<=comp.numLayers;i++){
            var layer = comp.layer(i);
            if (!layer.enabled) continue;

            var resultData = {};
            resultData["name"] = layer.name;
            resultData["layerType"] = layer.matchName;
            var sourceType = getSourceType(layer);
            resultData["sourceType"] = sourceType;
            
            switch(sourceType) {
                case "shape":
                    resultData["source"] = getRelativePath(layerFolder, footageFolder) + "/" + layerUniqueName(layer) + ".json";
                    break;
                default:
                    if (layer.source) {
                        var sourceName = getRelativePath(layerFolder, footageFolder) + "/" + layer.source.name;
                        if (sourceType === "composition") {
                            sourceName = sourceName + "/comp.json";
                        }
                        resultData["source"] = sourceName;
                    }
                    break;
            }

            var inPoint  = toFrame(layer.inPoint, true);
            var outPoint = toFrame(layer.outPoint, true);
            resultData["in"]  = inPoint;
            resultData["out"] = outPoint;

            if (layer.parent) resultData["parent"] = layerUniqueName(layer.parent);

    
            // レイヤーマーカーを追加
            var layerMarkers = extractLayerMarkers(layer);
            if (layerMarkers.length > 0) {
                resultData["markers"] = layerMarkers;
            }

            // レイヤー固有処理の統一化
            var extractedData = extractPropertiesRecursive(layer, options, layer);
            if(extractedData && extractedData.keys().length > 0) {
                for (var key in extractedData) {
                    if (extractedData.hasOwnProperty(key)) {
                        resultData[key] = extractedData[key];
                    }
                }
            }
    
            var layerNameForFile = layerUniqueName(layer);
            compInfo["layers"].push({
                name: layer.name,
                uniqueName: layerNameForFile,
                file: getRelativePath(outputFolder, layerFolder) + "/" + layerNameForFile + ".json"
            });

            try{
                var jsonString = JSON.stringify(resultData, null, 4);
                var saveFile = new File(layerFolder.fsName + "/" + layerNameForFile + ".json");
                saveFile.encoding = "UTF-8";
                saveFile.open("w");
                saveFile.write(jsonString);
                saveFile.close();
            }catch(e){
                alert("ファイルの保存中にエラーが発生しました: " + e.message);
            }

            // フッテージコピーとネスト再帰
            switch(sourceType){
                case "image":
                case "sequence":
                case "video":
                case "audio":
                    break;
                default:
                    continue; // フッテージコピーとネスト再帰はこれらのタイプのみ
            }
            if (layer.source){
                if (layer.source.mainSource instanceof FileSource){
                    var sourceFile = layer.source.mainSource.file;
                    if (sourceFile && sourceFile.exists){
                        if (layer.source.mainSource.isStill){
                            var destFile = new File(footageFolder.fsName + "/" + sourceFile.name);
                            try{ sourceFile.copy(destFile); }catch(e){ alert("ファイルのコピー中にエラー: " + e.message); }
                        }else{
                            var sourceType = getSourceType(layer);
                            
                            if (sourceType === "sequence") {
                                // 画像シーケンスの場合：連番ファイルをすべてコピー
                                var extMatch = (""+sourceFile.name).match(/(\.[^.]+)$/);
                                var ext = extMatch ? extMatch[1].toLowerCase() : "";
                                var sequenceFolder = new Folder(footageFolder.fsName + "/" + layer.source.name);
                                if (!sequenceFolder.exists) sequenceFolder.create();
                                var sequenceFiles = sourceFile.parent.getFiles(function(f){
                                    return f instanceof File && ((""+f.name).toLowerCase().indexOf(ext)>=0);
                                });
                                for (var s=0; s<sequenceFiles.length; s++){
                                    var sequenceFile = sequenceFiles[s];
                                    var dest = new File(sequenceFolder.fsName + "/" + sequenceFile.name);
                                    try{ sequenceFile.copy(dest); }catch(e){ alert("ファイルのコピー中にエラー: " + e.message); }
                                }
                            } else {
                                // 単一動画・音声ファイルの場合：そのファイルのみをコピー
                                var destFile = new File(footageFolder.fsName + "/" + sourceFile.name);
                                try{
                                    sourceFile.copy(destFile);
                                }catch(e){
                                    alert("ファイルのコピー中にエラー: " + e.message);
                                }
                            }
                        }
                    }
                }
                if (procNestedComp && layer.source instanceof CompItem){
                    var nestedOptions = {
                        outputFolderPath: footageFolderPath,
                        footageFolderPath: options.footageFolderPath,
                        procNestedComp: true,
                        decimalPlaces: DEC
                    };
                    extractPropertiesForAllLayers(layer.source, nestedOptions);
                }
            }
        }

        try{
            var compInfoFile = new File(outputFolder.fsName + "/comp.json");
            compInfoFile.encoding = "UTF-8";
            compInfoFile.open("w");
            compInfoFile.write(JSON.stringify(compInfo, null, 4));
            compInfoFile.close();
        }catch(e){
            alert("コンポジション情報の保存中にエラーが発生しました: " + e.message);
        }
        finally{
            if (DEBUG_PANEL && DEBUG_PANEL.visible) {
                updateDebugDisplay();
            }
        }
    }

    createUI(me);
})(this);