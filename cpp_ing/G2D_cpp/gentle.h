#pragma once
#include <iostream>
#include <stdio.h>
#include <map>
#include <list>
#include<fstream>
#include <filesystem>
#include<ATLComTime.h>
#include <windows.h>
//#include <string>

using namespace std;
namespace fs = std::filesystem;



typedef struct ascRasterExtent
{
	double bottom;
	double top;
	double left;
	double right;
	double extentWidth;
	double extentHeight;
};

typedef struct ascRasterHeader
{
	int nCols;
	int nRows;
	double xllcorner;
	double yllcorner;
	float dx;
	float dy;
	float cellsize;
	int nodataValue;
	int headerEndingLineIndex;
	int dataStartingLineIndex;
};

typedef struct cellPosition
{
	int x;
	int y;
};

//typedef struct dateTime
//{
//	int year;
//	int month;
//	int day;
//	int hours;
//	int minutes;
//	int seconds;
//};



typedef struct version
{
	WORD major;
	WORD minor;
	WORD build;
	char LastWrittenTime[30];
};


enum flowDirection4
{
	//N=1, E = 4, S = 16, W=64, NONE=0
	E4 = 1, S4 = 3, W4 = 5, N4 = 7, NONE4 = 0
};

enum flowDirection8
{
	//N=1, NE=2, E=4, SE=8, S=16,  SW=32, W=64, NW=128, NONE=0
	E8 = 1, SE8 = 2, S8 = 3, SW8 = 4, W8 = 5, NW8 = 6, N8 = 7, NE8 = 8, NONE8 = 0
};


enum rainfallDataType
{
	NoneRF, //0
	TextFileMAP, //1
	TextFileASCgrid //2
};

// 1:Discharge, 2:Depth, 3:Height, 4:None
enum conditionDataType
{
	Discharge, // 1
	Depth,      //2
	Height,    //3
	NoneCD     //4
};


enum fileOrConstant
{
	File,
	Constant,
	None
};


int confirmDeleteFile(string filePathNames);
int confirmDeleteFiles(vector<string> filePathNames);


bool isNumeric(string instr);
void g2dHelp();

//ascRasterExtent getAscRasterExtent(ascRasterHeader header);
//ascRasterHeader getAscRasterHeader(string fpn_ascRasterFile);
//ascRasterHeader getAscRasterHeader(string LinesForHeader[], string separator[]);
string getCPUinfo();
version getCurrentFileVersion();
string getGPUinfo();
string getValueStringFromXmlLine(string aLine, string fieldName);
//char* getPath(char *fpn);

int openProjectFile();
int openPrjSetupRunG2D();

vector<string> readTextFileToStringVector(string fpn);
map <int, vector<string>> readVatFile(string vatFPN, char seperator);
tm secToHHMMSS(long sec);
tm stringToDateTime(string yyyymmddHHMM);
tm stringToDateTime2(string yyyymmdd_HHcolonMM);

vector<double> splitToDoubleVector(string strToSplit, const char delimeter, bool removeEmptyEntry = true);
vector<double> splitToDoubleVector(string strToSplit, string delimeter, bool removeEmptyEntry = true);
vector<int> splitToIntVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);
vector<string> splitToStringVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);

char* stringToCharP(string c_charP);
char* timeToString(struct tm* t, int includeSEC = -1);
string timeToString(struct tm t, int includeSEC = -1);
string timeToString(COleDateTime t, int includeSEC);
string toLower(string instring);
string toUpper(string instring);

bool writeLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);
bool writeNewLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);


class ascRasterFile
{

private:

	const int BigSizeThreshold = 200000000;//2�ﰳ ����
	char separator = { ' ' };

public:
	bool disposed = false;
	// Instantiate a SafeHandle instance.
//SafeHandle handle = new SafeFileHandle(IntPtr.Zero, true);
	string linesForHeader[8];
	//double dataValueOri;
	ascRasterHeader header;
	string headerStringAll;
	double ** valuesFromTL;
	ascRasterExtent extent;

	ascRasterFile(string fpn_ascRasterFile);
	ascRasterHeader getAscRasterHeader(string LinesForHeader[], char separator);
	ascRasterExtent getAscRasterExtent(ascRasterHeader header);
	string makeHeaderString(int ncols, int nrows, double xll, double yll, float cellSize, float dx, float dy, int nodataValue);

	~ascRasterFile();
};


inline std::string trim(std::string& str)
{
	str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
	str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
	return str;
}

inline std::string trimL(std::string& str)
{
	str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
	return str;
}


inline std::string trimR(std::string& str)
{
	str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
	return str;
}
