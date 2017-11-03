#version 430 core

layout(location = 0) in vec3 appVertex;
layout(location = 1) in vec3 appNormal;
layout(location = 2) in vec2 appTexCoord2;
layout(location = 3) in float weight0;
layout(location = 4) in float weight1;
layout(location = 5) in float weight2;
layout(location = 6) in int weightIndex0;
layout(location = 7) in int weightIndex1;
layout(location = 8) in int weightIndex2;

layout(location = 0) out vec3 shaderVertex_EC;
layout(location = 1) out vec3 shaderNormal_EC;
layout(location = 2) out vec2 shaderTexCoord;

uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 modelViewInverseMatrix;
uniform mat4 globalMatrix[100];


void main(){
	shaderVertex_EC = vec3(modelViewMatrix * vec4(appVertex, 1.0f));

	shaderNormal_EC = normalize(modelViewInverseMatrix * appNormal);

	shaderTexCoord = appTexCoord2;

	//gl_Position = modelViewProjectionMatrix * vec4(appVertex, 1.0);
	if(weight0 == 0){
		gl_Position = modelViewProjectionMatrix * vec4(appVertex, 1.0);
	}
	else {
		vec4 vertex0 = globalMatrix[weightIndex0] * vec4(appVertex, 1.0f);
		//vec4 vertex1 = weight1 * globalMatrix[weightIndex1] * vec4(appVertex, 1.0f);

		vec4 newVertex = vec4(vertex0.xyz, 1.0f);

		gl_Position = modelViewProjectionMatrix * newVertex;
		//gl_Position = modelViewProjectionMatrix * vec4(appVertex, 1.0);
		//gl_Position = vec4(0.9, 0.9, 0.0, 1.0);
	}
}