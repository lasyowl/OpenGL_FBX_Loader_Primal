#pragma once
#include <fstream>
#include <iostream>
#include "Containers.h"

using namespace std;

class Parse_FBX {
public:
	Parse_FBX();
	Parse_FBX(const char *path);
	void SetVersion(int number);
	Object* GetObjectPtr();
	void Parse(const char *path);
	
private:
	char buffer_s[1000000];
	char buffer_c;
	float buffer_f;
	int buffer_i;
	bool endFlag;
	ifstream rawFile;
	enum ObjTypeDef { MODEL, GEOMETRY, MATERIAL, DEFORMER, TEXTURE, FRAMEDATA, ANIMATIONNODE, ETC = -1 };
	Object *object;
	int objType;
	int indModel, indTex, indMat, indDeform, indGeom, indAnimNode, indFrmData;
	int version;
	Model *currentModel;
	Geometry *currentGeom;
	Material *currentMat;
	Deformer *currentDeform;
	Texture *currentTex;
	Animation *currentAnim;
	AnimNode *currentAnimNode;
	FrameData *currentFrmData;

	void FindObjectType(const char *buffer);
	void Trianglize(float *);
	int InitObjectNumbers();
	void PreCount();
	float RawToFloat();
	int RawToIndex();
	int RawToInt();
	long long int RawToLLInt();
	int SeekUntil(char c);
	int SeekUntilExt(char c);
	void ParseVersion_1(const char *path);
	void ParseVersion_2(const char *path);
	float Normalize(float x, float y, float z);
};