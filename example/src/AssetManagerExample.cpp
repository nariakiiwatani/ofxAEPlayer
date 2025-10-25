#include "ofApp.h"
#include "ofxAEPlayer.h"
#include "../src/utils/ofxAEAssetManager.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofBackground(64, 64, 64);
    
    // Configure AssetManager memory limits
    auto& assetManager = ofx::ae::AssetManager::getInstance();
    assetManager.setTextureMemoryLimit(256);     // 256MB for textures
    assetManager.setVideoMemoryLimit(512);       // 512MB for videos
    assetManager.setCompositionMemoryLimit(128); // 128MB for compositions
    assetManager.setGlobalMemoryLimit(1024);     // 1GB total limit
    
    ofLogNotice("AssetManagerExample") << "AssetManager configured with memory limits";
    
    // Initialize variables
    isPlaying = false;
    showDebugInfo = true;
    showAssetStats = true;

    comp_ = std::make_shared<ofx::ae::Composition>();
    
    // Load composition - assets will be automatically cached via AssetManager
    if (comp_->load("Edit_Rough.json")) {
        ofLogNotice("AssetManagerExample") << "Composition loaded successfully";
        const auto& info = comp_->getInfo();
        ofLogNotice("AssetManagerExample") << "Duration: " << info.duration;
        ofLogNotice("AssetManagerExample") << "Layers: " << info.layers.size();
        
        // Log initial asset statistics
        assetManager.logCacheStats();
        
    } else {
        ofLogError("AssetManagerExample") << "Failed to load composition";
        comp_.reset();
    }
    
    timeline_ = 0;
    isPlaying = true;
    
    // Schedule periodic cache cleanup
    cleanupTimer_ = 0;
    cleanupInterval_ = 10.0f; // 10 seconds
}

//--------------------------------------------------------------
void ofApp::update(){
    if (isPlaying && comp_) {
        if(++timeline_ >= comp_->getInfo().duration) {
            timeline_ = 0;
        }
        if(comp_->setFrame(timeline_)) {
            comp_->update();
        }
    }
    
    // Periodic cache cleanup
    cleanupTimer_ += ofGetLastFrameTime();
    if (cleanupTimer_ >= cleanupInterval_) {
        auto& assetManager = ofx::ae::AssetManager::getInstance();
        assetManager.cleanup();
        cleanupTimer_ = 0;
        
        if (showAssetStats) {
            ofLogVerbose("AssetManagerExample") << "Periodic cache cleanup performed";
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(64, 64, 64);
    
    if (comp_) {
        comp_->draw(0, 0);
    }

    // Draw controls and debug info
    drawControls();
    if (showDebugInfo) {
        drawDebugInfo();
    }
    if (showAssetStats) {
        drawAssetStats();
    }
}

//--------------------------------------------------------------
void ofApp::drawControls(){
    ofSetColor(255);
    string controls = "AssetManager Example Controls:\n";
    controls += "SPACE: Play/Pause\n";
    controls += "R: Reset to beginning\n";
    controls += "D: Toggle debug info\n";
    controls += "A: Toggle asset statistics\n";
    controls += "C: Manual cache cleanup\n";
    controls += "S: Show cache statistics in console\n";
    controls += "M: Test memory limit (load extra assets)\n";
    controls += "LEFT/RIGHT: Seek -/+ 1 second";
    
    ofDrawBitmapString(controls, 20, 20);
}

//--------------------------------------------------------------
void ofApp::drawDebugInfo(){
    if (!comp_) return;

    ofSetColor(255);
    string info = "Debug Info:\n";
    info += "Playing: " + string(isPlaying ? "Yes" : "No") + "\n";
    info += "Frame: " + ofToString(comp_->getCurrentTime(), 2) + "s\n";
    const auto& compInfo = comp_->getInfo();
    float duration = static_cast<float>(compInfo.duration) / compInfo.fps;
    info += "Duration: " + ofToString(duration, 2) + "s\n";
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
    info += "Layers: " + ofToString(compInfo.layers.size()) + "\n";
    
    ofDrawBitmapString(info, 20, ofGetHeight() - 200);
}

//--------------------------------------------------------------
void ofApp::drawAssetStats(){
    auto& assetManager = ofx::ae::AssetManager::getInstance();
    auto stats = assetManager.getStats();
    
    ofSetColor(255, 255, 0); // Yellow for asset stats
    string assetInfo = "Asset Manager Statistics:\n";
    assetInfo += "=========================\n";
    assetInfo += "Total Memory: " + ofToString(stats.getTotalMemoryUsageMB()) + " MB\n";
    assetInfo += "Overall Hit Ratio: " + ofToString(stats.getOverallHitRatio() * 100.0, 1) + "%\n\n";
    
    assetInfo += "Texture Cache:\n";
    assetInfo += "  Items: " + ofToString(stats.texture_stats.cached_items.load()) + "\n";
    assetInfo += "  Memory: " + ofToString(stats.texture_stats.getMemoryUsageMB()) + " MB\n";
    assetInfo += "  Hits: " + ofToString(stats.texture_stats.hits.load()) + 
                 ", Misses: " + ofToString(stats.texture_stats.misses.load()) + "\n";
    assetInfo += "  Hit Ratio: " + ofToString(stats.texture_stats.getHitRatio() * 100.0, 1) + "%\n\n";
    
    assetInfo += "Video Cache:\n";
    assetInfo += "  Items: " + ofToString(stats.video_stats.cached_items.load()) + "\n";
    assetInfo += "  Memory: " + ofToString(stats.video_stats.getMemoryUsageMB()) + " MB\n";
    assetInfo += "  Hits: " + ofToString(stats.video_stats.hits.load()) + 
                 ", Misses: " + ofToString(stats.video_stats.misses.load()) + "\n";
    assetInfo += "  Hit Ratio: " + ofToString(stats.video_stats.getHitRatio() * 100.0, 1) + "%\n\n";
    
    assetInfo += "Composition Cache:\n";
    assetInfo += "  Items: " + ofToString(stats.composition_stats.cached_items.load()) + "\n";
    assetInfo += "  Memory: " + ofToString(stats.composition_stats.getMemoryUsageMB()) + " MB\n";
    assetInfo += "  Hits: " + ofToString(stats.composition_stats.hits.load()) + 
                 ", Misses: " + ofToString(stats.composition_stats.misses.load()) + "\n";
    assetInfo += "  Hit Ratio: " + ofToString(stats.composition_stats.getHitRatio() * 100.0, 1) + "%\n";
    
    ofDrawBitmapString(assetInfo, ofGetWidth() - 400, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    auto& assetManager = ofx::ae::AssetManager::getInstance();
    
    switch(key) {
        case ' ':
            isPlaying = !isPlaying;
            ofLogNotice("AssetManagerExample") << "Playback " << (isPlaying ? "started" : "paused");
            break;
            
        case 'r':
        case 'R':
            timeline_ = 0;
            ofLogNotice("AssetManagerExample") << "Reset to beginning";
            break;
            
        case 'd':
        case 'D':
            showDebugInfo = !showDebugInfo;
            break;
            
        case 'a':
        case 'A':
            showAssetStats = !showAssetStats;
            break;
            
        case 'c':
        case 'C':
            assetManager.cleanup();
            ofLogNotice("AssetManagerExample") << "Manual cache cleanup performed";
            break;
            
        case 's':
        case 'S':
            assetManager.logCacheStats();
            break;
            
        case 'm':
        case 'M':
            // Test memory management by loading additional assets
            testMemoryLimits();
            break;
            
        case OF_KEY_LEFT:
            timeline_ -= comp_ ? comp_->getInfo().fps : 30;
            timeline_ = std::max(0, timeline_);
            break;
            
        case OF_KEY_RIGHT:
            timeline_ += comp_ ? comp_->getInfo().fps : 30;
            if (comp_) {
                timeline_ = std::min(timeline_, comp_->getInfo().duration - 1);
            }
            break;
    }
}

//--------------------------------------------------------------
void ofApp::testMemoryLimits(){
    ofLogNotice("AssetManagerExample") << "Testing memory limits...";
    
    auto& assetManager = ofx::ae::AssetManager::getInstance();
    
    // Try to load multiple test assets to test memory management
    std::vector<std::string> testPaths = {
        "test_image_1.png",
        "test_image_2.jpg", 
        "test_video_1.mp4",
        "test_composition_1.json"
    };
    
    for (const auto& path : testPaths) {
        // Determine asset type from extension
        if (path.find(".png") != std::string::npos || path.find(".jpg") != std::string::npos) {
            auto texture = assetManager.getTexture(path);
            if (texture) {
                ofLogNotice("AssetManagerExample") << "Loaded test texture: " << path;
            }
        } else if (path.find(".mp4") != std::string::npos) {
            auto video = assetManager.getVideo(path);
            if (video) {
                ofLogNotice("AssetManagerExample") << "Loaded test video: " << path;
            }
        } else if (path.find(".json") != std::string::npos) {
            auto composition = assetManager.getComposition(path);
            if (composition) {
                ofLogNotice("AssetManagerExample") << "Loaded test composition: " << path;
            }
        }
    }
    
    // Log final statistics
    assetManager.logCacheStats();
}