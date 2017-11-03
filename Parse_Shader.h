#pragma once
#include <GL/glew.h>

struct ShaderInfo {
	GLenum type;
	const char *filename;
	GLuint shader;
};

GLuint LoadShader(ShaderInfo *shaderInfo);