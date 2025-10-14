#include "ofxAERenderContext.h"
#include "ofLog.h"

namespace ofx { namespace ae {
static std::stack<RenderContext> stack;
namespace {
RenderContext& getTop()
{
	static RenderContext root;
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

void RenderContext::setBlendMode(BlendMode mode)
{
	auto &ctx = getTop();
	ctx.blendMode = mode;
	// TODO: apply blendMode
}

}}
