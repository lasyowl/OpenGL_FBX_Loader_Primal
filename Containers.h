#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <list>

struct FrameData {
	unsigned int frameListID;
	long long int *keyTime;
	float *transformValue;
	int numKeyTime;
	int currentKey;
};

struct AnimNode {
	unsigned int nodeID;
	char type; // R,S,T
	FrameData *frameData[3];
	float defaultTransform[3];
};

struct Texture {
	unsigned int texID;
	char *name;
	char sourcePath[200];
};

struct Material {
	unsigned int matID;
	unsigned int lTexID;
	char *name;
	bool isLayered;
	bool texLinked;
	GLfloat color_ambient[4];
	GLfloat color_diffuse[4];
	GLfloat color_specular[4];
	GLfloat color_emissive[4];
	Texture *texture;
};

struct Model {
	char *type;
	unsigned int modelID;
	int modelNum;
	char *name;
	float *vertex;
	int *index;
	float *normal;
	unsigned int numVertex, numIndex, numNormal;
	float preRot[3];
	float lclTrans[3], lclRot[3], lclScale[3];
	int frame;
	glm::mat4 TransformMatrix;
	glm::mat4 TransformLinkMatrix;
	AnimNode *animNodeR, *animNodeS, *animNodeT;
	Material *material;
	Model *parent;
	Model *child;
	glm::mat4 GlobalMatrix;
};

struct Deformer {
	bool isSubDeformer;
	unsigned int deformerID;
	int *weightIndex;
	float *weight;
	int numWeight;
	glm::mat4 transform;
	glm::mat4 transformLink;
	std::list<Deformer*> subDeformer;
	Model *model;
	int numSubDeformer;
};

struct Geometry {
	unsigned int geomID;
	char *name;
	float *vertex;
	float **vertWeight;
	float **vertWeightDirect;
	int **weightMatrixIndex;
	int **weightMatrixIndexDirect;
	int *vertIndex;
	float *vertDirect;
	float *normal;
	float *normDirect;
	float *uv;
	float *uvDirect;
	int *uvIndex;
	unsigned int numVertex, numVertIndex, numNormal, numUV, numUVIndex;
	bool isDirect;
	Model *model;
	Deformer *deformer;
};

struct Animation {
	AnimNode *node;
	FrameData *frameData;
};

struct Object {
	char *name;
	Model *model;
	glm::mat4 *BoneMatrix;
	glm::mat4 *GlobalMatrix;
	Texture *texture;
	Material *material;
	Geometry *geometry;
	Animation *animation;
	AnimNode *animNode;
	FrameData *frameData;
	Deformer *deformer;
	int numModel, numGeom, numMat, numDeform, numTex, numFrmData, numAnimNode;
	GLuint *vao_mesh;
	GLuint **vbo_mesh;
	GLuint *vao_mesh_index;
	GLuint *vao_node;
	GLuint *vbo_node;
	float *vertex_node;
	GLuint *textureID;
};
