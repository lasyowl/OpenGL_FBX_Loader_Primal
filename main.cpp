#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <Windows.h>
#include <time.h>
#include <vector>
#include <string>
#include "Parse_FBX.h"
#include "Parse_Shader.h"
#include "Variables.h"
#include "GlobalFunctions.h"

using namespace std;
using namespace glm;

GLuint program, firstProgram;

Object *bastillon;
Object *cube;
GLuint *floor_vao;
GLuint fbo;
GLuint color_texture, depth_texture;
GLfloat color[]{ 0.2f, 0.2f, 0.4f, 0.7f };
GLfloat colorPlus[]{ 0.4f, 0.2f, 0.2f, 0.7f };

int multipassTime = 0, timerms = 0;

bool framebufferMode = false;
bool useTextureFlag = true;
bool indexMode;
bool fillMode = true;
bool nodeMode = false;
bool animMode = false;

float m = 0, n = 0, k = 0, j = -150.0f;
float cameraZ = 1000.0f;

void LoadTexture(const char *filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP *tx_pixmap, *tx_pixmap_32;

	int width, height;
	GLvoid *data;

	if (*filename == NULL) { 
		return; 
	}
	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		//fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	cout << filename << endl;
	fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

}



void Mouse(int button, int state, int x, int y) {

	switch (button) {

	}
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'g':
			nodeMode = !nodeMode;
			glutPostRedisplay();
			break;
		case 'y':
			animMode = !animMode;
			glutPostRedisplay();
			break;
		case 't':
			framebufferMode = !framebufferMode;
			glutPostRedisplay();
			break;
		case 'a':
			m -= 0.5;
			glutPostRedisplay();
			break;
		case 'd':
			m += 0.5;
			glutPostRedisplay();
			break;
		case 'w':
			n += 0.5;
			glutPostRedisplay();
			break;
		case 's':
			n -= 0.5;
			glutPostRedisplay();
			break;
		case 'q':
			cameraZ += 50.0f;
			glutPostRedisplay();
			break;
		case 'e':
			cameraZ -= 50.0f;
			glutPostRedisplay();
			break;
		case 'r':
			useTextureFlag = true;
			glutPostRedisplay();
			break;
		case 'f':
			useTextureFlag = false;
			glutPostRedisplay();
			break;
		case ' ':
			fillMode = !fillMode;
			if (fillMode == true) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glutPostRedisplay();
			break;
		default:
			break;
	}
}
void GetProgramVarLocation() {
	loc_MV = glGetUniformLocation(program, "modelViewMatrix");
	loc_MVP = glGetUniformLocation(program, "modelViewProjectionMatrix");
	loc_MVI = glGetUniformLocation(program, "modelViewInverseMatrix");
	loc_BM = glGetUniformLocation(program, "globalMatrix");

	loc_tex = glGetUniformLocation(program, "textureSampler");
	useTexture = glGetUniformLocation(program, "useTexture");
	loc_ambient = glGetUniformLocation(program, "color_ambient");
	loc_diffuse = glGetUniformLocation(program, "color_diffuse");
	loc_specular = glGetUniformLocation(program, "color_specular");
}

int frameSpeed = 50;

void PrintMatrix(mat4 matrix) {
	for (int x = 0; x < 4; x++) {
		cout << "[ ";
		for (int y = 0; y < 4; y++) {
			printf("%.6f ", matrix[x][y]);
		}
		cout << "]" << endl;
	}
	cout << endl;
}

mat4 CalcModelMat(mat4 input, Model *currentModel, Model *rootModel) {

	// currentGlobalMatrix = parentGlobalMatrix * currentLocalMatrix
	// Get parentGlobalMatrix
	if (currentModel->parent != nullptr) {
		//cout << "recursion : " << currentModel->name << endl;
		input = CalcModelMat(input, currentModel->parent, rootModel);
	}
	else {
		//cout << "root : " << currentModel->name << endl;
	}

	// If animation node exists
	if ((currentModel->animNodeR != nullptr || currentModel->animNodeT != nullptr || currentModel->animNodeS != nullptr) && animMode == true) {
		FrameData *currentFrmDataR[3], *currentFrmDataT[3];
		float dataT[3];

		if (currentModel->animNodeT != nullptr) {
			currentFrmDataT[0] = currentModel->animNodeT->frameData[0];
			currentFrmDataT[1] = currentModel->animNodeT->frameData[1];
			currentFrmDataT[2] = currentModel->animNodeT->frameData[2];
			for (int i = 0; i < 3; i++) {
				if (currentFrmDataT[i] != nullptr) {
					dataT[i] = currentFrmDataT[i]->transformValue[currentFrmDataT[i]->currentKey];
				}
				else {
					dataT[i] = currentModel->lclTrans[i];
				}
			}
			input = translate(input, vec3(dataT[0], dataT[1], dataT[2]));
		}
		else {
			input = translate(input, vec3(currentModel->lclTrans[0], currentModel->lclTrans[1], currentModel->lclTrans[2]));
		}

		input = rotate(input, radians(currentModel->preRot[2]), vec3(0.0f, 0.0f, 1.0f));
		input = rotate(input, radians(currentModel->preRot[1]), vec3(0.0f, 1.0f, 0.0f));
		input = rotate(input, radians(currentModel->preRot[0]), vec3(1.0f, 0.0f, 0.0f));

		if (currentModel->animNodeR != nullptr) {
			currentFrmDataR[0] = currentModel->animNodeR->frameData[0];
			currentFrmDataR[1] = currentModel->animNodeR->frameData[1];
			currentFrmDataR[2] = currentModel->animNodeR->frameData[2];
			input = rotate(input, radians(currentFrmDataR[2]->transformValue[currentFrmDataR[2]->currentKey]), vec3(0.0, 0.0, 1.0));
			input = rotate(input, radians(currentFrmDataR[1]->transformValue[currentFrmDataR[1]->currentKey]), vec3(0.0, 1.0, 0.0));
			input = rotate(input, radians(currentFrmDataR[0]->transformValue[currentFrmDataR[0]->currentKey]), vec3(1.0, 0.0, 0.0));
		}
		else {
			input = rotate(input, radians(currentModel->lclRot[2]), vec3(0.0f, 0.0f, 1.0f));
			input = rotate(input, radians(currentModel->lclRot[1]), vec3(0.0f, 1.0f, 0.0f));
			input = rotate(input, radians(currentModel->lclRot[0]), vec3(1.0f, 0.0f, 0.0f));
		}
		rootModel->frame++;
		if (rootModel->frame % frameSpeed == 0) {
			for (int j = 0; j < 3; j++) {
				if (currentFrmDataR[j]->currentKey == currentFrmDataR[j]->numKeyTime - 1) {
					currentFrmDataR[j]->currentKey = 0;
				}
				else {
					currentFrmDataR[j]->currentKey++;
				}
			}
		}
	}
	// If animation node does not exist
	else {
		input = translate(input, vec3(currentModel->lclTrans[0], currentModel->lclTrans[1], currentModel->lclTrans[2]));
		input = rotate(input, radians(currentModel->preRot[2]), vec3(0.0f, 0.0f, 1.0f));
		input = rotate(input, radians(currentModel->preRot[1]), vec3(0.0f, 1.0f, 0.0f));
		input = rotate(input, radians(currentModel->preRot[0]), vec3(1.0f, 0.0f, 0.0f));
		input = rotate(input, radians(currentModel->lclRot[2]), vec3(0.0f, 0.0f, 1.0f));
		input = rotate(input, radians(currentModel->lclRot[1]), vec3(0.0f, 1.0f, 0.0f));
		input = rotate(input, radians(currentModel->lclRot[0]), vec3(1.0f, 0.0f, 0.0f));
	}
	input = scale(input, vec3(currentModel->lclScale[0], currentModel->lclScale[1], currentModel->lclScale[2]));
	currentModel->GlobalMatrix = input;
	return input;
}

void CalculateBoneMatrix(Object *object, Model *model, int index) {

	ModelMatrix = UnitMatrix;
	EyeMatrix = lookAt(vec3(-cameraZ, 0.5 * cameraZ, 100.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	//EyeMatrix = lookAt(vec3(0.0f, 3.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	//ModelMatrix = translate(ModelMatrix, vec3(0.0f, 100.0f, 0.0f));
	//ModelMatrix = rotate(ModelMatrix, (float)clock() / 2000, RotAxis);
	mat4 TempMatrix = UnitMatrix;
	TempMatrix = translate(TempMatrix, vec3(model->lclTrans[0], model->lclTrans[1], model->lclTrans[2]));
	TempMatrix = rotate(TempMatrix, radians(model->preRot[2]), vec3(0.0f, 0.0f, 1.0f));
	TempMatrix = rotate(TempMatrix, radians(model->preRot[1]), vec3(0.0f, 1.0f, 0.0f));
	TempMatrix = rotate(TempMatrix, radians(model->preRot[0]), vec3(1.0f, 0.0f, 0.0f));
	TempMatrix = rotate(TempMatrix, radians(model->lclRot[2]), vec3(0.0f, 0.0f, 1.0f));
	TempMatrix = rotate(TempMatrix, radians(model->lclRot[1]), vec3(0.0f, 1.0f, 0.0f));
	TempMatrix = rotate(TempMatrix, radians(model->lclRot[0]), vec3(1.0f, 0.0f, 0.0f));

	mat4 TempMatrix2 = inverse(model->TransformLinkMatrix);
	ModelMatrix = CalcModelMat(ModelMatrix, model, model);
	object->GlobalMatrix[index] = ModelMatrix * TempMatrix2;
	//object->GlobalMatrix[index] = model->GlobalMatrix;
	/*cout << index << " ========================\n";
	PrintMatrix(ModelMatrix);
	PrintMatrix(TempMatrix);
	PrintMatrix(object->GlobalMatrix[index]);*/
	//object->BoneMatrix[index] = model->TransformLinkMatrix * TempMatrix;

	ProjectionMatrix = perspective(radians(90.0f), 800.0f / 800.0f, 0.1f, 10000.0f);
	if (framebufferMode == true) {
		ModelViewMatrix = FixedEyeMatrix * ModelMatrix;
	}
	//else ModelViewMatrix = EyeMatrix * object->BoneMatrix[index];
	//else ModelViewMatrix = EyeMatrix * object->GlobalMatrix[index];
	//else ModelViewMatrix = EyeMatrix * ModelMatrix;
	//else ModelViewMatrix = EyeMatrix * model->TransformLinkMatrix;
	else ModelViewMatrix = EyeMatrix * model->TransformMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewInverseMatrix = inverseTranspose(mat3(ModelViewMatrix));
}

void DrawNodes(Object *object, int mode) {
	for (int i = 0; i < object->numModel; i++) {
		CalculateBoneMatrix(object, &object->model[i], i);
		if (mode == 1) {
			// mode가 1일 경우 bone matrix만 계산
			// 그 외의 경우 bone matrix를 점으로 그림
		}
		else {
			GLfloat zero[] = { 0, 0, 0, 0 };
			glUniform4fv(loc_ambient, 1, zero);
			glUniform4fv(loc_diffuse, 1, zero);
			glUniform4fv(loc_specular, 1, zero);

			glUniform1i(loc_tex, i);
			glUniform1i(useTexture, useTextureFlag);
			glUniformMatrix4fv(loc_MV, 1, GL_FALSE, &ModelViewMatrix[0][0]);
			glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix3fv(loc_MVI, 1, GL_FALSE, &ModelViewInverseMatrix[0][0]);
			glBindVertexArray(object->vao_node[i]);
			glPointSize(4);
			glDrawArrays(GL_POINTS, 0, 1);
			glBindVertexArray(0);
			//vec4 result = ModelMatrix * vec4(0, 0, 0, 1);
			//cout << object->model[i].name << ": " << result[0] << " " << result[1] << " " << result[2] << " " << result[3] << endl;
		}
		
	}
	//Sleep(10000);
}
void DrawObject(Object *object, int s, float rotX, float rotY) {
	if (nodeMode == true) {
		DrawNodes(object, 0);
		return;
	}
	else {
		DrawNodes(object, 1);
	}
	for (int i = 0; i < object->numGeom; i++) {

		ModelMatrix = UnitMatrix;
		EyeMatrix = lookAt(vec3(-cameraZ, 0.5 * cameraZ, 100.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		//EyeMatrix = lookAt(vec3(0.0f, 3.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		//ModelMatrix = translate(ModelMatrix, vec3(0.0f, 100.0f, 0.0f));
		//ModelMatrix = rotate(ModelMatrix, (float)clock()/2000, RotAxis);
		ModelMatrix = rotate(ModelMatrix, radians(rotY), vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = rotate(ModelMatrix, radians(rotX), RotAxis);
		ModelMatrix = rotate(ModelMatrix, n, vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = rotate(ModelMatrix, m, vec3(0.0f, 1.0f, 0.0f));

		ModelMatrix = scale(ModelMatrix, vec3(s, s, s));

		ModelMatrix = CalcModelMat(ModelMatrix, object->geometry[i].model, object->geometry[i].model);


		ProjectionMatrix = perspective(radians(90.0f), 800.0f / 800.0f, 0.1f, 10000.0f);
		if (framebufferMode == true) {
			ModelViewMatrix = FixedEyeMatrix * ModelMatrix;
		}
		else ModelViewMatrix = EyeMatrix * ModelMatrix;
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewInverseMatrix = inverseTranspose(mat3(ModelViewMatrix));

		glUniform4fv(loc_ambient, 1, object->geometry[i].model->material->color_ambient);
		glUniform4fv(loc_diffuse, 1, object->geometry[i].model->material->color_diffuse);
		glUniform4fv(loc_specular, 1, object->geometry[i].model->material->color_specular);

		for (int n = 0; n < object->geometry[1].numVertIndex; n++) {
			//if(object->geometry[0].weightMatrixIndexDirect[0][n]>28)
			//cout << object->geometry[0].weightMatrixIndexDirect[0][n] << endl;
			//cout << object->geometry[1].vertWeightDirect[0][n] << endl;
			//cout << object->geometry[0].vertWeightDirect[0][n] << " : " << object->geometry[0].weightMatrixIndexDirect[0][n] << endl;
			//if (object->geometry[i].vertWeightDirect[0][n] + object->geometry[i].vertWeightDirect[1][n] + object->geometry[i].vertWeightDirect[2][n] != 1) {
			//	//Sleep(100);
			//}
			//cout << n << "  " << object->geometry[0].weightMatrixIndexDirect[0][n] << endl;
			//cout << object->geometry[i].vertWeightDirect[0][n] << "  " << object->geometry[i].vertWeightDirect[1][n] << "  "<< object->geometry[i].vertWeightDirect[2][n] << endl;
		}


		//cout << object->model[2].name << endl;
		//PrintMatrix(ModelMatrix);

		//for (int x = 0; x < object->numModel; x++) {
			//cout << x << endl;
			//PrintMatrix(ModelMatrix);
			//PrintMatrix(object->GlobalMatrix[x]);
			//object->GlobalMatrix[x] = scale(UnitMatrix, vec3(1, 1, 1));
		//}
		//Sleep(10000);
		indexMode = !object->geometry[i].isDirect;

		glUniformMatrix4fv(loc_BM, object->numModel, GL_FALSE, &object->GlobalMatrix[0][0][0]);
		glUniform1i(loc_tex, i);
		glUniform1i(useTexture, useTextureFlag);
		glUniformMatrix4fv(loc_MV, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_MVI, 1, GL_FALSE, &ModelViewInverseMatrix[0][0]);
		glBindVertexArray(object->vao_mesh[i]);
		if (indexMode == true) {
			glDrawElements(GL_TRIANGLES, object->geometry[i].numVertIndex, GL_UNSIGNED_INT, (void*)0);
		}
		else glDrawArrays(GL_TRIANGLES, 0, object->geometry[i].numVertex);
		glBindVertexArray(0);
	}
}

void RefreshFBO() {
	if (framebufferMode == true) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glActiveTexture(GL_TEXTURE0 + 30);
		glBindTexture(GL_TEXTURE_2D, color_texture);
	}
	else glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat one[] = { 1.0, 1.0, 1.0, 1.0 };
	glClearBufferfv(GL_COLOR, 0, color);
	glClearBufferfv(GL_DEPTH, 0, one);
}
void DrawRect() {

	GLfloat cc[4] = { 1.0, 1.0, 1.0, 1.0 };

	glUniform4fv(loc_ambient, 1, cc);
	glUniform4fv(loc_diffuse, 1, cc);
	glUniform4fv(loc_specular, 1, cc);

	glUniform1i(useTexture, useTextureFlag);
	if (framebufferMode == true) {
		glUniform1i(loc_tex, 30);
	}
	else glUniform1i(loc_tex, 100);
	glUniformMatrix4fv(loc_MV, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix3fv(loc_MVI, 1, GL_FALSE, &ModelViewInverseMatrix[0][0]);
	glBindVertexArray(floor_vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 18);
	glBindVertexArray(0);
}

void display() {
	GlobalFunctions timer1;
	timer1.Start();
	FixedEyeMatrix = lookAt(vec3(0.0f, 0.0f, 700.0f), vec3(0.0f, 150.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	glEnable(GL_DEPTH_TEST);
	glUseProgram(program);

	
	RefreshFBO();
	glViewport(0, 0, 800, 800);
	RotAxis = vec3(0.0f, 1.0f, 0.0f);
	DrawObject(bastillon, 1, 0, 0);
	timerms = timer1.End();

	GlobalFunctions timer;

	//if (framebufferMode == true) {
	//	timer.Start();
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//}
	//else {
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	//ModelMatrix = translate(ModelMatrix, vec3(0.0f, -0.5f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();
	//}

	//if (framebufferMode == true) {
	//	glEnable(GL_CULL_FACE);
	//	RotAxis = vec3(0.3f, 1.0f, 0.6f);
	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 90, 0);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//		glClearBufferfv(GL_COLOR, 0, color);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock()/1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(0.0f, 0.0f, -0.5f));
	//	ModelMatrix = rotate(ModelMatrix, radians(-180.0f), vec3(1.0f, 0.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();

	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 180, 0);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock() / 1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(0.0f, 0.0f, 0.5f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();

	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 270, 0);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock() / 1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(0.0f, 0.5f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();

	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 0, 90);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock() / 1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(0.5f, 0.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();

	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 0, 270);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock() / 1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, 0.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();

	//	RefreshFBO();
	//	DrawObject(bastillon, 1, 0, 0);
	//	if (framebufferMode == true) {
	//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	}
	//	ModelMatrix = rotate(UnitMatrix, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, (float)clock() / 1000, RotAxis);
	//	ModelMatrix = scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(0.0f, -0.5f, 0.0f));
	//	ModelMatrix = rotate(ModelMatrix, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	//	ModelMatrix = translate(ModelMatrix, vec3(-0.5f, -0.5f, 0.0f));
	//	ModelViewMatrix = EyeMatrix * ModelMatrix;
	//	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//	ModelViewInverseMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	//	DrawRect();
	//	multipassTime = timer.End();
	//}
	

	glutSwapBuffers();
}

void displayPerFrame() {
	static float time_ms = 0;
	static int time_s = 0;
	static int count = 0;
	if (clock() - time_ms > -1) {
		display();
		time_ms = clock();
		count++;
	}

	if ((clock() / CLOCKS_PER_SEC) != time_s) {
		time_s = clock() / CLOCKS_PER_SEC;
		cout << count << " FPS";
		cout << ", SinglePass : " << timerms << "ms\n";
		if (multipassTime != 0) {
			cout << ", Rendered through multipass(6) : " << multipassTime << "ms\n";
		}
		else cout << endl;
		multipassTime = 0;
		count = 0;
	}
}

void register_callback() {
	glutDisplayFunc(displayPerFrame);
	glutIdleFunc(displayPerFrame);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
}

void PrepareShaders() {
	ShaderInfo defaultShader[3] = {
		{GL_VERTEX_SHADER, "Shader/DefaultShader.vert"},
		{GL_FRAGMENT_SHADER, "Shader/DefaultShader.frag"},
		{GL_NONE, NULL}
	};
	ShaderInfo firstShader[3] = {
		{GL_VERTEX_SHADER, "Shader/FirstShader.vert"},
		{GL_FRAGMENT_SHADER, "Shader/FirstShader.frag"},
		{GL_NONE, NULL}
	};
	program = LoadShader(firstShader);
	glUseProgram(program);
	GetProgramVarLocation();
	//firstProgram = LoadShader(defaultShader);
}

GLfloat vert[18]{
	0.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 1.0f,1.0f,0.0f, 
	0.0f,0.0f,0.0f, 1.0f,1.0f,0.0f, 0.0f,1.0f,0.0f
};

GLfloat norm[18]{
	0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f,
	0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f
};
//GLfloat norm[18]{
//	-0.57735f,-0.57735f,0.57735f, 0.57735f,-0.57735f,0.57735f, 0.57735f,0.57735f,0.57735f,
//	-0.57735f,-0.57735f,0.57735f, 0.57735f,0.57735f,0.57735f, -0.333333f,0.333333f,0.333333f
//};
GLfloat uv[12]{
	0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f,
	0.0f,0.0f, 1.0f,1.0f, 0.0f,1.0f
};

GLuint* PrepareTerrain(const char *filePath) {
	GLuint *vao;
	GLuint *vbo;
	GLuint *texture;
	vao = new GLuint;
	vbo = new GLuint[3];
	texture = new GLuint;
	glGenVertexArrays(1, vao);
	glGenBuffers(3, vbo);
	glGenTextures(1, texture);
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(norm), norm, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glActiveTexture(GL_TEXTURE0 + 100);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	LoadTexture(filePath);
	glGenerateMipmap(GL_TEXTURE_2D); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return vao;
}

Object* PrepareObject(Object* object, const char *filePath) {
	Parse_FBX temp;
	temp.SetVersion(2);

	temp.Parse(filePath);
	
	cout << "Parse Done" << endl;
	object = temp.GetObjectPtr();
	GLuint *vbo_temp;
	object->vao_mesh = new GLuint[object->numGeom];
	object->vbo_mesh = new GLuint*[object->numGeom];
	vbo_temp = new GLuint[object->numGeom * 9];
	object->vao_mesh_index = new GLuint[object->numGeom];
	glGenVertexArrays(object->numGeom, object->vao_mesh);
	glGenBuffers(object->numGeom * 9, vbo_temp);
	for (int i = 0, j = 0; i < object->numGeom; i++, j+=9) {
		object->vbo_mesh[i] = new GLuint[9];
		for (int k = 0; k < 9; k++) {
			object->vbo_mesh[i][k] = vbo_temp[j + k];
		}
	}
	glGenBuffers(object->numGeom, object->vao_mesh_index);

	if (object->numTex > 0) {
		object->textureID = new GLuint[object->numTex];
		glGenTextures(object->numTex, object->textureID);
	}

	for (int i = 0; i < object->numGeom; i++) {

		glBindVertexArray(object->vao_mesh[i]);

		float *vertData, *normData, *uvData, **weightData;
		int **weightIndexData;
		weightData = new float*[3];
		weightIndexData = new int*[3];

		indexMode = !object->geometry[i].isDirect;

		if (indexMode == true) {
			vertData = object->geometry[i].vertex;
			normData = object->geometry[i].normal;
			uvData = object->geometry[i].uv;
			weightData[0] = object->geometry[i].vertWeight[0];
			weightData[1] = object->geometry[i].vertWeight[1];
			weightData[2] = object->geometry[i].vertWeight[2];
			weightIndexData[0] = object->geometry[i].weightMatrixIndex[0];
			weightIndexData[1] = object->geometry[i].weightMatrixIndex[1];
			weightIndexData[2] = object->geometry[i].weightMatrixIndex[2];
		}
		else {
			vertData = object->geometry[i].vertDirect;
			normData = object->geometry[i].normDirect;
			uvData = object->geometry[i].uvDirect;
			weightData[0] = object->geometry[i].vertWeightDirect[0];
			weightData[1] = object->geometry[i].vertWeightDirect[1];
			weightData[2] = object->geometry[i].vertWeightDirect[2];
			weightIndexData[0] = object->geometry[i].weightMatrixIndexDirect[0];
			weightIndexData[1] = object->geometry[i].weightMatrixIndexDirect[1];
			weightIndexData[2] = object->geometry[i].weightMatrixIndexDirect[2];
		}

		for (int j = 0; j < object->geometry[i].numVertex; j++) {
			//printf("%d: %f\n", j, object->geometry[i].vertex[j]);
		}

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertex, vertData);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numNormal, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numNormal, normData);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		
		if (object->numTex > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numUV, NULL, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numUV, uvData);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
			glEnableVertexAttribArray(2);
		}

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightData[0]);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][4]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightData[1]);
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][5]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightData[2]);
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][6]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightIndexData[0]);
		glVertexAttribIPointer(6, 1, GL_INT, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][7]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightIndexData[1]);
		glVertexAttribIPointer(7, 1, GL_INT, 0, (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_mesh[i][8]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->geometry[i].numVertIndex, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * object->geometry[i].numVertIndex, weightIndexData[2]);
		glVertexAttribIPointer(8, 1, GL_INT, 0, (GLvoid*)0);

		for (int x = 0; x < object->geometry[0].numVertIndex; x++) {
			if (object->geometry[0].weightMatrixIndexDirect[0][x] > 10000) cout << "OK";
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glEnableVertexAttribArray(8);

		if (indexMode == true) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->vao_mesh_index[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * object->geometry[i].numVertIndex, object->geometry[i].vertIndex, GL_STATIC_DRAW);
			//if (i == 1) {
			//	//cout << object->geometry[i].numVertex << " " << object->geometry[i].numNormal << endl;
			//	//printf("%f, %f, %f\n", object->geometry[i].model->lclScale[0], object->geometry[i].model->lclScale[1], object->geometry[i].model->lclScale[2]);
			//	for (int j = 0; j < object->geometry[i].numVertIndex; j++) {
			//		printf("(%d) %d, %d, %d : %f, %f, %f\n", object->geometry[i].vertIndex[j], object->geometry[i].vertIndex[j] * 3, object->geometry[i].vertIndex[j] * 3 + 1, object->geometry[i].vertIndex[j] * 3 + 2, object->geometry[i].vertex[3 * object->geometry[i].vertIndex[j]], object->geometry[i].vertex[3 * object->geometry[i].vertIndex[j]+1], object->geometry[i].vertex[3 * object->geometry[i].vertIndex[j]+2]);
			//	}
			//}
		}

		if (object->geometry[i].model->material->texLinked == true) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, object->textureID[i]);

			//cout << strlen(object->geometry[i].model->material->texture->sourcePath) << endl;
			LoadTexture(object->geometry[i].model->material->texture->sourcePath);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	object->vao_node = new GLuint[object->numModel];
	object->vbo_node = new GLuint[object->numModel];
	object->vertex_node = new float[object->numModel * 3]();
	glGenVertexArrays(object->numModel, object->vao_node);
	glGenBuffers(object->numModel, object->vbo_node);
	for (int i = 0; i < object->numModel; i++) {
		glBindVertexArray(object->vao_node[i]);
		glBindBuffer(GL_ARRAY_BUFFER, object->vbo_node[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3, object->vertex_node);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);
	}

	cout << "Buffering To GPU Done" << endl;
	return object;
}

void FBO_init() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 800, 800);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 800, 800);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Parse Test");
	glewExperimental = GL_TRUE;

	glewInit();
	
	PrepareShaders();
	FBO_init();
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/Soldier.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/BlackDragon.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/StickMan_noanim.fbx");
	bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/StickMan.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/StickMan_text.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Overwatch/Mercy/Mercy.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Overwatch/Bastillon/Bastillon2.fbx");
	//bastillon = PrepareObject(bastillon, "Bastillon2.fbx");
	//bastillon = PrepareObject(bastillon, "D:/3D Models/Export/Etc/Door.fbx");
	//bastillon = PrepareObject(bastillon, "Bastillon.fbx");
	//cube = PrepareObject(cube, "Cube.fbx");
	floor_vao = PrepareTerrain("Texture/checker_tex.jpg");
	//TestGLAttributes();
	register_callback();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	
	glutMainLoop();

	return 0;
}