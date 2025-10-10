#include "ofxAELayerFactory.h"
#include "ofxAECompositionLayer.h"
#include "ofLog.h"
#include <fstream>

namespace ofx { namespace ae {

std::shared_ptr<Layer> LayerFactory::createFromJson(const std::string& layerFilePath) {
    // JSONファイルの読み込み
    std::ifstream file(layerFilePath);
    if (!file.is_open()) {
        ofLogError("ofxAELayerFactory") << "Cannot open file: " << layerFilePath;
        return nullptr;
    }
    
    ofJson json;
    try {
        file >> json;
    } catch (const std::exception &e) {
        ofLogError("ofxAELayerFactory") << "JSON parse error in " << layerFilePath << ": " << e.what();
        return nullptr;
    }
    
    return createFromJson(json);
}

std::shared_ptr<Layer> LayerFactory::createFromJson(const ofJson& json) {
    // LayerTypeの解析
    Layer::LayerType layerType = parseLayerType(json);
    
    // 適切なLayerインスタンスを生成
    auto layer = createLayerInstance(layerType);
    if (!layer) {
        ofLogError("ofxAELayerFactory") << "Failed to create layer instance for type: " << static_cast<int>(layerType);
        return nullptr;
    }
    
    // JSONデータでLayerを初期化
    if (!layer->setup(json)) {
        ofLogError("ofxAELayerFactory") << "Failed to setup layer from JSON";
        return nullptr;
    }
    
    return layer;
}

Layer::LayerType LayerFactory::parseLayerType(const ofJson& json) {
    if (json.contains("layerType") && json["layerType"].is_string()) {
        std::string layerTypeStr = json["layerType"].get<std::string>();
        return Layer::stringToLayerType(layerTypeStr);
    }
    
    // デフォルトはAV_LAYER
    ofLogWarning("ofxAELayerFactory") << "No layerType found in JSON, defaulting to AV_LAYER";
    return Layer::AV_LAYER;
}

std::shared_ptr<Layer> LayerFactory::createLayerInstance(Layer::LayerType layerType) {
    switch (layerType) {
        case Layer::SHAPE_LAYER:
            ofLogVerbose("ofxAELayerFactory") << "Creating ShapeLayer instance";
            return std::make_shared<ShapeLayer>();
            
        case Layer::COMPOSITION_LAYER:
            ofLogVerbose("ofxAELayerFactory") << "Creating CompositionLayer instance";
            return std::make_shared<CompositionLayer>();
            
        case Layer::VECTOR_LAYER:
            ofLogVerbose("ofxAELayerFactory") << "Creating VectorLayer instance (using base Layer for now)";
            return std::make_shared<Layer>();
            
        case Layer::AV_LAYER:
        default:
            ofLogVerbose("ofxAELayerFactory") << "Creating AV Layer instance";
            return std::make_shared<Layer>();
    }
}

}} // namespace ofx::ae