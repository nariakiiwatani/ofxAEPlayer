#pragma once

#include "ofMain.h"
#include "prop/ofxAEShapeProp.h"
#include "ofxAERenderContext.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <map>

namespace ofx { namespace ae {

/**
 * RenderItem - 描画要素を表現するデータ構造
 * compositeOrderに基づく正しい描画順序を管理する
 */
struct RenderItem {
    ofPath path;                // 描画用パス
    int compositeOrder;         // 描画順序（After Effects互換）
    BlendMode blendMode;        // ブレンドモード
    bool isFill;               // true=Fill, false=Stroke
    
    // Fill固有プロパティ
    ofFloatColor fillColor;
    float fillOpacity;
    int fillRule;
    
    // Stroke固有プロパティ
	ofFloatColor strokeColor;
    float strokeOpacity;
    float strokeWidth;
    int lineCap;
    int lineJoin;
    float miterLimit;
    
    RenderItem() : compositeOrder(1), blendMode(BlendMode::NORMAL), isFill(true),
                   fillColor(1.0f), fillOpacity(1.0f), fillRule(1),
                   strokeColor(1.0f), strokeOpacity(1.0f), strokeWidth(1.0f),
                   lineCap(1), lineJoin(1), miterLimit(4.0f) {}
};

/**
 * RenderGroup - compositeOrder別にグループ化されたRenderItemsのコレクション
 */
struct RenderGroup {
    int compositeOrder;
    std::vector<RenderItem> items;
    
    // Default constructor
    RenderGroup() : compositeOrder(1) {}
    
    // Parameterized constructor
    RenderGroup(int order) : compositeOrder(order) {}
};

/**
 * RenderGroupProcessor - compositeOrder処理の責務を持つクラス
 * 正確なcompositeOrder順序とFill→Stroke順序を保証する
 */
class RenderGroupProcessor {
public:
    RenderGroupProcessor();
    ~RenderGroupProcessor();
    
    /**
     * メイン処理 - ShapeDataから最適化されたRenderItems配列を生成
     * @param shapeData 入力となるShapeDataコレクション
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 描画順序でソートされたRenderItemsの配列
     */
    std::vector<RenderItem> processRenderOrder(const ShapeData& shapeData, 
                                               float x = 0, float y = 0, 
                                               float w = 0, float h = 0);
    
    /**
     * RenderItemsを描画順序でソート
     * @param items ソート対象のRenderItems
     */
    void sortRenderItems(std::vector<RenderItem>& items);
    
    /**
     * 冗長描画の最適化
     * @param items 最適化対象のRenderItems
     * @return 最適化されたRenderItems
     */
    std::vector<RenderItem> optimizeRenderItems(const std::vector<RenderItem>& items);

private:
    /**
     * compositeOrder別にRenderItemsをグルーピング
     * @param items グルーピング対象のRenderItems
     * @return compositeOrder別のRenderGroupsのマップ
     */
    std::map<int, RenderGroup> groupByCompositeOrder(const std::vector<RenderItem>& items);
    
    /**
     * ShapeDataBaseからRenderItemを作成
     * @param shapePtr 入力となるShapeDataBase
     * @param currentPath 現在のパス（形状パス）
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 作成されたRenderItems
     */
    std::vector<RenderItem> createRenderItemFromShape(const std::unique_ptr<ShapeDataBase>& shapePtr,
                                                      const ofPath& currentPath,
                                                      float x, float y, float w, float h);
    
    /**
     * 同一パス・設定のRenderItemをマージして最適化
     * @param items マージ対象のRenderItems
     * @return マージされたRenderItems
     */
    std::vector<RenderItem> mergeSimilarItems(const std::vector<RenderItem>& items);
    
    /**
     * PathBuilderインスタンス
     */
    std::unique_ptr<class PathBuilder> pathBuilder_;
    
    /**
     * GroupData処理のメインメソッド（旧GroupProcessorの機能を統合）
     * @param groupData 処理対象のGroupData
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 処理されたRenderItems
     */
    std::vector<RenderItem> processGroup(const GroupData& groupData,
                                         float x, float y, float w, float h);
    
    /**
     * ネストしたShapeDataを再帰的に処理
     * @param shapeData 処理対象のShapeData
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 処理されたRenderItems
     */
    std::vector<RenderItem> processNestedShapes(const ShapeData& shapeData,
                                                 float x, float y, float w, float h);
};

/**
 * RenderItemRenderer - RenderItemsを実際に描画するクラス
 * ブレンドモードやレンダリングコンテキストを考慮した描画を実行
 */
class RenderItemRenderer {
public:
    RenderItemRenderer();
    ~RenderItemRenderer();
    
    /**
     * RenderItemsを描画
     * @param items 描画対象のRenderItems
     */
    void render(const std::vector<RenderItem>& items);
    
    /**
     * 単一RenderItemを描画
     * @param item 描画対象のRenderItem
     */
    void renderItem(const RenderItem& item);

private:
    /**
     * Fillの描画設定を適用
     * @param item 対象のRenderItem
     */
    void applyFillSettings(const RenderItem& item);
    
    /**
     * Strokeの描画設定を適用
     * @param item 対象のRenderItem
     */
    void applyStrokeSettings(const RenderItem& item);
    
    /**
     * ブレンドモードを適用
     * @param blendMode 適用するブレンドモード
     */
    void applyBlendMode(BlendMode blendMode);
};

}} // namespace ofx::ae
