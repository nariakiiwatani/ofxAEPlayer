#pragma once

#include "ofxAELayer.h"
#include "ofxAEShapeLayer.h"
#include <memory>
#include <string>

namespace ofx { namespace ae {

/**
 * @brief LayerFactory - After EffectsのJSONデータからLayerオブジェクトを生成するファクトリークラス
 * 
 * JSONファイルのlayerTypeフィールドを解析し、適切なLayerサブクラスのインスタンスを生成します。
 * 既存のLayer::stringToLayerType()を活用した責任分離設計です。
 */
class LayerFactory {
public:
    /**
     * @brief JSONファイルからLayerオブジェクトを生成
     * @param layerFilePath レイヤーJSONファイルのパス
     * @return 生成されたLayerオブジェクト（失敗時はnullptr）
     */
    static std::shared_ptr<Layer> createFromJson(const std::string& layerFilePath);
    
    /**
     * @brief 既に読み込まれたJSONからLayerオブジェクトを生成
     * @param json 解析済みのJSONオブジェクト
     * @return 生成されたLayerオブジェクト（失敗時はnullptr）
     */
    static std::shared_ptr<Layer> createFromJson(const ofJson& json);

private:
    /**
     * @brief JSONからLayerTypeを解析
     * @param json 解析対象のJSONオブジェクト
     * @return 解析されたLayerType
     */
    static Layer::LayerType parseLayerType(const ofJson& json);
    
    /**
     * @brief 指定されたLayerTypeに対応するLayerインスタンスを生成
     * @param layerType 生成するLayerのタイプ
     * @return 生成されたLayerオブジェクト
     */
    static std::shared_ptr<Layer> createLayerInstance(Layer::LayerType layerType);
};

}} // namespace ofx::ae