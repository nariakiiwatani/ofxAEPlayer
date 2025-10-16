#include "ofxAEPathBuilder.h"
#include "ofLog.h"
#include <typeinfo>

namespace ofx { namespace ae {

PathBuilder::PathBuilder() {
}

PathBuilder::~PathBuilder() {
}

ofPath PathBuilder::buildRenderableShapes(const ShapeData& shapeData, float x, float y, float w, float h) {
    std::vector<ofPath> shapePaths;
    std::vector<int> directions;
    
    // 各形状データからパスを生成
    for (const auto& shapePtr : shapeData) {
        if (!shapePtr) continue;
        
        // 型判定と対応するパス作成メソッドの呼び出し
        if (auto ellipse = dynamic_cast<const EllipseData*>(shapePtr.get())) {
            ofPath path = createEllipsePath(*ellipse, x, y, w, h);
            if (path.getOutline().size() > 0) {
                shapePaths.push_back(path);
                directions.push_back(ellipse->direction);
            }
        }
        else if (auto rectangle = dynamic_cast<const RectangleData*>(shapePtr.get())) {
            ofPath path = createRectanglePath(*rectangle, x, y, w, h);
            if (path.getOutline().size() > 0) {
                shapePaths.push_back(path);
                directions.push_back(rectangle->direction);
            }
        }
        else if (auto polygon = dynamic_cast<const PolygonData*>(shapePtr.get())) {
            ofPath path = createPolygonPath(*polygon, x, y, w, h);
            if (path.getOutline().size() > 0) {
                shapePaths.push_back(path);
                directions.push_back(polygon->direction);
            }
        }
        // FillData、StrokeData、GroupDataは形状パスではないのでスキップ
    }
    
    // パスを合成して返す
    return combinePaths(shapePaths, directions);
}

ofPath PathBuilder::createEllipsePath(const EllipseData& ellipse, float x, float y, float w, float h) {
    ofPath path;
    
    // 位置とサイズを計算（After Effectsの中心基準からopenFrameworksの左上基準に変換）
    float ellipseX = x + ellipse.position.x - ellipse.size.x * 0.5f;
    float ellipseY = y + ellipse.position.y - ellipse.size.y * 0.5f;
    
    // スケーリング計算
    glm::vec2 scale = calculateScale(ellipse.size, w, h);
    float scaledWidth = ellipse.size.x * scale.x;
    float scaledHeight = ellipse.size.y * scale.y;
    
    // 楕円パスを作成
    path.ellipse(ellipseX + scaledWidth * 0.5f, ellipseY + scaledHeight * 0.5f, scaledWidth, scaledHeight);
    
    return path;
}

ofPath PathBuilder::createRectanglePath(const RectangleData& rectangle, float x, float y, float w, float h) {
    ofPath path;
    
    // 位置とサイズを計算（After Effectsの中心基準からopenFrameworksの左上基準に変換）
    float rectX = x + rectangle.position.x - rectangle.size.x * 0.5f;
    float rectY = y + rectangle.position.y - rectangle.size.y * 0.5f;
    
    // スケーリング計算
    glm::vec2 scale = calculateScale(rectangle.size, w, h);
    float scaledWidth = rectangle.size.x * scale.x;
    float scaledHeight = rectangle.size.y * scale.y;
    
    // 矩形パスを作成
    if (rectangle.roundness > 0) {
        // 角丸矩形
        path.rectRounded(rectX, rectY, scaledWidth, scaledHeight, rectangle.roundness);
    } else {
        // 通常の矩形
        path.rectangle(rectX, rectY, scaledWidth, scaledHeight);
    }
    
    return path;
}

ofPath PathBuilder::createPolygonPath(const PolygonData& polygon, float x, float y, float w, float h) {
    ofPath path;
    
    // 点数チェック
    int numPoints = polygon.points;
    if (numPoints < 3) {
        ofLogWarning("PathBuilder") << "Polygon needs at least 3 points, got: " << numPoints;
        return path;
    }
    
    // 基準位置
    float polyX = x + polygon.position.x;
    float polyY = y + polygon.position.y;
    
    // スター形状かどうか
    bool isStar = (polygon.type == 2);
    float outerRadius = polygon.outerRadius;
    float innerRadius = isStar ? polygon.innerRadius : outerRadius;
    
    // スケーリング（半径に適用）
    glm::vec2 scale = calculateScale(glm::vec2(outerRadius * 2, outerRadius * 2), w, h);
    float scaledOuterRadius = outerRadius * std::min(scale.x, scale.y);
    float scaledInnerRadius = innerRadius * std::min(scale.x, scale.y);
    
    // 角度計算
    float angleStep = TWO_PI / numPoints;
    float startAngle = polygon.rotation * DEG_TO_RAD;
    
    // パス構築開始
    bool firstPoint = true;
    
    for (int i = 0; i < numPoints; i++) {
        float angle = startAngle + i * angleStep;
        
        // 外側の点
        float pointX = polyX + cos(angle) * scaledOuterRadius;
        float pointY = polyY + sin(angle) * scaledOuterRadius;
        
        if (firstPoint) {
            path.moveTo(pointX, pointY);
            firstPoint = false;
        } else {
            path.lineTo(pointX, pointY);
        }
        
        // スター形状の場合、内側の点を追加
        if (isStar) {
            float innerAngle = angle + angleStep * 0.5f;
            float innerPointX = polyX + cos(innerAngle) * scaledInnerRadius;
            float innerPointY = polyY + sin(innerAngle) * scaledInnerRadius;
            path.lineTo(innerPointX, innerPointY);
        }
    }
    
    // パスを閉じる
    path.close();
    
    return path;
}

ofPath PathBuilder::combinePaths(const std::vector<ofPath>& paths, const std::vector<int>& directions) {
    if (paths.empty()) {
        return ofPath();
    }
    
    if (paths.size() == 1) {
        // パスが1つの場合、direction基準で方向を調整
        ofPath result = paths[0];
        if (directions.size() > 0 && directions[0] < 0) {
            result = reversePath(result);
        }
        return result;
    }
    
    // 複数パスの場合、最初のパスをベースとして順次統合
    ofPath combinedPath = paths[0];
    if (directions.size() > 0 && directions[0] < 0) {
        combinedPath = reversePath(combinedPath);
    }
    
    for (size_t i = 1; i < paths.size(); i++) {
        int direction = (i < directions.size()) ? directions[i] : 1;
        combinedPath = integratePath(combinedPath, paths[i], direction);
    }
    
    return combinedPath;
}

ofPath PathBuilder::integratePath(const ofPath& basePath, const ofPath& addPath, int direction) {
    ofPath result = basePath;
    ofPath pathToAdd = addPath;
    
    // direction基準でパスの方向を調整
    if (direction < 0) {
        pathToAdd = reversePath(pathToAdd);
    }
    
    // パスを統合 - ofPathのappendメソッドを使用
    try {
        // 空のパスチェック
        if (pathToAdd.getOutline().empty()) {
            return result;
        }
        
        // パスを統合（ofPathのappendメソッドを使用）
        result.append(pathToAdd);
    }
    catch (const std::exception& e) {
        ofLogError("PathBuilder") << "Error integrating paths: " << e.what();
        return basePath; // エラー時は元のパスを返す
    }
    
    return result;
}

ofPath PathBuilder::reversePath(const ofPath& path) {
    ofPath reversedPath;
    
    // パスのアウトラインを取得
    const auto& outlines = path.getOutline();
    
    for (const auto& outline : outlines) {
        if (outline.size() < 2) continue;
        
        // アウトラインの点を逆順で追加
        auto vertices = outline.getVertices();
        if (vertices.empty()) continue;
        
        reversedPath.moveTo(vertices.back());
        for (int i = vertices.size() - 2; i >= 0; i--) {
            reversedPath.lineTo(vertices[i]);
        }
        reversedPath.close();
    }
    
    return reversedPath;
}

glm::vec2 PathBuilder::calculateScale(const glm::vec2& shapeSize, float renderW, float renderH) {
    glm::vec2 scale(1.0f, 1.0f);
    
    // レンダリング範囲が指定されている場合のみスケーリングを適用
    if (renderW > 0 && shapeSize.x > 0) {
        scale.x = renderW / shapeSize.x;
    }
    
    if (renderH > 0 && shapeSize.y > 0) {
        scale.y = renderH / shapeSize.y;
    }
    
    return scale;
}

}} // namespace ofx::ae
