#pragma once

#include "ofVideoPlayer.h"
#include "ofImage.h"
#include "ofAppRunner.h"
#include "ofFileUtils.h"
#include "ofPixels.h"


class TexturePlayer;
class SequencePlayer;
namespace VideoLoader { 
template<typename T=ofVideoPlayer>
std::shared_ptr<T> load(const std::string &filepath) {
	auto ret = std::make_shared<T>();
	if(ret->load(filepath)) {
		return ret;
	}
	return nullptr;
}
template<>
std::shared_ptr<ofVideoPlayer> load(const std::string &filepath);

}
