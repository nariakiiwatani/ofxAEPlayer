#include "ofxAETrackMatte.h"
#include "ofShader.h"

namespace ofx { namespace ae {
std::unique_ptr<ofShader> createShaderForTrackMatteType(TrackMatteType type) {
	std::string vertex = R"(#version 150
uniform mat4 uLayerToMatte;
in vec2 position;
out vec2 vMatteUV;

void main(){
	vec4 m = uLayerToMatte * vec4(position, 0.0, 1.0);
	vMatteUV = m.xy;
	gl_Position = vec4(position, 0.0, 1.0);
})";
	std::string fragment = R"(#version 150
uniform sampler2DRect matte;
in vec2 vMatteUV;
out vec4 fragColor;

float luma(vec3 c){ return dot(c, vec3(0.2126,0.7152,0.0722)); }

void main(){
   vec4 m = texture(matte, vMatteUV);
   float k = m.a;
   fragColor = vec4(k, k, k, 1);
}
)";
	auto ret = std::make_unique<ofShader>();
	ret->setupShaderFromSource(GL_VERTEX_SHADER, vertex);
	ret->setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
	ret->linkProgram();
	return ret;
}

}}
