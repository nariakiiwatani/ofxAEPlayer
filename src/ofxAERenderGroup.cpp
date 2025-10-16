#include "ofxAERenderGroup.h"
#include "ofxAEPathBuilder.h"
#include "ofLog.h"
#include <typeinfo>

namespace ofx { namespace ae {

//==============================================================================
// RenderGroupProcessor Implementation
//==============================================================================

RenderGroupProcessor::RenderGroupProcessor() {
    pathBuilder_ = std::make_unique<PathBuilder>();
    // No more circular dependency - removed groupProcessor_
}

RenderGroupProcessor::~RenderGroupProcessor() {
}

std::vector<RenderItem> RenderGroupProcessor::processRenderOrder(const ShapeData& shapeData, 
                                                                  float x, float y, float w, float h) {
    std::vector<RenderItem> allItems;
    
    // 現在の形状パスを構築（形状要素のみ）
    ofPath currentPath = pathBuilder_->buildRenderableShapes(shapeData, x, y, w, h);
    
    // 各ShapeDataBaseからRenderItemを作成
    for (const auto& shapePtr : shapeData) {
        if (!shapePtr) continue;
        
        auto items = createRenderItemFromShape(shapePtr, currentPath, x, y, w, h);
        allItems.insert(allItems.end(), items.begin(), items.end());
    }
    
    // compositeOrder順でソート
    sortRenderItems(allItems);
    
    // 最適化を適用
    return optimizeRenderItems(allItems);
}

void RenderGroupProcessor::sortRenderItems(std::vector<RenderItem>& items) {
    // 安定ソートを使用して元の順序を保持
    std::stable_sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
        // まずcompositeOrderで比較
        if (a.compositeOrder != b.compositeOrder) {
            return a.compositeOrder < b.compositeOrder;
        }
        
        // 同じcompositeOrderの場合、Fill → Stroke順序
        if (a.isFill != b.isFill) {
            return a.isFill; // Fill(true)が先、Stroke(false)が後
        }
        
        // 同じタイプの場合は元の順序を維持（stable_sortにより保証）
        return false;
    });
}

std::vector<RenderItem> RenderGroupProcessor::optimizeRenderItems(const std::vector<RenderItem>& items) {
    if (items.empty()) {
        return items;
    }
    
    // 基本的な最適化として同一設定のマージを実行
    return mergeSimilarItems(items);
}

std::map<int, RenderGroup> RenderGroupProcessor::groupByCompositeOrder(const std::vector<RenderItem>& items) {
    std::map<int, RenderGroup> groups;
    
    for (const auto& item : items) {
        int order = item.compositeOrder;
        
        if (groups.find(order) == groups.end()) {
            groups[order] = RenderGroup(order);
        }
        
        groups[order].items.push_back(item);
    }
    
    return groups;
}

std::vector<RenderItem> RenderGroupProcessor::createRenderItemFromShape(const std::unique_ptr<ShapeDataBase>& shapePtr,
                                                                         const ofPath& currentPath,
                                                                         float x, float y, float w, float h) {
    std::vector<RenderItem> items;
    
    if (!shapePtr) return items;
    
    // 型判定とRenderItem作成
    if (auto fillData = dynamic_cast<const FillData*>(shapePtr.get())) {
        RenderItem item;
        item.path = currentPath;
        item.compositeOrder = fillData->compositeOrder;
        item.blendMode = static_cast<BlendMode>(fillData->blendMode);
        item.isFill = true;
        
        // Fill固有プロパティの設定
        item.fillColor = fillData->color;
        item.fillOpacity = fillData->opacity;
        item.fillRule = fillData->rule;
        
        items.push_back(item);
    }
    else if (auto strokeData = dynamic_cast<const StrokeData*>(shapePtr.get())) {
        RenderItem item;
        item.path = currentPath;
        item.compositeOrder = strokeData->compositeOrder;
        item.blendMode = static_cast<BlendMode>(strokeData->blendMode);
        item.isFill = false;
        
        // Stroke固有プロパティの設定
        item.strokeColor = strokeData->color;
        item.strokeOpacity = strokeData->opacity;
        item.strokeWidth = strokeData->width;
        item.lineCap = strokeData->lineCap;
        item.lineJoin = strokeData->lineJoin;
        item.miterLimit = strokeData->miterLimit;
        
        items.push_back(item);
    }
    else if (auto groupData = dynamic_cast<const GroupData*>(shapePtr.get())) {
        // GroupDataを内部メソッドで処理（循環依存を解決）
        auto groupItems = processGroup(*groupData, x, y, w, h);
        items.insert(items.end(), groupItems.begin(), groupItems.end());
    }
    // EllipseData、RectangleData、PolygonDataは形状パス生成に使用されるため
    // ここでは直接RenderItemを作成しない
    
    return items;
}

std::vector<RenderItem> RenderGroupProcessor::mergeSimilarItems(const std::vector<RenderItem>& items) {
    // 基本実装：現在は最適化なしで全てのアイテムを返す
    // 将来的には同一パス・同一設定のアイテムをマージする処理を追加
    return items;
}

//==============================================================================
// Integrated GroupData Processing (previously in GroupProcessor)
//==============================================================================

std::vector<RenderItem> RenderGroupProcessor::processGroup(const GroupData& groupData,
                                                            float x, float y, float w, float h) {
    std::vector<RenderItem> items;
    
    // ネストしたShapeDataを再帰的に処理
    auto nestedItems = processNestedShapes(groupData.data, x, y, w, h);
    
    // GroupのblendModeを各アイテムに適用
    BlendMode groupBlendMode = static_cast<BlendMode>(groupData.blendMode);
    for (auto& item : nestedItems) {
        // グループのブレンドモードで上書き（より詳細な処理が必要な場合がある）
        if (groupBlendMode != BlendMode::NORMAL) {
            item.blendMode = groupBlendMode;
        }
    }
    
    items.insert(items.end(), nestedItems.begin(), nestedItems.end());
    return items;
}

std::vector<RenderItem> RenderGroupProcessor::processNestedShapes(const ShapeData& shapeData,
                                                                   float x, float y, float w, float h) {
    // 内部的に再帰処理（循環依存なし）
    return processRenderOrder(shapeData, x, y, w, h);
}

//==============================================================================
// RenderItemRenderer Implementation
//==============================================================================

RenderItemRenderer::RenderItemRenderer() {
}

RenderItemRenderer::~RenderItemRenderer() {
}

void RenderItemRenderer::render(const std::vector<RenderItem>& items) {
    for (const auto& item : items) {
        renderItem(item);
    }
}

void RenderItemRenderer::renderItem(const RenderItem& item) {
    // レンダリングコンテキストを保存
    RenderContext::push();
    
    try {
        // ブレンドモードを適用
        applyBlendMode(item.blendMode);
        
        if (item.isFill) {
            // Fill描画
            applyFillSettings(item);
            ofFill();
            item.path.draw();
        } else {
            // Stroke描画
            applyStrokeSettings(item);
            ofNoFill();
            item.path.draw();
        }
    }
    catch (const std::exception& e) {
        ofLogError("RenderItemRenderer") << "Error rendering item: " << e.what();
    }
    
    // レンダリングコンテキストを復元
    RenderContext::pop();
}

void RenderItemRenderer::applyFillSettings(const RenderItem& item) {
    // Fill色と透明度を設定
    ofFloatColor fillColor(item.fillColor.r, item.fillColor.g, item.fillColor.b, item.fillOpacity);
    RenderContext::setColorRGB(fillColor);
    RenderContext::setOpacity(item.fillOpacity);
    
    // Fill ruleの適用（openFrameworksでの実装は限定的）
    // fillRuleはパスの塗りつぶしルール（偶奇則、非ゼロ巻数則）に関連
    // ofPathのFillRuleで設定可能
    switch (item.fillRule) {
        case 1: // Even-odd (奇偶則)
            // ofPath::setPolyWindingMode(OF_POLY_WINDING_ODD);
            break;
        case 2: // Non-zero (非ゼロ巻数則)
            // ofPath::setPolyWindingMode(OF_POLY_WINDING_NONZERO);
            break;
        default:
            break;
    }
}

void RenderItemRenderer::applyStrokeSettings(const RenderItem& item) {
    // Stroke色と透明度を設定
    ofFloatColor strokeColor(item.strokeColor.r, item.strokeColor.g, item.strokeColor.b, item.strokeOpacity);
    RenderContext::setColorRGB(strokeColor);
    RenderContext::setOpacity(item.strokeOpacity);
    
    // ストローク幅を設定
    ofSetLineWidth(item.strokeWidth);
    
    // Line capの設定（openFrameworksでの制限あり）
    switch (item.lineCap) {
        case 1: // Round cap
            // 実装は限定的、パスレベルでの制御が必要
            break;
        case 2: // Square cap
            // 実装は限定的、パスレベルでの制御が必要
            break;
        default: // Butt cap
            break;
    }
    
    // Line joinの設定（openFrameworksでの制限あり）
    switch (item.lineJoin) {
        case 1: // Round join
            // 実装は限定的、パスレベルでの制御が必要
            break;
        case 2: // Bevel join
            // 実装は限定的、パスレベルでの制御が必要
            break;
        default: // Miter join
            // miterLimitも考慮する必要がある
            break;
    }
}

void RenderItemRenderer::applyBlendMode(BlendMode blendMode) {
    RenderContext::setBlendMode(blendMode);
}

}} // namespace ofx::ae
