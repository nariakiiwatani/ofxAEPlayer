#include "ofxAERenderContext.h"
#include "ofLog.h"

namespace ofx { namespace ae {
struct Style {
	ofFloatColor color{1,1,1,1};
	BlendMode blendMode{BlendMode::NORMAL};
};
static std::stack<Style> stack;
namespace {
Style& getTop()
{
	static Style root;
	return stack.empty() ? root : stack.top();
}
}
void RenderContext::push()
{
	stack.push(getTop());
	ofPushStyle();
}
void RenderContext::pop()
{
	ofPopStyle();
	if(stack.empty()) {
		ofLogWarning("ofxAERenderContext::pop") << "stack is empty. something is wrong.";
	}
	else {
		stack.pop();
	}
}

void RenderContext::setOpacity(float opacity)
{
	auto &ctx = getTop();
	ctx.color.a = opacity;
	ofSetColor(ctx.color);
}
void RenderContext::setColorRGB(ofFloatColor color)
{
	auto &ctx = getTop();
	ctx.color.r = color.r;
	ctx.color.g = color.g;
	ctx.color.b = color.b;
	ofSetColor(ctx.color);
}

void RenderContext::mulColorRGBA(ofFloatColor color)
{
	auto &ctx = getTop();
	ctx.color.r *= color.r;
	ctx.color.g *= color.g;
	ctx.color.b *= color.b;
	ctx.color.a *= color.a;
	ofSetColor(ctx.color);
}

void RenderContext::setBlendMode(BlendMode mode)
{
	auto &ctx = getTop();
	ctx.blendMode = mode;
	// TODO: apply blendMode
}

}}
