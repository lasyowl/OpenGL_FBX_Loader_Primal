#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "Parse_Shader.h"

using namespace std;

GLuint LoadShader(ShaderInfo *shaderInfo) {
	GLuint programHandle;
	GLuint fileLength;
	ifstream shaderSource;
	string shaderString;

	programHandle = glCreateProgram();

	for (int i = 0; i < 3; i++) {
		if (shaderInfo[i].type == GL_NONE) {
			//cout << "There is no shaderinfo" << endl;
			continue;
		}
		shaderInfo[i].shader = glCreateShader(shaderInfo[i].type);
		shaderSource.open(shaderInfo[i].filename);
		shaderSource.seekg(0, ios::end);
		if (!shaderSource.is_open()) {
			cout << "Failed to open shader file\n";
			continue;
		}
		fileLength = shaderSource.tellg();
		shaderSource.seekg(0, ios::beg);
		shaderString.clear();
		shaderString.reserve(fileLength);
		while (1) {
			shaderString += shaderSource.get();
			if (shaderSource.eof()) {
				shaderString[shaderString.size() - 1] = '\0';
				break;
			}
		}
		shaderSource.close();
		GLchar *source = (GLchar *)shaderString.c_str();
		glShaderSource(shaderInfo[i].shader, 1, &source, NULL);
		glCompileShader(shaderInfo[i].shader);
		GLint isCompiled = GL_FALSE;
		glGetShaderiv(shaderInfo[i].shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shaderInfo[i].shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(shaderInfo[i].shader, maxLength, &maxLength, &errorLog[0]);

			std::cout << "SHADER COMPILATION ERROR:\n";
			for (std::vector<GLchar>::iterator iter = errorLog.begin(); iter != errorLog.end(); ++iter)
			{
				std::cout << *iter;
			}
			std::cout << "\n";
			glDeleteShader(shaderInfo[i].shader);
			continue;
		}
		glAttachShader(programHandle, shaderInfo[i].shader);
	}
	glLinkProgram(programHandle);
	GLint isLinked = 0;
	glGetProgramiv(programHandle, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(programHandle, maxLength, &maxLength, &infoLog[0]);
		std::cout << "PROGRAM LINK ERROR:\n";
		for (std::vector<GLchar>::iterator iter = infoLog.begin(); iter != infoLog.end(); ++iter)
		{
			std::cout << *iter;
		}
		std::cout << "\n";
		//We don't need the program anymore.
		glDeleteProgram(programHandle);
		//Don't leak shaders either.
		for (int i = 0; i < 3; i++) {
			glDeleteShader(shaderInfo[i].shader);
		}

		//Use the infoLog as you see fit.

		//In this simple program, we'll just leave
		return 0;
	}
	for (int i = 0; i < 3; i++) {
		if (shaderInfo[i].type == GL_NONE) {
			continue;
		}
		glDetachShader(programHandle, shaderInfo[i].shader);
		glDeleteShader(shaderInfo[i].shader);
	}
	return programHandle;
}
