# ofxAEPlayer

openFrameworksアドオン。After Effectsコンポジションを書き出し、リアルタイムで再生する。

## 概要

ofxAEPlayerは、After Effectsで作成したコンポジションをJSONファイルとして書き出し、openFrameworksアプリケーション内で再生するためのアドオン。付属のExtendScriptツールでコンポジションデータを抽出し、C++側でレンダリングを行う。

## 主な機能

- After Effectsコンポジションの書き出しと再生
- 基本的な2Dトランスフォーム（位置、スケール、回転、不透明度）
- シェイプレイヤー（楕円、矩形、多角形、パス、グループ、線の高度なプロパティ）
- 画像・動画素材の読み込み
- プリコンポジション（ネストされたコンポジション）
- マスク機能
- トラックマット（アルファ、ルミナンス、反転）
- キーフレームアニメーション（リニア、ベジェ、ホールド補間）
- エクスプレッションのベイク処理
- タイムリマップ
- マーカー（コンポジション、レイヤー）
- 親子階層構造
- エフェクトとテキストの自動ベイク（PNGシーケンス化）
- 共有アセットフォルダによる効率的なアセット管理
- PSD/AIファイルの自動PNG変換
- 画像シーケンスの重複フレーム削減

## システム要件

- openFrameworks 0.12.0以降
- After Effects（コンポジション書き出し用）
- C++17対応コンパイラ

## インストール

1. このリポジトリをopenFrameworksの`addons`ディレクトリにクローンまたは配置

```
of_v0.12.0_osx_release/
  addons/
    ofxAEPlayer/
```

2. プロジェクトの`addons.make`に以下を追加

```
ofxAEPlayer
```

## 使用方法

### 1. After Effectsでの書き出し

1. After Effectsで`tools/ExportComposition.jsx`をスクリプトパネルから実行
2. 書き出し先フォルダを選択
3. 対象のコンポジションを選択して実行
4. JSON形式でコンポジションデータとアセットが出力される

#### 書き出しオプション

書き出しツールには以下のオプションがあります:

##### **基本設定**
- **出力先**: 書き出し先フォルダを指定
- **小数桁**: プロパティ値の小数桁数（デフォルト: 4）

##### **共有アセット設定**
- **共有アセットフォルダを使用**: 複数のコンポジションで共通のアセットフォルダを使用
- **共有アセットパス**: 共有アセットの保存先（相対パスまたは絶対パス）

##### **書き出しモード**
- **フルフレームアニメーションで書き出し**: キーフレーム補間ではなく全フレームを書き出し
- **ネストされたコンポジションを処理**: プリコンポジションを再帰的に書き出し

##### **自動ベイク機能**
- **エフェクト付きレイヤーをシーケンス書き出し**: エフェクトを検出してPNGシーケンスに自動変換
- **テキストレイヤーをシーケンス書き出し**: テキストレイヤーをPNGシーケンスに自動変換
- **シーケンスの同一画像を間引く**: 重複フレームを自動検出して削除

### 2. openFrameworksでの読み込みと再生

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

## ファイル構成

```
ofxAEPlayer/
├── src/                      # C++ソースコード
│   ├── ofxAEPlayer.h        # メインヘッダー
│   ├── core/                # コア機能（Composition, Layer, Mask等）
│   ├── data/                # データ構造（Enums, KeyframeData, PathData等）
│   ├── prop/                # プロパティ（Transform, Shape, Mask等）
│   ├── source/              # ソース（Shape, Solid, Still, Video, Composition等）
│   └── utils/               # ユーティリティ（BlendMode, TrackMatte, AssetManager等）
├── tools/                   # After Effects書き出しツール
│   └── ExportComposition.jsx
├── example/                 # 基本的な使用例
├── example-collision/       # 衝突判定の使用例
├── example-marker/          # マーカーの使用例
└── docs/                    # ドキュメント
```

## API概要

### Composition

コンポジション全体を管理するクラス。

```cpp
namespace ofx::ae {

class Composition {
public:
    // コンポジションの読み込み
    bool load(const std::string& filepath);
    
    // 時間設定と更新
    void setTime(double seconds);
    void update();
    
    // 描画
    void draw(float x, float y);
    
    // コンポジション情報の取得
    const CompositionInfo& getInfo() const;
    
    // レイヤー操作
    std::vector<std::shared_ptr<Layer>> getLayers() const;
    
    // サイズ取得
    float getWidth() const;
    float getHeight() const;
    
    // 再生時間取得
    double getTime() const;
    
    // 再生時間の長さ取得
    double getDuration() const;
};

}
```

### Layer

レイヤーを表すクラス。

```cpp
namespace ofx::ae {

class Layer {
public:
    // レイヤー情報
    std::string getName() const;
    SourceType getSourceType() const;
    
    // サイズ
    float getWidth() const;
    float getHeight() const;
    
    // ソース取得
    std::shared_ptr<LayerSource> getSource() const;
};

}
```

### SourceType

レイヤーのソースタイプ。

```cpp
enum class SourceType {
    SHAPE,          // シェイプレイヤー
    SOLID,          // 平面
    STILL,          // 静止画
    VIDEO,          // 動画
    SEQUENCE,       // 画像シーケンス
    COMPOSITION,    // プリコンポジション
    CAMERA,         // カメラ（未実装）
    LIGHT,          // ライト（未実装）
    TEXT,           // テキスト（未実装）
    ADJUSTMENT,     // 調整レイヤー（未実装）
    NULL_OBJECT     // ヌルオブジェクト（未実装）
};
```

## 機能対応状況

詳細な機能対応状況については [`docs/AE_FEATURE_SUPPORT_STATUS.md`](docs/AE_FEATURE_SUPPORT_STATUS.md) を参照。
(English version: [`docs/AE_FEATURE_SUPPORT_STATUS_EN.md`](docs/AE_FEATURE_SUPPORT_STATUS_EN.md))

### 主な対応機能

- ✅ 基本的な2Dアニメーション
- ✅ シェイプレイヤー（基本図形）
- ✅ 画像・動画素材
- ✅ プリコンポジション
- ✅ マスク
- ✅ トラックマット
- ✅ マーカー
- ✅ 親子関係

### 主な未対応機能

- ❌ 3D機能（カメラ、ライト、3Dレイヤー）
- ⚠️ テキストレイヤー（自動ベイク機能により実質的に使用可能）
- ⚠️ エフェクト（自動ベイク機能により実質的に使用可能）
- ❌ レイヤースタイル
- ❌ 高度な描画モード（基本7種類以外）

## 高度な機能

### 自動ベイクシステム

エフェクトやテキストレイヤーを含むコンポジションでも、書き出しツールが自動的に処理します：

1. **エフェクト自動ベイク**: エフェクトが適用されたレイヤーを検出し、プリコンポーズを作成してPNGシーケンスとしてレンダリング
2. **テキスト自動ベイク**: テキストレイヤーをPNGシーケンスとして自動レンダリング
3. **エクスプレッション自動ベイク**: エクスプレッションを全フレームに展開して書き出し

これらの機能により、手動でのプリレンダリング作業が不要になります。

### レイヤーフィルタリング

書き出し時に、可視レイヤーとその依存関係（親レイヤー、トラックマット）のみを自動的に抽出します。これにより：

- 書き出しファイルサイズの削減
- 読み込み・再生パフォーマンスの向上
- 不要なレイヤーの除外

### 共有アセット管理

複数のコンポジションで同じアセット（画像、動画など）を使用する場合、共有アセットフォルダを使用することで：

- ディスク容量の節約
- アセット管理の簡素化
- 書き出し時間の短縮

## 制限事項

1. **3D機能**: カメラ、ライト、3Dレイヤーは未対応
2. **エフェクト**: 直接的には未対応（自動ベイク機能により実用的に使用可能）
3. **テキストレイヤー**: 直接的には未対応（自動ベイク機能により実用的に使用可能）
4. **描画モード**: 基本的な7種類のみ対応（通常、加算、減算、乗算、スクリーン、比較明、比較暗）
5. **エクスプレッション**: 書き出し時に全フレームにベイク処理される
6. **動画再生**: 環境によって不安定な場合がある
7. **自動ベイク使用時**: エフェクトやテキストのパラメータをリアルタイムに制御できない

## サンプルプロジェクト

### example

基本的なコンポジション再生の例。

```bash
cd example
make
./bin/example
```

### example-collision

シェイプレイヤーを使った衝突判定の例。

### example-marker

マーカーを使ったイベントトリガーの例。

## ライセンス

本プロジェクトは[MITライセンス](LICENSE)の下で公開されています。

## 貢献

バグ報告や機能リクエストはIssueにて受け付けています。

## 参考

- [openFrameworks](https://openframeworks.cc/)
- [After Effects Scripting Guide](https://ae-scripting.docsforadobe.dev/)