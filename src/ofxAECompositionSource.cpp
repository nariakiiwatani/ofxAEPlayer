#include "ofxAECompositionSource.h"
#include <sstream>

namespace ofx { namespace ae {

CompositionSource::CompositionSource()
    : LayerSource()
    , composition_(nullptr)
    , timeOffset_(0.0f)
    , frameBufferDirty_(true)
    , frameBufferInitialized_(false)
    , lastRenderTime_(-1.0f)
    , boundsNeedUpdate_(true)
{
}

CompositionSource::~CompositionSource() {
    // フレームバッファのクリア
    if (frameBufferInitialized_) {
        frameBuffer_.clear();
    }
}

bool CompositionSource::load(const std::filesystem::path& filepath) {
	composition_ = std::make_shared<Composition>();
	return composition_->load(filepath);
}

void CompositionSource::update(float currentTime) {
    if (!composition_) {
        return;
    }
    
    float adjustedTime = currentTime + timeOffset_;
    
    // 時間変化チェック
    if (shouldUpdateFrameBuffer(adjustedTime)) {
        markFrameBufferDirty();
        lastRenderTime_ = adjustedTime;
    }
    
    // Compositionの更新（CompositionLayer::update()から移植）
    composition_->setCurrentFrame(static_cast<int>(adjustedTime));
    composition_->update();
    
    // バウンズの無効化
    if (frameBufferDirty_) {
        boundsNeedUpdate_ = true;
    }
}

void CompositionSource::draw(const RenderContext& context) const {
    if (!composition_) {
        return;
    }
    
    // フレームバッファを使用する場合
    if (frameBufferInitialized_) {
        if (frameBufferDirty_) {
            updateFrameBuffer();
            frameBufferDirty_ = false;
        }
        
        // フレームバッファからの描画（CompositionLayer::draw()から移植）
        ofPushMatrix();
        ofTranslate(context.x, context.y);
        ofScale(context.w / getWidth(), context.h / getHeight());
        frameBuffer_.draw(0, 0);
        ofPopMatrix();
    } else {
        // 直接描画
        renderComposition();
    }
}

float CompositionSource::getWidth() const {
    if (!composition_) {
        return 0.0f;
    }
    return composition_->getWidth();
}

float CompositionSource::getHeight() const {
    if (!composition_) {
        return 0.0f;
    }
    return composition_->getHeight();
}

ofRectangle CompositionSource::getBounds() const {
    if (boundsNeedUpdate_ || cachedBounds_.isEmpty()) {
        if (composition_) {
            cachedBounds_ = ofRectangle(0, 0, getWidth(), getHeight());
        } else {
            cachedBounds_ = ofRectangle(0, 0, 0, 0);
        }
        boundsNeedUpdate_ = false;
    }
    return cachedBounds_;
}

std::string CompositionSource::getDebugInfo() const {
    std::stringstream ss;
    ss << "CompositionSource[";
    if (composition_) {
        const auto& info = composition_->getInfo();
		ss << "name=" << info.layers.size() << "layers";
        ss << ", size=" << getWidth() << "x" << getHeight();
        ss << ", timeOffset=" << timeOffset_;
        ss << ", fbDirty=" << (frameBufferDirty_ ? "true" : "false");
        ss << ", fbInit=" << (frameBufferInitialized_ ? "true" : "false");
    } else {
        ss << "no composition";
    }
    ss << "]";
    return ss.str();
}

void CompositionSource::setComposition(std::shared_ptr<Composition> composition) {
    composition_ = composition;
    invalidateFrameBuffer();
    boundsNeedUpdate_ = true;
    
    // フレームバッファのセットアップ
    if (composition_ && canCache()) {
        setupFrameBuffer();
    }
}

void CompositionSource::renderComposition() const {
    if (!composition_) {
        return;
    }
    
    // 現在の時間でコンポジションをレンダリング
    float currentTime = lastRenderTime_ >= 0 ? lastRenderTime_ : 0.0f;
    composition_->draw(0, 0, getWidth(), getHeight());
}

void CompositionSource::setupFrameBuffer() {
    if (!composition_) {
        return;
    }
    
    int width = static_cast<int>(getWidth());
    int height = static_cast<int>(getHeight());
    
    if (width > 0 && height > 0) {
        ensureFrameBufferSize(width, height);
        frameBufferInitialized_ = true;
        frameBufferDirty_ = true;
    }
}

void CompositionSource::updateFrameBuffer() const {
    if (!frameBufferInitialized_ || !composition_) {
        return;
    }
    
    // フレームバッファに描画（CompositionLayer::renderToFbo()から移植）
    frameBuffer_.begin();
    ofClear(0, 0, 0, 0); // 透明でクリア
    
    // コンポジションをレンダリング
    composition_->draw(0, 0, frameBuffer_.getWidth(), frameBuffer_.getHeight());
    
    frameBuffer_.end();
    
    frameBufferDirty_ = false;
}

void CompositionSource::clearFrameBuffer() {
    if (frameBufferInitialized_) {
        frameBuffer_.clear();
        frameBufferInitialized_ = false;
    }
    frameBufferDirty_ = true;
}

bool CompositionSource::parseCompositionProperties(const ofJson& data) {
    // 基本プロパティの解析
    try {
        if (data.contains("width") && data["width"].is_number()) {
            // 幅の情報は参考程度（Compositionクラスが管理）
        }
        
        if (data.contains("height") && data["height"].is_number()) {
            // 高さの情報は参考程度（Compositionクラスが管理）
        }
        
        if (data.contains("duration") && data["duration"].is_number()) {
            // 長さの情報は参考程度（Compositionクラスが管理）
        }
        
        return true;
    } catch (const std::exception& e) {
        ofLogError("CompositionSource") << "Parse properties error: " << e.what();
        return false;
    }
}

void CompositionSource::invalidateFrameBuffer() const {
    frameBufferDirty_ = true;
    boundsNeedUpdate_ = true;
}

void CompositionSource::ensureFrameBufferSize(int width, int height) const {
    if (!frameBufferInitialized_ || 
        frameBuffer_.getWidth() != width || 
        frameBuffer_.getHeight() != height) {
        
        // CompositionLayer::setFboSize()から移植
        const_cast<CompositionSource*>(this)->frameBuffer_.clear();
        const_cast<CompositionSource*>(this)->frameBuffer_.allocate(width, height, GL_RGBA);
        frameBufferInitialized_ = true;
        frameBufferDirty_ = true;
    }
}

void CompositionSource::initializeFrameBuffer() const {
    if (composition_ && !frameBufferInitialized_) {
        int width = static_cast<int>(getWidth());
        int height = static_cast<int>(getHeight());
        if (width > 0 && height > 0) {
            ensureFrameBufferSize(width, height);
        }
    }
}

bool CompositionSource::shouldUpdateFrameBuffer(float currentTime) const {
    const float TIME_EPSILON = 0.001f;
    return std::abs(currentTime - lastRenderTime_) > TIME_EPSILON;
}

void CompositionSource::markFrameBufferDirty() const {
    frameBufferDirty_ = true;
    boundsNeedUpdate_ = true;
}

}}
