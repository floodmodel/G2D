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

typedef struct _ascRasterExtent
{
	double bottom=0.0;
	double top = 0.0;
	double left = 0.0;
	double right = 0.0;
	double extentWidth = 0.0;
	double extentHeight = 0.0;
} ascRasterExtent;

typedef struct _ascRasterHeader
{
	int nCols=0;
	int nRows = 0;
	double xllcorner = 0.0;
	double yllcorner = 0.0;
	float dx = 0.0;
	float dy = 0.0;
	float cellsize = 0.0;
	int nodataValue = 0;
	int headerEndingLineIndex = 0;
	int dataStartingLineIndex = 0;
} ascRasterHeader;

typedef struct _cellPosition
{
	int x = 0;
	int y = 0;
} cellPosition;

//typedef struct dateTime
//{
//	int year;
//	int month;
//	int day;
//	int hours;
//	int minutes;
//	int seconds;
//};
typedef struct _CPUsInfo
{
	string infoString = "";
	int numberOfCPUs = 0;
	int totalNumberOfLogicalProcessors = 0;
} CPUsInfo;

typedef struct _version
{
	WORD major=NULL	;
	WORD minor = NULL;
	WORD build = NULL;
	char LastWrittenTime[30] = { ""};
} version;

enum class flowDirection4
{
	//N=1, E = 4, S = 16, W=64, NONE=0
	E4 = 1, S4 = 3, W4 = 5, N4 = 7, NONE4 = 0
};

enum class flowDirection8
{
	//N=1, NE=2, E=4, SE=8, S=16,  SW=32, W=64, NW=128, NONE=0
	E8 = 1, SE8 = 2, S8 = 3, SW8 = 4, W8 = 5, NW8 = 6, N8 = 7, NE8 = 8, NONE8 = 0
};

enum class rainfallDataType
{
	NoneRF, //0
	TextFileMAP, //1
	TextFileASCgrid //2
};

// 1:Discharge, 2:Depth, 3:Height, 4:None
enum class conditionDataType
{
	NoneCD,   //0
	Discharge, // 1
	Depth,      //2
	Height     //3
} ;

enum class fileOrConstant
{
	File,
	Constant,
	None
};

enum class rendererType
{
	Depth,
	Risk
};

void appendTextToTextFile(string fpn, string textToAppend);
int confirmDeleteFile(string filePathNames);
int confirmDeleteFiles(vector<string> filePathNames);

// formatted numeric string
string forString(double value, int precision); 

void g2dHelp();

//ascRasterExtent getAscRasterExtent(ascRasterHeader header);
//ascRasterHeader getAscRasterHeader(string fpn_ascRasterFile);
//ascRasterHeader getAscRasterHeader(string LinesForHeader[], string separator[]);
CPUsInfo getCPUinfo();
version getCurrentFileVersion();
string getGPUinfo();
string getValueStringFromXmlLine(string aLine, string fieldName);
//char* getPath(char *fpn);

bool isNumericDbl(string instr);
bool isNumericInt(string instr);

void makeASCTextFile(string fpn, string allHeader, double** array2D,
	int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue);
void makeBMPFileUsingArrayGTzero_InParallel(string imgFPNtoMake,
	double** array2D,
	int colxNum, int rowyNum, rendererType rt,
    double rendererMaxV = 0, 	double nodataV = -9999);

int openProjectFile();
int openPrjAndSetupModel();

vector<double> readTextFileToDoubleVector(string fpn);
vector<float> readTextFileToFloatVector(string fpn);
vector<string> readTextFileToStringVector(string fpn);
map <int, vector<string>> readVatFile(string vatFPN, char seperator);
string replaceText(string inText, string textToFind, string textToRepalce);
tm secToHHMMSS(long sec);
tm stringToDateTime(string yyyymmddHHMM);
tm stringToDateTime2(string yyyymmdd_HHcolonMM);

vector<double> splitToDoubleVector(string strToSplit, const char delimeter, bool removeEmptyEntry = true);
vector<float> splitToFloatVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry=true);
vector<double> splitToDoubleVector(string strToSplit, string delimeter, bool removeEmptyEntry = true);
vector<int> splitToIntVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);
vector<string> splitToStringVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);

char* stringToCharP(string c_charP);
string timeElaspedToString_yyyymmddHHMM(string startTime_yyyymmdd_HHcolonMM, int elaspedTimeSec);
char* timeToString__yyyymmdd_HHcolonMMcolonSS(struct tm* t, int includeSEC = -1);
string timeToString_yyyymmdd_HHclnMMclnSS(struct tm t, int includeSEC = -1);
string timeToString_yyyymmdd_HHclnMMclnSS(COleDateTime t, int includeSEC);
string toLower(string instring);
string toUpper(string instring);

bool writeLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);
bool writeNewLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);
void writeTwoDimData(string fpn, double** array2D, int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue);


class ascRasterFile
{

private:
	const int BigSizeThreshold = 200000000;//2억개 기준
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

