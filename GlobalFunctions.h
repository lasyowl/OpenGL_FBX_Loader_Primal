#pragma once
#include <Windows.h>
#include <time.h>
#include <iostream>

using namespace std;

class GlobalFunctions {
public:
	GlobalFunctions() : start(0), end(0) {}
	~GlobalFunctions() {};
	void Start();
	void End(char *msg);
	int End();
private:
	int start, end;
};

void GlobalFunctions::Start() {
	start = clock();
}

void GlobalFunctions::End(char* msg) {
	end = clock();
	cout << msg << ": ";
	cout << end - start << "ms elapsed" << endl;
	start = 0;
}

int GlobalFunctions::End() {
	end = clock();
	int temp = end - start;
	start = 0;
	return temp;
}