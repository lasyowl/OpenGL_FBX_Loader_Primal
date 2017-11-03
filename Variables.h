#pragma once
#include <GL/glew.h>
using namespace glm;

GLuint loc_MV;
GLuint loc_MVP;
GLuint loc_MVI;
GLuint loc_BM;

GLuint loc_tex;
GLuint useTexture;
GLuint loc_ambient;
GLuint loc_diffuse;
GLuint loc_specular;

vec3 RotAxis;
mat4 UnitMatrix = mat4(1.0f);
mat4 ModelMatrix;
mat4 EyeMatrix;
mat4 FixedEyeMatrix;
mat4 ModelViewMatrix;
mat4 ProjectionMatrix;
mat4 ModelViewProjectionMatrix;
mat3 ModelViewInverseMatrix;