#pragma once
#include <stdio.h>
#include <math.h>
#include "Parse_FBX.h"


Parse_FBX::Parse_FBX() : indModel(0), indTex(0), indMat(0), indDeform(0), indGeom(0), indAnimNode(0), indFrmData(0), endFlag(false), object(nullptr), version(1) {

}

Parse_FBX::Parse_FBX(const char *path) : indModel(0), indTex(0), indMat(0), indDeform(0), indGeom(0), indAnimNode(0), indFrmData(0), endFlag(false), object(nullptr), version(1) {
	Parse(path);
}

void Parse_FBX::SetVersion(int number) {
	version = number;
}

void Parse_FBX::FindObjectType(const char *buffer) {
	objType = 0;
	if (strcmp(buffer, "\"Model\"") == 0) objType = MODEL;
	else if (strcmp(buffer, "\"Geometry\"") == 0) objType = GEOMETRY;
	else if (strcmp(buffer, "\"Material\"") == 0) objType = MATERIAL;
	else if (strcmp(buffer, "\"Deformer\"") == 0) objType = DEFORMER;
	else if (strcmp(buffer, "\"Texture\"") == 0) objType = TEXTURE;
	else if (strcmp(buffer, "\"AnimationCurve\"") == 0) objType = FRAMEDATA;
	else if (strcmp(buffer, "\"AnimationCurveNode\"") == 0) objType = ANIMATIONNODE;
	else objType = ETC;
}

int Parse_FBX::InitObjectNumbers() {

	// ObjectType:  À» Ã£À»¶§±îÁö °Ë»ö
	while (strcmp(buffer_s, "ObjectType:") != 0) {
		rawFile >> buffer_s;
		//cout << buffer_s << endl;
		//cout << (char)rawFile.get();
		if (strcmp(buffer_s, "properties") == 0) {
			//cout << buffer_s;
			return -1;
		}
	}
	rawFile >> buffer_s;
	//cout << buffer_s << objType << endl;
	FindObjectType(buffer_s);

	// Object °¹¼ö °Ë»ö
	while (strcmp(buffer_s, "Count:") != 0) {
		rawFile >> buffer_s;
	}

	switch (objType) {
	case MODEL:
		rawFile >> object->numModel;
		object->model = new Model[object->numModel];
		for (int i = 0; i < object->numModel; i++) {
			for (int j = 0; j < 3; j++) {
				object->model[i].lclRot[j] = object->model[i].lclTrans[j] = object->model[i].preRot[j] = 0;
				object->model[i].lclScale[j] = 1;
				object->model[i].animNodeR = nullptr;
				object->model[i].animNodeS = nullptr;
				object->model[i].animNodeT = nullptr;
				object->model[i].parent = nullptr;
				object->model[i].child = nullptr;
				object->model[i].frame = 0;
			}
		}
		break;
	case GEOMETRY:
		rawFile >> object->numGeom;
		object->geometry = new Geometry[object->numGeom];
		for (int i = 0; i < object->numGeom; i++) {
			object->geometry[i].isDirect = false;
			object->geometry[i].numUV = 0;
			object->geometry[i].deformer = nullptr;
		}
		break;
	case MATERIAL:
		rawFile >> object->numMat;
		object->material = new Material[object->numMat];
		for (int i = 0; i < object->numMat; i++) {
			object->material[i].isLayered = false;
			object->material[i].texLinked = false;
		}
		break;
	case DEFORMER:
		rawFile >> object->numDeform;
		object->deformer = new Deformer[object->numDeform];
		for (int i = 0; i < object->numDeform; i++) {
			object->deformer[i].transform = glm::mat4(1.0f);
			object->deformer[i].transformLink = glm::mat4(1.0f);
		}
		break;
	case TEXTURE:
		rawFile >> object->numTex;
		object->texture = new Texture[object->numTex];
		break;
	case FRAMEDATA:
		rawFile >> object->numFrmData;
		object->frameData = new FrameData[object->numFrmData];
		for (int i = 0; i < object->numFrmData; i++) {
			object->frameData[i].currentKey = 0;
		}
		break;
	case ANIMATIONNODE:
		rawFile >> object->numAnimNode;
		object->animNode = new AnimNode[object->numAnimNode];
		for (int i = 0; i < object->numAnimNode; i++) {
			object->animNode[i].defaultTransform[0] = object->animNode[i].defaultTransform[1] = object->animNode[i].defaultTransform[2] = 0;
			for (int j = 0; j < 3; j++) {
				object->animNode[i].frameData[j] = nullptr;
			}
		}
	default:
		break;
	}
	return 0;
}

float Parse_FBX::RawToFloat() {
	while (1) {
		buffer_s[0] = rawFile.get();
		if (buffer_s[0] != 32 || buffer_s[0] != ',') {
			break;
		}
	}
	for (int i = 1;; i++) {
		buffer_s[i] = rawFile.get();
		if (buffer_s[i] == ',') {
			buffer_s[i] = '\0';
			break;
		}
		else if (buffer_s[i] == '\n') {
			buffer_s[i] = '\0';
			endFlag = true;
			break;
		}
	}
	sscanf_s(buffer_s, "%f", &buffer_f);

	return buffer_f;
}

int Parse_FBX::RawToIndex() {
	while (1) {
		buffer_s[0] = rawFile.get();
		if (buffer_s[0] != 32 || buffer_s[0] != ',') {
			break;
		}
	}
	for (int i = 1;; i++) {
		buffer_s[i] = rawFile.get();
		if (buffer_s[i] == ',') {
			buffer_s[i] = '\0';
			break;
		}
		else if (buffer_s[i] == '\n') {
			buffer_s[i] = '\0';
			endFlag = true;
			break;
		}
	}
	sscanf_s(buffer_s, "%d", &buffer_i);

	return buffer_i;
}

void Parse_FBX::PreCount() {
	while (1) {
		buffer_s[0] = rawFile.get();
		if (buffer_s[0] != 32 || buffer_s[0] != ',') {
			break;
		}
	}
	for (int i = 1;; i++) {
		buffer_s[i] = rawFile.get();
		if (buffer_s[i] == ',') {
			buffer_s[i] = '\0';
			break;
		}
		else if (buffer_s[i] == '\n') {
			buffer_s[i] = '\0';
			endFlag = true;
			break;
		}
	}
}

Object* Parse_FBX::GetObjectPtr() {
	if (object != nullptr) return object;
	else {
		cout << "FBX NOT PREPARED" << endl;
		return nullptr;
	}
}

void Parse_FBX::Trianglize(float *) {

}

int Parse_FBX::SeekUntilExt(char c) {
	char temp;
	while (1) {
		temp = rawFile.get();
		if (temp == c) {
			return 0;
		}
		else if (temp == EOF) {
			return -1;
		}
	}
}

int Parse_FBX::SeekUntil(char c) {
	while (rawFile.get() != c) {
	}
	return 0;
}

int Parse_FBX::RawToInt() {
	char buffer[100];
	int answer;
	do {
		buffer[0] = rawFile.get();
	} while ((buffer[0] < 48) || (buffer[0] > 57));

	for (int i = 1;; i++) {
		buffer[i] = rawFile.get();
		if ((buffer[i] < 48) || (buffer[i] > 57)) {
			buffer[i] = '\0';
			break;
		}
	}
	sscanf_s(buffer, "%d", &answer);
	//cout << buffer << endl;
	//cout << answer << endl;
	//Sleep(500);
	return answer;
}

long long int Parse_FBX::RawToLLInt() {
	char buffer[100];
	long long int answer;
	for (int i = 0;; i++) {
		buffer[i] = rawFile.get();
		if (buffer[i] == ' ') continue;
		//cout << buffer[i];
		if ((buffer[i] < 48) || (buffer[i] > 57)) {
			buffer[i] = '\0';
			//for (int j = 0; j < i; j++) {
			//	//cout << (char)buffer[j];
			//	if (buffer[j] != '0') {
			//		break;
			//	}
			//	else return 0;
			//}
			break;
		}
	}
	sscanf_s(buffer, "%lld", &answer);
	return answer;
}

void Parse_FBX::ParseVersion_1(const char *path) {
	
	object = new Object;

	rawFile.open(path, ios::binary);

	while (1) {
		if (InitObjectNumbers() == -1) break;
	}

	// Model ÀÔ·ÂºÎºÐ
	while (strcmp(buffer_s, "Model:") != 0) {
		rawFile >> buffer_s;
	}
	rawFile >> buffer_s;
	
	object->model->name = new char[strlen(buffer_s)];
	for (int i = 0; i < strlen(buffer_s) + 1; i++) {
		object->model->name[i] = buffer_s[i];
	}

	// Model-Vertex ÀÔ·Â
	while (strcmp(buffer_s, "Vertices:") != 0) {
		rawFile >> buffer_s;
	}
	int current = rawFile.tellg();

	for (int i = 0;; i++) {
		PreCount();
		if (endFlag == true) {
			object->model->vertex = new float[i + 1];
			break;
		}
	}
	endFlag = false;
	rawFile.seekg(current, ios::beg);

	object->model->numVertex = 0;
	for (int i = 0; endFlag == false; i++) {
		object->model->vertex[i] = RawToFloat();
		object->model->numVertex++;
		//printf("Vertex[%d]: %f\n", i, object->model->vertex[i]);
	}
	endFlag = false;

	// Model-PolygonVertexIndex ÀÔ·Â
	while (strcmp(buffer_s, "PolygonVertexIndex:") != 0) {
		rawFile >> buffer_s;
	}
	current = rawFile.tellg();
	for (int i = 0;; i++) {
		PreCount();
		if (endFlag == true) {
			object->model->index = new int[i + 1];
			break;
		}
	}
	endFlag = false;
	rawFile.seekg(current);

	object->model->numIndex = 0;
	for (int i = 0; endFlag == false; i++) {
		object->model->index[i] = RawToIndex();
		object->model->numIndex++;
		if (object->model->index[i] < 0) object->model->index[i] = -object->model->index[i] - 1;
		//printf("%i\n", object->model->index[i]);
	}
	endFlag = false;

	// File close
	rawFile.close();
}

float Parse_FBX::Normalize(float x, float y, float z) {
	return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
}

void Parse_FBX::ParseVersion_2(const char *path) {

	object = new Object;
	object->numModel = object->numGeom = object->numMat = object->numTex = 0;

	rawFile.open(path, ios::binary);

	while (1) {
		if (InitObjectNumbers() == -1) break;
	}
	
	while (1) {
		rawFile >> buffer_s;

		// Geometry parameter ÆÄ½Ì
		if (strcmp(buffer_s, "Geometry:") == 0) {
			currentGeom = &object->geometry[indGeom];
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &currentGeom->geomID);
			while (1) {
				rawFile >> buffer_s;
				if (strcmp(buffer_s, "Vertices:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &currentGeom->numVertex);
					currentGeom->vertex = new float[currentGeom->numVertex];
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "a:") == 0) {
							break;
						}
					}
					for (int i = 0; i < currentGeom->numVertex; i++) {
						currentGeom->vertex[i] = RawToFloat();
						//printf("%.15f\n", currentGeom->vertex[i]);
					}
				}
				else if (strcmp(buffer_s, "PolygonVertexIndex:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &currentGeom->numVertIndex);
					currentGeom->vertIndex = new int[currentGeom->numVertIndex];
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "a:") == 0) {
							break;
						}
					}
					for (int i = 0; i < currentGeom->numVertIndex; i++) {
						currentGeom->vertIndex[i] = RawToIndex();
						if (currentGeom->vertIndex[i] < 0) currentGeom->vertIndex[i] = -currentGeom->vertIndex[i] - 1;
						//printf("%d\n", currentGeom->index[i]);
					}
					object->geometry[indGeom].vertWeight = new float*[3];
					object->geometry[indGeom].weightMatrixIndex = new int*[3];
					for (int j = 0; j < 3; j++) {
						object->geometry[indGeom].vertWeight[j] = new float[object->geometry[indGeom].numVertIndex]();
						object->geometry[indGeom].weightMatrixIndex[j] = new int[object->geometry[indGeom].numVertIndex]();
					}
				}
				// Skip Edges component
				else if (strcmp(buffer_s, "Edges:") == 0) {
					SeekUntil('}');
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "MappingInformationType:") == 0) {
							rawFile >> buffer_s;
							if (strcmp(buffer_s, "\"ByVertice\"") == 0) {
								currentGeom->isDirect = true;
							}
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "Normals:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &currentGeom->numNormal);
					currentGeom->normal = new float[currentGeom->numNormal];
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "a:") == 0) {
							break;
						}
					}
					for (int i = 0; i < currentGeom->numNormal; i++) {
						currentGeom->normal[i] = RawToFloat();
						//printf("%.15f\n", currentGeom->normal[i]);
					}
				}
				// Skip NormalsW component
				else if (strcmp(buffer_s, "NormalsW:") == 0) {
					SeekUntil('}');
				}
				// Skip UV, UVIndex
				else if (strcmp(buffer_s, "UV:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &currentGeom->numUV);
					currentGeom->uv = new float[currentGeom->numUV];
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "a:") == 0) {
							break;
						}
					}
					for (int i = 0; i < currentGeom->numUV; i++) {
						currentGeom->uv[i] = RawToFloat();
						//printf("%.15f\n", currentGeom->normal[i]);
					}
					if (currentGeom->isDirect == true) {
						break;
					}
				}
				else if (strcmp(buffer_s, "UVIndex:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &currentGeom->numUVIndex);
					currentGeom->uvIndex = new int[currentGeom->numUVIndex];
					while (1) {
						rawFile >> buffer_s;
						if (strcmp(buffer_s, "a:") == 0) {
							break;
						}
					}
					for (int i = 0; i < currentGeom->numUVIndex; i++) {
						currentGeom->uvIndex[i] = RawToIndex();
						if (currentGeom->uvIndex[i] < 0) currentGeom->uvIndex[i] = -currentGeom->vertIndex[i] - 1;
					}
				}
				else if ((strcmp(buffer_s, "Geometry:") == 0) || (strcmp(buffer_s, "Model:") == 0)) {
					rawFile.seekg(-12, ios::cur);
					break;
				}
			}
			//printf("numUV: %d, numVertIndex: %d\n", currentGeom->numUV, currentGeom->numVertIndex);
			indGeom++;
		}

		// Model parameter ÆÄ½Ì
		else if (strcmp(buffer_s, "Model:") == 0) {
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &object->model[indModel].modelID);
			rawFile >> buffer_s;
			object->model[indModel].name = new char[strlen(buffer_s) - strlen("\"Model::\",") + 1];
			for (int i = 0; i < strlen(buffer_s) - strlen("\"Model::\","); i++) {
				object->model[indModel].name[i] = buffer_s[i + strlen("\"Model::")];
				object->model[indModel].name[i + 1] = '\0';
			}
			rawFile >> buffer_s;
			object->model[indModel].type = new char[strlen(buffer_s) - 1];
			for (int i = 0; i < strlen(buffer_s) - 1; i++) {
				object->model[indModel].type[i] = buffer_s[i + 1];
				if (buffer_s[i + 1] == '"') {
					object->model[indModel].type[i] = '\0';
				}
			}
			//cout << object->model[indModel].type << endl;

			while (1) {
				rawFile >> buffer_s;
				if (strcmp(buffer_s, "\"PreRotation\",") == 0) {
					SeekUntil(',');
					SeekUntil(',');
					SeekUntil(',');
					for (int i = 0; i < 3; i++) {
						object->model[indModel].preRot[i] = RawToFloat();
					}
				}
				else if (strcmp(buffer_s, "Translation\",") == 0) {
					SeekUntil('A');
					SeekUntil(',');
					for (int i = 0; i < 3; i++) {
						object->model[indModel].lclTrans[i] = RawToFloat();
					}
				}
				else if (strcmp(buffer_s, "Rotation\",") == 0) {
					SeekUntil('A');
					SeekUntil(',');
					for (int i = 0; i < 3; i++) {
						object->model[indModel].lclRot[i] = RawToFloat();
					}
				}
				else if (strcmp(buffer_s, "Scaling\",") == 0) {
					SeekUntil('A');
					SeekUntil(',');
					for (int i = 0; i < 3; i++) {
						object->model[indModel].lclScale[i] = RawToFloat();
					}
				}
				else if (strcmp(buffer_s, "}") == 0) {
					break;
				}
			}
			//cout << object->model[indModel].modelNum << " : " << object->model[indModel].name << endl;
			object->model[indModel].modelNum = indModel;
			indModel++;
		}
		// Material parameter ÆÄ½Ì
		else if (strcmp(buffer_s, "Material:") == 0) {
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &object->material[indMat].matID);
			while (1) {
				if (rawFile.get() == ':') break;
			}
			rawFile.get();
			object->material[indMat].name = new char[100];
			for(int i = 0;;i++) {
				object->material[indMat].name[i] = rawFile.get();
				if (object->material[indMat].name[i] == '\"') {
					object->material[indMat].name[i] = '\0';
					break;
				}
			}
			//cout << object->material[indMat].matNum << " : " << object->material[indMat].name << endl;
			while (1) {
				rawFile >> buffer_s;
				if (strcmp(buffer_s, "\"AmbientColor\",") == 0) {
					SeekUntil(',');
					SeekUntil(',');
					SeekUntil(',');
					object->material[indMat].color_ambient[0] = RawToFloat();
					object->material[indMat].color_ambient[1] = RawToFloat();
					object->material[indMat].color_ambient[2] = RawToFloat();
					object->material[indMat].color_ambient[3] = 1.0f;
				}
				else if (strcmp(buffer_s, "\"DiffuseColor\",") == 0) {
					SeekUntil(',');
					SeekUntil(',');
					SeekUntil(',');
					object->material[indMat].color_diffuse[0] = RawToFloat();
					object->material[indMat].color_diffuse[1] = RawToFloat();
					object->material[indMat].color_diffuse[2] = RawToFloat();
					object->material[indMat].color_diffuse[3] = 1.0f;
				}
				else if (strcmp(buffer_s, "\"SpecularColor\",") == 0) {
					SeekUntil(',');
					SeekUntil(',');
					SeekUntil(',');
					object->material[indMat].color_specular[0] = RawToFloat();
					object->material[indMat].color_specular[1] = RawToFloat();
					object->material[indMat].color_specular[2] = RawToFloat();
					object->material[indMat].color_specular[3] = 1.0f;
					break;
				}
			}
			indMat++;
		}
		else if (strcmp(buffer_s, "Deformer:") == 0) {
			rawFile >> buffer_s;
			currentDeform = &object->deformer[indDeform];
			sscanf_s(buffer_s, "%d,", &currentDeform->deformerID);
			SeekUntil(',');
			rawFile >> buffer_s;
			if (strcmp(buffer_s, "\"Cluster\"") == 0) {
				currentDeform->isSubDeformer = true;
				SeekUntil('*');
				rawFile >> buffer_s;
				sscanf_s(buffer_s, "%d", &currentDeform->numWeight);
				currentDeform->weightIndex = new int[currentDeform->numWeight];
				currentDeform->weight = new float[currentDeform->numWeight];
				SeekUntil(':');
				rawFile.get();
				for (int i = 0; i < currentDeform->numWeight; i++) {
					currentDeform->weightIndex[i] = RawToInt();
					/*if (currentDeform->deformerID == 1681601392) {
						cout << currentDeform->weightIndex[i] << endl;
						cout << currentDeform->weight[i] << endl;
					}*/
				}
				SeekUntil(':');
				SeekUntil(':');
				rawFile.get();
				for (int i = 0; i < currentDeform->numWeight; i++) {
					currentDeform->weight[i] = RawToFloat();
				}
				SeekUntil(':');
				SeekUntil(':');
				rawFile.get();
				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 4; j++) {
						currentDeform->transform[i][j] = RawToFloat();
					}
				}
				SeekUntil(':');
				SeekUntil(':');
				rawFile.get();
				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 4; j++) {
						currentDeform->transformLink[i][j] = RawToFloat();
					}
				}
			}
			else {
				currentDeform->isSubDeformer = false;
			}
			currentDeform->numSubDeformer = 0;
			indDeform++;
		}
		// Texture parameter ÆÄ½Ì
		else if (strcmp(buffer_s, "Video:") == 0) {
			SeekUntil('}');
			SeekUntil('}');
		}
		else if (strcmp(buffer_s, "Texture:") == 0) {
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &object->texture[indTex].texID);
			while (1) {
				if (rawFile.get() == ':') break;
			}
			rawFile.get();
			object->texture[indTex].name = new char[100];
			for (int i = 0;; i++) {
				object->texture[indTex].name[i] = rawFile.get();
				if (object->texture[indTex].name[i] == '\"') {
					object->texture[indTex].name[i] = '\0';
					break;
				}
			}
			while (1) {
				rawFile >> buffer_s;
				if (strcmp(buffer_s, "FileName:") == 0) {
					SeekUntil('\"');
					for (int i = 0;; i++) {
						object->texture[indTex].sourcePath[i] = rawFile.get();
						if (object->texture[indTex].sourcePath[i] == '\"') {
							object->texture[indTex].sourcePath[i] = '\0';
							//cout << object->texture[indTex].sourcePath << endl;
							break;
						}
					}
					break;
				}
			}
			//cout << object->texture[indTex].texNum << " : " << object->texture[indTex].name << endl;
			indTex++;
		}
		// Animation Node parameter ÆÄ½Ì
		else if (strcmp(buffer_s, "AnimationCurveNode:") == 0) {
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &object->animNode[indAnimNode].nodeID);
			SeekUntil(':');
			SeekUntil(':');
			object->animNode[indAnimNode].type = rawFile.get();
			
			SeekUntil('{');
			for (int i = 0; i < 3; i++) {
				SeekUntil('A');
				SeekUntil(',');
				object->animNode[indAnimNode].defaultTransform[i] = RawToFloat();
				//printf("::%f\n", object->animNode[indAnimNode].defaultTransform[i]);
			}
			indAnimNode++;
		}
		// Key Frame Data ÆÄ½Ì
		else if (strcmp(buffer_s, "AnimationCurve:") == 0) {
			rawFile >> buffer_s;
			sscanf_s(buffer_s, "%d,", &object->frameData[indFrmData].frameListID);
			//cout << "ID: " << object->frameData[indFrmData].frameListID << endl;
			while (1) {
				rawFile >> buffer_s;
				if (strcmp(buffer_s, "KeyTime:") == 0) {
					rawFile >> buffer_s;
					sscanf_s(buffer_s, "*%d", &object->frameData[indFrmData].numKeyTime);
					object->frameData[indFrmData].keyTime = new long long int[object->frameData[indFrmData].numKeyTime];
					SeekUntil(':');
					for (int i = 0; i < object->frameData[indFrmData].numKeyTime; i++) {
						object->frameData[indFrmData].keyTime[i] = RawToLLInt();
						//cout << "::"<<(long long int)object->frameData[indFrmData].keyTime[i] << endl;
					}
				}
				else if (strcmp(buffer_s, "KeyValueFloat:") == 0) {
					rawFile >> buffer_s;
					object->frameData[indFrmData].transformValue = new float[object->frameData[indFrmData].numKeyTime];
					SeekUntil(':');
					for (int i = 0; i < object->frameData[indFrmData].numKeyTime; i++) {
						object->frameData[indFrmData].transformValue[i] = RawToFloat();
						//cout << object->frameData[indFrmData].transformValue[i] << endl;
					}
					break;
				}
			}
			indFrmData++;
		}
		// Parameters Link
		else if (strcmp(buffer_s, "Connections:") == 0) {
			while (1) {
				if (SeekUntilExt(';') == -1) {
					break;
				}
				
				for (int i = 0;; i++) {
					buffer_s[i] = rawFile.get();
					if (buffer_s[i] == ':') {
						buffer_s[i] = '\0';
						rawFile.get();
						break;
					}
				}
				if (strcmp(buffer_s, "Model") == 0) {
					rawFile >> buffer_s;
					rawFile.get();
					for (int i = 0;; i++) {
						buffer_s[i] = rawFile.get();
						if (buffer_s[i] == ':') {
							buffer_s[i] = '\0';
							rawFile.get();
							break;
						}
					}
					if (strcmp(buffer_s, "Model") == 0) {
						SeekUntil(',');
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numModel; i++) {
							if (buffer_i == object->model[i].modelID) {
								currentModel = &object->model[i];
								break;
							}
						}
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numModel; i++) {
							if (buffer_i == object->model[i].modelID) {
								currentModel->parent = &object->model[i];
								object->model[i].child = currentModel;
								//cout << currentModel->modelID << "  " << currentModel->parent->modelID << endl;
								break;
							}
						}
					}
					else if (strcmp(buffer_s, "SubDeformer") == 0) {
						SeekUntil(',');
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numModel; i++) {
							if (buffer_i == object->model[i].modelID) {
								currentModel = &object->model[i];
								break;
							}
						}
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numDeform; i++) {
							if (buffer_i == object->deformer[i].deformerID) {
								object->deformer[i].model = currentModel;
								currentModel->TransformLinkMatrix = object->deformer[i].transformLink;
								currentModel->TransformMatrix = object->deformer[i].transform;
								//cout << object->deformer[i].model->modelID << "  " << object->deformer[i].deformerID << endl;
								break;
							}
						}
						
					}
				}
				else if (strcmp(buffer_s, "AnimCurveNode") == 0) {
					rawFile >> buffer_s;
					rawFile.get();
					for (int i = 0;; i++) {
						buffer_s[i] = rawFile.get();
						if (buffer_s[i] == ':') {
							buffer_s[i] = '\0';
							rawFile.get();
							break;
						}
					}
					if (strcmp(buffer_s, "Model") == 0) {
						SeekUntil(',');
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numAnimNode; i++) {
							if (buffer_i == object->animNode[i].nodeID) {
								currentAnimNode = &object->animNode[i];
								break;
							}
						}
						buffer_i = (unsigned int)RawToInt();
						for (int i = 0; i < object->numModel; i++) {
							if (buffer_i == object->model[i].modelID) {
								switch (currentAnimNode->type) {
									case 'R':
										object->model[i].animNodeR = currentAnimNode;
										break;
									case 'S':
										object->model[i].animNodeS = currentAnimNode;
										break;
									case 'T':
										object->model[i].animNodeT = currentAnimNode;
										break;
									default:
										break;
								}
								break;
							}
						}
					}
				}
				else if (strcmp(buffer_s, "Deformer") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numDeform; i++) {
						if (buffer_i == object->deformer[i].deformerID) {
							currentDeform = &object->deformer[i];
							break;
						}
					}
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numGeom; i++) {
						if (buffer_i == object->geometry[i].geomID) {
							object->geometry[i].deformer = currentDeform;
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "SubDeformer") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numDeform; i++) {
						if (buffer_i == object->deformer[i].deformerID) {
							currentDeform = &object->deformer[i];
							break;
						}
					}
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numDeform; i++) {
						if (buffer_i == object->deformer[i].deformerID) {
							object->deformer[i].subDeformer.push_back(currentDeform);
							object->deformer[i].numSubDeformer++;
							//cout << currentDeform->deformerID << "  " << object->deformer[i].deformerID << "  " << object->deformer[i].numSubDeformer << endl;
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "Geometry") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numGeom; i++) {
						if (buffer_i == object->geometry[i].geomID) {
							//cout << "geometry found : " << (unsigned int)buffer_i << ", " << object->geometry[i].geomNum << endl;
							currentGeom = &object->geometry[i];
							break;
						}
					}
					buffer_i = (unsigned int)RawToInt();
					for (int i = 0; i < object->numModel; i++) {
						if (buffer_i == object->model[i].modelID) {
							currentGeom->model = &object->model[i];
							//cout << "Model found : " << currentGeom->model->modelNum << endl;
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "Material") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					unsigned int buffer_temp = (unsigned int)RawToInt();
					for (int i = 0; i < object->numModel; i++) {
						if (buffer_temp == object->model[i].modelID) {
							for (int j = 0; j < object->numMat; j++) {
								if (buffer_i == object->material[j].matID) {
									object->model[i].material = &object->material[j];
									break;
								}
							}
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "LayeredTexture") == 0) {
					SeekUntil('\"');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					unsigned int buffer_temp = (unsigned int)RawToInt();
					for (int i = 0; i < object->numMat; i++) {
						if (buffer_temp == object->material[i].matID) {
							object->material[i].lTexID = buffer_i;
							object->material[i].isLayered = true;
							object->material[i].texLinked = true;
							//cout << "LT, MAT: "<< object->material[i].lTexNum << ", " << object->material[i].matNum << endl;
							break;
						}
					}
				}
				else if (strcmp(buffer_s, "Texture") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = (unsigned int)RawToInt();
					unsigned int buffer_temp = (unsigned int)RawToInt();
					
					int flag_temp = false;

					for (int i = 0; i < object->numMat; i++) {
						if (buffer_temp == object->material[i].lTexID) {
							for (int j = 0; j < object->numTex; j++) {
								if (buffer_i == object->texture[j].texID) {
									object->material[i].texture = &object->texture[j];
									object->material[i].texLinked = true;
									//cout << "TEX, LT: " << object->material[i].texture->texNum << ", " << object->material[i].lTexNum << endl;
									flag_temp = true;
									break;
								}
							}
							break;
						}
					}

					for (int i = 0; i < object->numMat; i++) {
						if (flag_temp == true) {
							break;
						}
						if (buffer_temp == object->material[i].matID) {
							for (int j = 0; j < object->numTex; j++) {
								if (buffer_i == object->texture[j].texID) {
									object->material[i].texture = &object->texture[j];
									object->material[i].texLinked = true;
									break;
								}
							}
							break;
						}
					}
					//cout << object->geometry[0].model->material->texture->texNum << endl;
				}
				else if (strcmp(buffer_s, "AnimCurve") == 0) {
					SeekUntil('C');
					SeekUntil(',');
					buffer_i = RawToInt();
					for (int i = 0; i < object->numFrmData; i++) {
						if (buffer_i == object->frameData[i].frameListID) {
							currentFrmData = &object->frameData[i];
							break;
						}
					}
					buffer_i = RawToInt();
					for (int i = 0; i < object->numAnimNode; i++) {
						if (buffer_i == object->animNode[i].nodeID) {
							SeekUntil('|');
							buffer_c = rawFile.get();
							if (buffer_c == 'X') {
								object->animNode[i].frameData[0] = currentFrmData;
							}
							else if (buffer_c == 'Y') {
								object->animNode[i].frameData[1] = currentFrmData;
							}
							else if (buffer_c == 'Z') {
								object->animNode[i].frameData[2] = currentFrmData;
							}
							/*cout << object->animNode[i].nodeNum << endl;
							cout << currentFrmData->frameListID << endl;*/
							break;
						}
					}
				}
			}
		}
		if (rawFile.eof()) {
			/*for (int i = 0; i < indGeom; i++) {
				cout << object->geometry[i].geomNum << ", ";
				cout << object->geometry[i].model->modelNum << ", ";
				cout << object->geometry[i].model->material->matNum << ", ";
				cout << object->geometry[i].model->material->lTexNum << ", ";
				cout << object->geometry[i].model->material->texture->texNum << endl;
			}*/
			//cout << object->geometry[indGeom-1].model->material->texture->texNum << endl;
			
			break;
		}
	}
	// weight Á¤º¸ geometry¿¡ º¹»ç
	for (int i = 0; i < object->numGeom; i++) {
		currentGeom = &object->geometry[i];
		if (currentGeom->deformer == nullptr) continue;
		std::list<Deformer*>::iterator iter = currentGeom->deformer->subDeformer.begin();
		currentDeform = currentGeom->deformer;
		while (currentDeform->subDeformer.end() != iter) {
			for (int j = 0; j < (*iter)->numWeight; j++) {
				/*cout << (*iter)->deformerID << endl;
				cout << (*iter)->numWeight << endl;
				cout << (*iter)->weight[j] << endl;
				cout << ((*iter)->weightIndex)[j] << endl;*/
				if ((*iter)->weight[j] > currentGeom->vertWeight[0][(*iter)->weightIndex[j]]) {
					currentGeom->vertWeight[2][(*iter)->weightIndex[j]] = currentGeom->vertWeight[1][(*iter)->weightIndex[j]];
					currentGeom->vertWeight[1][(*iter)->weightIndex[j]] = currentGeom->vertWeight[0][(*iter)->weightIndex[j]];
					currentGeom->vertWeight[0][(*iter)->weightIndex[j]] = (*iter)->weight[j];
					currentGeom->weightMatrixIndex[2][(*iter)->weightIndex[j]] = currentGeom->weightMatrixIndex[1][(*iter)->weightIndex[j]];
					currentGeom->weightMatrixIndex[1][(*iter)->weightIndex[j]] = currentGeom->weightMatrixIndex[0][(*iter)->weightIndex[j]];
					currentGeom->weightMatrixIndex[0][(*iter)->weightIndex[j]] = (*iter)->model->modelNum;
					//cout << (*iter)->model->modelID << "  " << object->model[(*iter)->model->modelNum].modelID << endl;
				}
				else if ((*iter)->weight[j] > currentGeom->vertWeight[1][(*iter)->weightIndex[j]]) {
					currentGeom->vertWeight[2][(*iter)->weightIndex[j]] = currentGeom->vertWeight[1][(*iter)->weightIndex[j]];
					currentGeom->vertWeight[1][(*iter)->weightIndex[j]] = (*iter)->weight[j];
					currentGeom->weightMatrixIndex[2][(*iter)->weightIndex[j]] = currentGeom->weightMatrixIndex[1][(*iter)->weightIndex[j]];
					currentGeom->weightMatrixIndex[1][(*iter)->weightIndex[j]] = (*iter)->model->modelNum;
				}
				else if ((*iter)->weight[j] > currentGeom->vertWeight[2][(*iter)->weightIndex[j]]) {
					currentGeom->vertWeight[2][(*iter)->weightIndex[j]] = (*iter)->weight[j];
					currentGeom->weightMatrixIndex[2][(*iter)->weightIndex[j]] = (*iter)->model->modelNum;
				}
			}
			iter++;
		}
	}

	for (int i = 0; i < object->numGeom; i++) {
		currentGeom = &object->geometry[i];
		// Change vertex mode: index to direct
		if (currentGeom->isDirect == true) {
			currentGeom->vertDirect = new float[currentGeom->numVertIndex * 3]();
			currentGeom->vertWeightDirect = new float*[3];
			currentGeom->vertWeightDirect[0] = new float[currentGeom->numVertIndex]();
			currentGeom->vertWeightDirect[1] = new float[currentGeom->numVertIndex]();
			currentGeom->vertWeightDirect[2] = new float[currentGeom->numVertIndex]();
			currentGeom->weightMatrixIndexDirect = new int*[3];
			currentGeom->weightMatrixIndexDirect[0] = new int[currentGeom->numVertIndex]();
			currentGeom->weightMatrixIndexDirect[1] = new int[currentGeom->numVertIndex]();
			currentGeom->weightMatrixIndexDirect[2] = new int[currentGeom->numVertIndex]();
			for (int i = 0, j = 0; i < currentGeom->numVertIndex; i++, j += 3) {
				currentGeom->vertDirect[j] = currentGeom->vertex[3 * currentGeom->vertIndex[i]];
				currentGeom->vertDirect[j + 1] = currentGeom->vertex[3 * currentGeom->vertIndex[i] + 1];
				currentGeom->vertDirect[j + 2] = currentGeom->vertex[3 * currentGeom->vertIndex[i] + 2];
				currentGeom->vertWeightDirect[0][i] = currentGeom->vertWeight[0][currentGeom->vertIndex[i]];
				currentGeom->vertWeightDirect[1][i] = currentGeom->vertWeight[1][currentGeom->vertIndex[i]];
				currentGeom->vertWeightDirect[2][i] = currentGeom->vertWeight[2][currentGeom->vertIndex[i]];
				currentGeom->weightMatrixIndexDirect[0][i] = currentGeom->weightMatrixIndex[0][currentGeom->vertIndex[i]];
				currentGeom->weightMatrixIndexDirect[1][i] = currentGeom->weightMatrixIndex[1][currentGeom->vertIndex[i]];
				currentGeom->weightMatrixIndexDirect[2][i] = currentGeom->weightMatrixIndex[2][currentGeom->vertIndex[i]];
			}
		}
		// Change normal mode: float to index, single normal per vertex
		if (currentGeom->isDirect == false) {
			float *normal_temp = new float[currentGeom->numNormal]();
			for (int i = 0, j = 0; i < currentGeom->numVertIndex; i++, j += 3) {
				normal_temp[3 * currentGeom->vertIndex[i]] += currentGeom->normal[j];
				normal_temp[3 * currentGeom->vertIndex[i] + 1] += currentGeom->normal[j + 1];
				normal_temp[3 * currentGeom->vertIndex[i] + 2] += currentGeom->normal[j + 2];
			}
			for (int i = 0; i < currentGeom->numNormal; i += 3) {
				buffer_f = Normalize(normal_temp[i], normal_temp[i + 1], normal_temp[i + 2]);
				currentGeom->normal[i] = normal_temp[i] / buffer_f;
				currentGeom->normal[i + 1] = normal_temp[i + 1] / buffer_f;
				currentGeom->normal[i + 2] = normal_temp[i + 2] / buffer_f;
			}
		}
		// Change normal mode: index to direct
		if (currentGeom->isDirect == true) {
			currentGeom->normDirect = new float[currentGeom->numVertIndex * 3]();
			for (int i = 0, j = 0; i < currentGeom->numVertIndex; i++, j += 3) {
				currentGeom->normDirect[j] = currentGeom->normal[3 * currentGeom->vertIndex[i]];
				currentGeom->normDirect[j + 1] = currentGeom->normal[3 * currentGeom->vertIndex[i] + 1];
				currentGeom->normDirect[j + 2] = currentGeom->normal[3 * currentGeom->vertIndex[i] + 2];
			}
		}
		// Change UV mode: index to direct
		if ((currentGeom->isDirect == true) && (currentGeom->numUV > 0)) {
			currentGeom->uvDirect = new float[currentGeom->numVertIndex * 3]();
			for (int i = 0, j = 0; i < currentGeom->numVertIndex; i++, j += 2) {
				currentGeom->uvDirect[j] = currentGeom->uv[2 * currentGeom->vertIndex[i]];
				currentGeom->uvDirect[j + 1] = currentGeom->uv[2 * currentGeom->vertIndex[i] + 1];
			}
		}
	}

	/*for (int i = 0; i < object->numGeom; i++) {
		for (int j = 0; j < object->geometry[i].numVertIndex; j++) {
			cout << j << ": ";
			cout << object->geometry[i].vertWeightDirect[0][j]<<"  ";
			cout << object->geometry[i].vertWeightDirect[1][j] << "  ";

			cout << object->geometry[i].vertWeightDirect[2][j] << endl;

		}
	}*/

	object->BoneMatrix = new glm::mat4[object->numModel];
	object->GlobalMatrix = new glm::mat4[object->numModel];
	for (int i = 0; i < object->numModel; i++) {
		object->GlobalMatrix[i] = glm::mat4(1.0f);
		object->BoneMatrix[i] = object->model[i].TransformLinkMatrix;
	}

	// File close
	rawFile.close();
}

void Parse_FBX::Parse(const char *path) {
	switch (version) {
		case -1:
			cout << "INSERT VERSION" << endl;
			break;
		case 1:
			cout << "FBX parser V1 was called." << endl;
			ParseVersion_1(path);
			break;
		case 2:
			cout << "FBX parser V2 was called." << endl;
			ParseVersion_2(path);
			break;
		default:
			break;
	}
}