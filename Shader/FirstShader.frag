#version 430 core

layout(location = 0) in vec3 shaderVertex_EC;
layout(location = 1) in vec3 shaderNormal_EC;
layout(location = 2) in vec2 shaderTexCoord;

layout(location = 0) out vec4 finalColor;

uniform sampler2D textureSampler;

uniform vec4 color_ambient;
uniform vec4 color_diffuse;
uniform vec4 color_specular;

uniform bool useTexture = false;

vec4 shadedColor;
vec4 loadedTexture;

void main(){

	float x = dot(vec3(0.0, 0.0, 1.0), shaderNormal_EC);
	if(x < 0) x = -x;

	shadedColor = x * (color_ambient + color_diffuse + color_specular);

	if((loadedTexture = texture(textureSampler, shaderTexCoord)) == vec4(0.0, 0.0, 0.0, 1.0)){
		finalColor = shadedColor * 0.8;
	}
	else if(useTexture == false){
		finalColor = shadedColor * 0.5;
	}
	else {
		finalColor = texture(textureSampler, shaderTexCoord) * shadedColor;
	}
}