#pragma once

#include "ofMain.h"
#include "prop/ofxAEShapeProp.h"
#include <memory>
#include <vector>

namespace ofx { namespace ae {

/**
 * PathBuilderクラス
 * ShapeDataから合成されたofPathを生成する責務を持つ
 */
class PathBuilder {
public:
    /**
     * コンストラクタ
     */
    PathBuilder();
    
    /**
     * デストラクタ
     */
    ~PathBuilder();
    
    /**
     * メインの処理メソッド - レンダリング可能な形状を構築
     * @param shapeData 入力となるShapeDataコレクション
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 合成されたofPath
     */
    ofPath buildRenderableShapes(const ShapeData& shapeData, float x = 0, float y = 0, float w = 0, float h = 0);
    
private:
    /**
     * 楕円パス作成
     * @param ellipse 楕円データ
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 楕円のofPath
     */
    ofPath createEllipsePath(const EllipseData& ellipse, float x, float y, float w, float h);
    
    /**
     * 矩形パス作成
     * @param rectangle 矩形データ
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return 矩形のofPath
     */
    ofPath createRectanglePath(const RectangleData& rectangle, float x, float y, float w, float h);
    
    /**
     * ポリゴンパス作成
     * @param polygon ポリゴンデータ
     * @param x レンダリング位置X
     * @param y レンダリング位置Y
     * @param w レンダリング幅
     * @param h レンダリング高さ
     * @return ポリゴンのofPath
     */
    ofPath createPolygonPath(const PolygonData& polygon, float x, float y, float w, float h);
    
    /**
     * パス合成処理
     * @param paths 合成するパスのコレクション
     * @param directions 各パスのdirection値
     * @return 合成されたofPath
     */
    ofPath combinePaths(const std::vector<ofPath>& paths, const std::vector<int>& directions);
    
    /**
     * direction基準でパスを統合する
     * @param basePath ベースとなるパス
     * @param addPath 追加するパス
     * @param direction パスの方向（1=時計回り、-1=反時計回り）
     * @return 統合されたパス
     */
    ofPath integratePath(const ofPath& basePath, const ofPath& addPath, int direction);
    
    /**
     * パスの方向を反転させる
     * @param path 対象パス
     * @return 方向反転されたパス
     */
    ofPath reversePath(const ofPath& path);
    
    /**
     * スケーリング値を計算
     * @param shapeSize 形状のサイズ
     * @param renderW レンダリング幅
     * @param renderH レンダリング高さ
     * @return スケール値（X, Y）
     */
    glm::vec2 calculateScale(const glm::vec2& shapeSize, float renderW, float renderH);
};

}} // namespace ofx::ae