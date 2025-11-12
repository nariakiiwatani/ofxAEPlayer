# フレーム補間実装の比較分析

## 問題の定義

15fps で作成された AE コンポジションを 30fps の oF アプリケーションで再生する際、現在の実装では整数フレーム番号のみを使用するため、0,0,1,1,2,2,3,3... のように同じフレームが繰り返し表示される。

**目標:** 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5... のようにキーフレーム間を補間して滑らかな再生を実現する。

---

## 現在の実装の詳細分析

### コードベース構造

**主要コンポーネント:**

1. **[`ofxAEComposition`](src/core/ofxAEComposition.h:43)** - コンポジション全体
   - [`setFrame(int frame)`](src/core/ofxAEComposition.cpp:106)
   - [`current_frame_`](src/core/ofxAEComposition.h:64) (int型)
   - [`getCurrentTime()`](src/core/ofxAEComposition.h:50) - `current_frame_/info_.fps` で計算

2. **[`ofxAELayer`](src/core/ofxAELayer.h:42)** - 個別レイヤー
   - [`setFrame(int frame)`](src/core/ofxAELayer.cpp:203)
   - [`current_frame_`](src/core/ofxAELayer.h:88) (int型)
   - transform, mask, source の更新を管理

3. **[`Property<T>`](src/prop/ofxAEProperty.h:159)** - プロパティアニメーション
   - [`setFrame(int frame)`](src/prop/ofxAEProperty.h:159)
   - キーフレームデータ: `std::map<int, ofxAEKeyframe<T>>`
   - [`findKeyframePair()`](src/prop/ofxAEKeyframe.h:176) - 補間用のキーフレームペアを検索
   - [`interpolateKeyframe()`](src/prop/ofxAEKeyframe.h:234) - 実際の補間計算

4. **Source系** - メディアソース
   - [`CompositionSource::setFrame(int)`](src/source/ofxAECompositionSource.cpp:33)
   - [`VideoSource::setFrame(int)`](src/source/ofxAEVideoSource.cpp:25)
   - [`SequenceSource::setFrame(int)`](src/source/ofxAESequenceSource.cpp:27)
   - [`ShapeSource::setFrame(int)`](src/source/ofxAEShapeSource.cpp:40)

5. **[`ofxAEPlayer`](src/ofxAEPlayer.h:39)** - 再生管理
   - [`target_frame_`](src/ofxAEPlayer.h:83) (int型)
   - フレーム更新ロジック

### 重要な発見

**現在の補間システム:**
- [`Property<T>::setFrame(int)`](src/prop/ofxAEProperty.h:159-173) は既に**ratio補間**を実装している
- [`findKeyframePair()`](src/prop/ofxAEKeyframe.h:176-231) がキーフレーム間の比率を計算
- [`interpolation::calculate()`](src/prop/ofxAEKeyframe.h:115-135) が LINEAR/BEZIER/HOLD 補間をサポート

**現在のボトルネック:**
- 整数フレーム番号のため、ratio は常に整数間でしか計算されない
- 例: frame=10の時、frame_a=10, frame_b=11, ratio=0.0
- 例: frame=11の時、frame_a=10, frame_b=11, ratio=1.0
- **0.5のような中間値が計算できない**

---

## アプローチ 1: Float型 setFrame の導入

### 概要
現在の整数ベースのフレーム番号システムを維持しつつ、`setFrame(float)` メソッドを追加し、小数点以下のフレーム値で補間を行う。

### 実装範囲

**変更が必要なコンポーネント (約10-15ファイル):**

1. **コアクラス:**
   - [`ofxAEComposition`](src/core/ofxAEComposition.h)
     - `bool setFrame(float frame)` 追加
     - `current_frame_` を `float` に変更
   - [`ofxAELayer`](src/core/ofxAELayer.h)
     - `bool setFrame(float frame)` 追加
     - `current_frame_` を `float` に変更

2. **プロパティシステム:**
   - [`PropertyBase`](src/prop/ofxAEProperty.h:22)
     - `virtual bool setFrame(float frame)` 追加
   - [`Property<T>`](src/prop/ofxAEProperty.h:159)
     - `bool setFrame(float frame)` 実装
   - [`PropertyGroup`](src/prop/ofxAEProperty.h:217), [`PropertyArray`](src/prop/ofxAEProperty.h:267)
     - 同様に `setFrame(float)` 追加

3. **キーフレームユーティリティ:**
   - [`findKeyframePair()`](src/prop/ofxAEKeyframe.h:176)
     - `float frame` パラメータに変更
     - ratio 計算を float に対応

4. **ソース系:**
   - [`LayerSource::setFrame(float)`](src/source/ofxAELayerSource.h:26)
   - [`CompositionSource::setFrame(float)`](src/source/ofxAECompositionSource.h:18)
   - [`VideoSource::setFrame(float)`](src/source/ofxAEVideoSource.h:16)
   - [`SequenceSource::setFrame(float)`](src/source/ofxAESequenceSource.h:14)
   - [`ShapeSource::setFrame(float)`](src/source/ofxAEShapeSource.h:19)

5. **プレイヤー:**
   - [`ofxAEPlayer::setFrame(float)`](src/ofxAEPlayer.h:39)
   - `target_frame_` を `float` に変更

### メリット

1. **最小限の変更範囲**
   - 既存の整数ベース API を維持可能
   - 後方互換性の完全な保持
   - 段階的な移行が可能

2. **直感的な API**
   ```cpp
   composition.setFrame(15.5f);  // 15フレームと16フレームの中間
   ```

3. **実装の局所性**
   - 補間ロジックは既に [`interpolation::calculate()`](src/prop/ofxAEKeyframe.h:115) に集中
   - 変更は主にパラメータ型の変更

4. **デバッグの容易性**
   - フレーム番号が直接可視化可能
   - 既存のデバッグツールがそのまま使用可能

5. **低リスク**
   - 既存コードとの共存が可能
   - テストを段階的に追加できる

6. **既存の補間システムを活用**
   - [`interpolation::bezier()`](src/prop/ofxAEKeyframe.h:58) などは既に実装済み
   - キーフレーム検索ロジックも再利用可能

### デメリット

1. **API の二重化**
   ```cpp
   void setFrame(int frame);   // 既存
   void setFrame(float frame); // 新規
   ```
   - C++ のオーバーロード解決の問題
   - `setFrame(0)` が int版 を呼ぶ可能性
   - ドキュメントの複雑化

2. **内部状態の複雑化**
   - `current_frame_` の型選択 (int vs float)
   - 浮動小数点の比較問題:
     ```cpp
     if(current_frame_ == frame) // 危険！
     ```
   - キャッシュキーの管理

3. **フレームレート依存性の残存**
   - フレームレートが異なるコンポジションのネスト時の計算が複雑
   - `frame / fps` の変換が随所に必要

4. **精度の問題**
   ```cpp
   setFrame(15.5f);
   setFrame(15.500001f); // ほぼ同じだが異なる値
   ```

5. **概念的な不整合**
   - AE は時間ベースのシステム
   - フレーム番号はフレームレートに依存
   - 15.5 フレームという概念の曖昧性

### 実装の詳細

```cpp
// ofxAEComposition.h
class Composition {
public:
    bool setFrame(int frame) { return setFrame(static_cast<float>(frame)); }
    bool setFrame(float frame);
    
    float getCurrentFrame() const { return current_frame_; }
    
private:
    float current_frame_ = 0.0f;   // int から float へ変更
};

// ofxAEComposition.cpp
bool Composition::setFrame(float frame) {
    constexpr float EPSILON = 0.0001f;
    if(std::abs(current_frame_ - frame) < EPSILON) {
        return false;
    }
    
    bool ret = false;
    auto offset = [this](std::shared_ptr<Layer> layer) {
        auto found = layer_offsets_.find(layer);
        if(found == end(layer_offsets_)) return 0;
        return found->second;
    };
    
    for(auto& layer : layers_) {
        ret |= layer->setFrame(frame - offset(layer));
    }
    current_frame_ = frame;
    return ret;
}

// ofxAEKeyframe.h - findKeyframePair の更新
template<typename T>
KeyframePair<T> findKeyframePair(const std::map<int, Keyframe::Data<T>>& keyframes, 
                                  float frame) {  // float に変更
    KeyframePair<T> result;
    
    if (keyframes.empty()) {
        return result;
    }
    
    // frame を整数部分と小数部分に分離
    int frame_floor = static_cast<int>(std::floor(frame));
    int frame_ceil = static_cast<int>(std::ceil(frame));
    
    // 同じ整数フレーム内の場合
    if(frame_floor == frame_ceil) {
        auto upper = keyframes.upper_bound(frame_floor);
        // ... 既存のロジック
    }
    
    // キーフレーム間の補間
    auto lower = keyframes.upper_bound(frame_floor);
    if(lower == keyframes.begin()) {
        // frame より前に keyframe がない
        result.keyframe_a = &keyframes.begin()->second;
        result.keyframe_b = &keyframes.begin()->second;
        result.frame_a = keyframes.begin()->first;
        result.frame_b = keyframes.begin()->first;
        result.ratio = 0.0f;
        return result;
    }
    
    auto keyframe_a_it = std::prev(lower);
    auto keyframe_b_it = lower;
    
    if(keyframe_b_it == keyframes.end()) {
        // frame より後に keyframe がない
        auto last = std::prev(keyframes.end());
        result.keyframe_a = &last->second;
        result.keyframe_b = &last->second;
        result.frame_a = last->first;
        result.frame_b = last->first;
        result.ratio = 0.0f;
        return result;
    }
    
    result.keyframe_a = &keyframe_a_it->second;
    result.keyframe_b = &keyframe_b_it->second;
    result.frame_a = keyframe_a_it->first;
    result.frame_b = keyframe_b_it->first;
    
    if (result.frame_b != result.frame_a) {
        result.ratio = (frame - result.frame_a) / 
                      static_cast<float>(result.frame_b - result.frame_a);
    } else {
        result.ratio = 0.0f;
    }
    
    return result;
}

// ofxAEProperty.h
template<typename T>
class Property : public PropertyBase {
public:
    bool setFrame(int frame) override { 
        return setFrame(static_cast<float>(frame)); 
    }
    
    bool setFrame(float frame) override {
        bool is_first = !cache_.has_value();
        if (keyframes_.empty()) {
            cache_ = base_;
            return is_first;
        }
        
        auto pair = ofx::ae::util::findKeyframePair(keyframes_, frame);
        if (pair.keyframe_a == nullptr || pair.keyframe_b == nullptr) {
            cache_ = base_;
            return is_first;
        }
        
        float fps = 30.f; // TODO: use correct fps
        float dt = (pair.frame_b - pair.frame_a) / fps;
        cache_ = ofx::ae::util::interpolateKeyframe(
            *pair.keyframe_a, *pair.keyframe_b, dt, pair.ratio);
        return true;
    }
    
private:
    T base_;
    std::optional<T> cache_;
    std::map<int, ofxAEKeyframe<T>> keyframes_;  // キーは int のまま
};
```

### 推定工数

- **設計・仕様策定:** 2-3日
- **コア実装 (Property/Keyframe):** 3-5日
- **Composition/Layer 対応:** 2-3日
- **Source 系対応:** 2-3日
- **テスト作成:** 3-5日
- **ドキュメント更新:** 1-2日

**合計:** 13-21日 (約 3-4週間)

---

## アプローチ 2: フレームから時間への移行

### 概要
ライブラリ全体の基本時間単位をフレーム番号から秒単位の時間に変更する。

### 実装範囲

**全面的な変更が必要 (推定 30+ ファイル):**

1. **すべてのコアクラス:**
   - Composition, Layer, Property 系
   - `setFrame(int)` → `setTime(double)`
   - `current_frame_` → `current_time_`

2. **キーフレームデータ構造:**
   - `std::map<int, Keyframe>` → `std::map<double, Keyframe>`
   - JSON エクスポート/インポートの変更

3. **すべての Source:**
   - LayerSource, CompositionSource, VideoSource, etc.

4. **Player:**
   - [`ofxAEPlayer`](src/ofxAEPlayer.h) 全面書き換え
   - `target_frame_` → `target_time_`

5. **AE エクスポーター:**
   - [`ExportComposition.jsx`](tools/ExportComposition.jsx) の変更
   - フレーム番号 → 時間への変換

### メリット

1. **概念的な正確性**
   - AE の内部表現に一致
   - フレームレート非依存
   - 時間は物理的に明確な単位

2. **ネストされたコンポジションの扱いが容易**
   ```cpp
   // 異なるフレームレートのコンポジションのネスト
   parent_comp.setTime(1.5);  // 1.5秒
   // 子コンポの fps に関わらず同じ時間を参照
   ```

3. **フレームレート変換の柔軟性**
   ```cpp
   comp.setFrameRate(24);
   comp.setTime(1.0);  // 24フレーム目
   
   comp.setFrameRate(30);
   comp.setTime(1.0);  // 30フレーム目（同じ時刻）
   ```

4. **将来的な拡張性**
   - 可変フレームレート対応が容易
   - タイムリマップのサポートが自然
   - オーディオ同期が直感的

5. **精度の向上**
   - double 精度で時間を扱える
   - フレーム変換の累積誤差を削減

### デメリット

1. **膨大な変更範囲**
   - 全ファイルの大幅な書き換え
   - リグレッションのリスク大
   - テストケースの全面的な更新

2. **後方互換性の完全な喪失**
   ```cpp
   // 旧 API
   comp.setFrame(30);
   int frame = comp.getFrame();
   
   // 新 API
   comp.setTime(1.0);
   double time = comp.getTime();
   ```

3. **既存プロジェクトへの影響**
   - すべての既存コードが動作不可
   - 移行ガイドの作成が必須
   - ユーザーの学習コスト大

4. **デバッグの複雑化**
   ```cpp
   comp.setTime(0.6333333);  // これは何フレーム目？
   // → 19フレーム目 (30fps の場合)
   ```

5. **AE エクスポーターの変更**
   - [`ExportComposition.jsx`](tools/ExportComposition.jsx) の全面書き換え
   - フレーム番号→時間への変換ロジック追加
   - 既存の JSON データとの非互換性

6. **パフォーマンスオーバーヘッド**
   ```cpp
   // フレーム番号が必要な場合は常に変換が発生
   int frame = static_cast<int>(time * fps);
   ```

### 実装の詳細

```cpp
// ofxAEComposition.h
class Composition {
public:
    void setTime(double time_sec);
    double getTime() const { return current_time_; }
    
    // ユーティリティ（オプション）
    void setFrame(int frame) { setTime(frame / info_.fps); }
    int getFrame() const { return static_cast<int>(current_time_ * info_.fps); }
    
private:
    double current_time_ = 0.0;  // 秒単位
    Info info_;
};

// ofxAEProperty.h
template<typename T>
class Property {
public:
    T getValue(double time) const;  // 秒単位
    bool setTime(double time);
    
private:
    std::map<double, Keyframe<T>> keyframes_;  // 時間ベース
};

// JSON エクスポート変更例
// Before: { "frame": 30, "value": 100 }
// After:  { "time": 1.0, "value": 100 }

// tools/ExportComposition.jsx の変更
// Before:
// keyframe.frame = kf.time * comp.frameRate;
// After:
// keyframe.time = kf.time;  // 直接時間を使用
```

### 推定工数

- **設計・仕様策定:** 5-7日
- **データ構造変更:** 5-7日
- **Property/Keyframe 書き換え:** 7-10日
- **Composition/Layer 書き換え:** 7-10日
- **Source 系書き換え:** 5-7日
- **AE エクスポーター変更:** 3-5日
- **全テスト更新:** 10-15日
- **Example 更新:** 3-5日
- **移行ガイド作成:** 3-5日
- **ドキュメント全面更新:** 5-7日

**合計:** 53-78日 (約 11-16週間、3-4ヶ月)

---

## 比較表

| 項目 | Float setFrame | Frame→Time 移行 |
|------|----------------|------------------|
| **変更範囲** | 限定的（10-15ファイル） | 全面的（30+ファイル） |
| **後方互換性** | 完全に維持 | 完全に喪失 |
| **実装工数** | 3-4週間 | 3-4ヶ月 |
| **リスク** | 低 | 高 |
| **概念的正確性** | 中（フレームは AE 本来の単位でない） | 高（時間は物理的に正確） |
| **デバッグ容易性** | 高（フレーム番号が直感的） | 中（時間↔フレーム変換が必要） |
| **拡張性** | 中（フレームレート依存性が残る） | 高（時間ベースで柔軟） |
| **学習コスト** | 低（既存ユーザーへの影響小） | 高（全面的な API 変更） |
| **パフォーマンス** | ほぼ同等 | わずかなオーバーヘッド |
| **既存コード活用** | 高（補間システムは再利用） | 低（ほぼ全面書き換え） |

---

## 推奨事項

### 短期的な解決策（強く推奨）

**アプローチ 1: Float setFrame の導入**

**理由:**
1. **実用性:** 3-4週間で実装可能、すぐに効果が得られる
2. **安全性:** 既存コードへの影響が最小限
3. **段階的移行:** 必要に応じて将来的にアプローチ 2 へ移行可能
4. **既存の補間システムを活用:** [`interpolation::calculate()`](src/prop/ofxAEKeyframe.h:115) は既に実装済み

**実装戦略:**
```cpp
// Phase 1: 内部を float 化（後方互換性維持）
class Composition {
    float current_frame_ = 0.0f;
public:
    bool setFrame(int frame) { return setFrame(static_cast<float>(frame)); }
    bool setFrame(float frame);  // 新規実装
};

// Phase 2: Player 対応
class Player {
    float target_frame_ = 0.0f;
public:
    void setFrame(int frame) { setFrame(static_cast<float>(frame)); }
    void setFrame(float frame);
};

// Phase 3: ドキュメント・サンプル更新
```

### 長期的なビジョン（将来的な検討）

**アプローチ 2: Frame→Time 移行**

**実施条件:**
- メジャーバージョンアップ（v2.0 など）のタイミング
- 十分な移行期間（3-6ヶ月）の確保
- 両バージョンの並行サポート体制

**移行パス:**
1. **v1.x:** Float setFrame 実装（後方互換性維持）
2. **v2.0-alpha:** Time ベース実装（deprecation 警告付き）
3. **v2.0-beta:** Time ベース（Frame API は互換レイヤーとして提供）
4. **v2.0:** Time ベース正式版

---

## 技術的考察

### 浮動小数点精度の扱い

```cpp
// Float setFrame アプローチでの注意点
namespace {
    constexpr float FRAME_EPSILON = 0.0001f;
}

bool isNearFrame(float a, float b) {
    return std::abs(a - b) < FRAME_EPSILON;
}

// Composition::setFrame での使用例
bool Composition::setFrame(float frame) {
    if(isNearFrame(current_frame_, frame)) {
        return false;  // 変更なし
    }
    // ... 処理
}
```

### フレームレート変換の正確性

```cpp
// 15fps → 30fps の例
// AE側: 15fps, 0,1,2,3,...
// oF側: 30fps, 0,1,2,3,...

void ofApp::update() {
    int of_frame = ofGetFrameNum();
    float ae_time = of_frame / 30.0f;           // oFの時間
    float ae_frame = ae_time * 15.0f;           // AEのフレーム番号
    composition.setFrame(ae_frame);              // 0, 0.5, 1, 1.5, ...
    
    // または直接
    composition.setFrame(of_frame * 0.5f);       // 15fps/30fps = 0.5
}
```

### キーフレーム補間の詳細

現在の実装では [`interpolation::calculate()`](src/prop/ofxAEKeyframe.h:115-135) が以下をサポート:

- **LINEAR:** 線形補間
- **BEZIER:** ベジェ曲線補間（temporal ease 対応）
- **HOLD:** ステップ補間

float frame 対応により、これらの補間が**サブフレーム精度**で機能するようになります。

---

## 結論

**即座に実装すべき:** アプローチ 1 (Float setFrame)
- ✅ 実用的な補間が 3-4週間で実現
- ✅ リスクとコストが最小
- ✅ 既存ユーザーへの影響なし
- ✅ 既存の補間システムを最大限活用

**将来的な検討課題:** アプローチ 2 (Frame→Time 移行)
- ⏰ メジャーバージョンアップ時に再評価
- 📊 より根本的な解決だが、現時点では投資対効果が低い

**ハイブリッドアプローチ:**
```cpp
// 内部は float で処理、将来の time 移行も考慮
class Composition {
private:
    float current_frame_ = 0.0f;
    float fps_ = 30.0f;
    
    // 内部ヘルパー（将来的に公開 API へ）
    double getTimeInternal() const { return current_frame_ / fps_; }
    
public:
    bool setFrame(int frame);         // 既存API
    bool setFrame(float frame);       // 今回追加
    // void setTime(double time);     // v2.0 で追加予定
};
```

この段階的アプローチにより、**短期的な問題解決と長期的なアーキテクチャ改善の両立**が可能になります。