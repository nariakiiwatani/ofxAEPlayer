#include "ofxAETrackMatte.h"
#include "ofShader.h"

namespace ofx { namespace ae {
std::unique_ptr<ofShader> createShaderForTrackMatteType(TrackMatteType type) {
	std::string vertex = R"(#version 150
uniform mat4 modelViewProjectionMatrix;
uniform mat4 uLayerToMatte;
uniform vec2 matteOffset;
in vec2 position;
out vec2 vMatteUV;

void main(){
	vec4 m = uLayerToMatte * vec4(position, 0.0, 1.0);
	vMatteUV = m.xy+matteOffset;
	gl_Position = modelViewProjectionMatrix * vec4(position, 0.0, 1.0);
})";
	std::string fragment = R"(#version 150
uniform sampler2DRect matte;
in vec2 vMatteUV;
out vec4 fragColor;

float luma(vec3 c){ return dot(c, vec3(0.2126,0.7152,0.0722)); }
float val(vec4 a);

void main(){
   vec4 m = texture(matte, vMatteUV);
   float k = val(m);
   fragColor = vec4(k, k, k, k);
}
)";
	switch(type) {
		case TrackMatteType::ALPHA: fragment += R"(float val(vec4 c) { return c.a; })"; break;
		case TrackMatteType::ALPHA_INVERTED: fragment += R"(float val(vec4 c) { return 1.0-c.a; })"; break;
		case TrackMatteType::LUMA: fragment += R"(float val(vec4 c) { return luma(c.rgb); })"; break;
		case TrackMatteType::LUMA_INVERTED: fragment += R"(float val(vec4 c) { return 1.0-luma(c.rgb); })"; break;
		default: fragment += R"(float val(vec4 c) { return c.a; })"; break;
	}
	auto ret = std::make_unique<ofShader>();
	ret->setupShaderFromSource(GL_VERTEX_SHADER, vertex);
	ret->setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
	ret->linkProgram();
	return ret;
}

}}
