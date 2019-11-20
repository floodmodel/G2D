#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <list>
#include <map>
#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;


const string CONST_FILENAME_TAG_DISCHARGE = "_Discharge";
const string CONST_FILENAME_TAG_DEPTH = "_Depth";
const string CONST_FILENAME_TAG_HEIGHT = "_Height";
const string CONST_FILENAME_TAG_VELOCITY = "_Velocity";
const string CONST_FILENAME_TAG_FLOWDIRECTION = "_FDirection";
const string CONST_FILENAME_TAG_RFGRID = "_RFGrid";
const string CONST_FILENAME_TAG_BCDATA = "_BC";
const string CONST_FILENAME_TAG_SOURCEALL = "_SourceAll";
const string CONST_FILENAME_TAG_SINKDATA = "_Sink";

const string CONST_OUTPUT_ASCFILE_EXTENSION = ".out";
const string CONST_OUTPUT_IMGFILE_EXTENSION = ".png";
const string CONST_OUTPUT_PROJECTIONFILE_EXTENSION = ".prj";
const string CONST_OUTPUT_QMLFILE_EXTENSION_LCASE = ".qml";
const string CONST_OUTPUT_QMLFILE_EXTENSION_UCASE = ".QML";

const string CONST_TIME_FIELD_NAME = "DataTime";

const int CONST_IMG_WIDTH = 600;
const int CONST_IMG_HEIGHT = 600;


typedef struct cvatt
{// -1 : false, 1: true
	int isSimulatingCell;  // -1 : false, 1: true
	int colx;
	int rowy;
	double elez;
	int cvaryNum_atW;
	int cvaryNum_atE;
	int cvaryNum_atN;
	int cvaryNum_atS;

	double rc;
	double impervR;

	double dp_tp1;  // 새로 계산될 수심 t+dt
	double dp_t; //현재 기존 수심
	double hp_tp1;//z+d

	double dfe; //e로의 흐름수심
	double dfs; //s로의 흐름수심

	double ve_tp1;
	double vs_tp1;
	/// <summary>
	/// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	/// </summary>
	double slpe;

	/// <summary>
	/// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	/// </summary>
	double slps;

	double qe_tp1;
	double qw_tp1;
	double qs_tp1;
	double qn_tp1;

	double qe_t;
	double qw_t;
	double qs_t;
	double qn_t;

	double resd; //residual
};


//GPU parameter 로 넘기는 매개변수를 최소화 하기 위해서 이것을 추가로 사용한다. 여기에 포함된 값은 gpu로 안넘긴다.
typedef struct cvattAdd
{// -1 : false, 1: true
	//int cvid;
	double rfReadintensity_mPsec;
	double sourceRFapp_dt_meter;
	double bcData_curOrder;
	double bcData_nextOrder;
	double bcData_curOrderStartedTime_sec;

	double initialConditionDepth_m;

	/// <summary>
	/// cms
	/// </summary>
	double Qmax_cms;
	double vmax;
	int fdmax; // N = 1, E = 4, S = 16, W = 64, NONE = 0
};

typedef struct domaininfo
{
	double dx;
	int nRows;
	int nCols;
	double xll;
	double yll;
	double cellSize;
	int nodata_value;
	string headerStringAll;
};

typedef struct domainCell
{
	int isInDomain;
	int cvid;
	//double elez;
};

typedef struct LCInfo
{
	int LCCode;
	string LCname;
	double roughnessCoeff;
	double imperviousRatio;
};

typedef struct generalEnv
{
	int modelSetupIsNormal;
	double gravity;
	double dMinLimitforWet; // 이거보다 같거나 작으면 마른 것이다.
	double dMinLimitforWet_ori;
	double slpMinLimitforFlow; //이거보다 작으면 경사가 없는 것이다.
	double dtMaxLimit_sec;
	double dtMinLimit_sec;
	double dtStart_sec;
	double dflowmaxInThisStep; // courant number 계산용
	double vmaxInThisStep;
	double VNConMinInThisStep;
	double convergenceConditionh;
	double convergenceConditionhr;
	double convergenceConditionq;
	//int iNRmax_forCE;
	//int iNRmax_forME;
	//int iGSmax;
	bool isAnalyticSolution = false;
	bool isDWE = false;
	bool vdtest = false;
	bool movingDomain = true;
	int iGS = 0;
	int iNR = 0;
	int cellCountNotNull;
	//int iGSmax_GPU = 0;
	//int iNRmax_GPU = 0;
	//vector<double> floodingCellDepthThresholds_m;// 수렴 조건 적용
};

typedef struct projectFile
{
	string fpnDEM;
	string fpnDEMprjection;
	string fpnLandCover;
	string fpnLandCoverVat;
	int usingLCFile;
	int isFixedDT;// true : 1, false : -1
	double calculationTimeStep_sec;
	int isParallel;// true : 1, false : -1
	int maxDegreeOfParallelism;
	int usingGPU;// true : 1, false : -1
	int effCellThresholdForGPU;
	int maxIterationAllCellsOnCPU;
	int maxIterationACellOnCPU;
	int maxIterationAllCellsOnGPU;
	int maxIterationACellOnGPU;
	double printOUTinterval_min;
	double simDuration_hr;
	double simDuration_min;
	string startDateTime;
	rainfallDataType rainfallDataType;
	int rainfallDataInterval_min;
	string rainfallFile;
	int bcDataInterval_min;
	vector<double> floodingCellDepthThresholds_cm;
	int outputDepth;// true : 1, false : -1
	int outputHeight;// true : 1, false : -1	
	int outputVelocityMax;// true : 1, false : -1	
	int outputFDofMaxV;// true : 1, false : -1
	int outputDischargeMax;// true : 1, false : -1	
	int outputRFGrid;// true : 1, false : -1
	double depthImgRendererMaxV;
	double heightImgRendererMaxV;
	double velocityMaxImgRendererMaxV;
	double dischargeImgRendererMaxV;
	double rfImgRendererMaxV;
	int makeASCFile; // true : 1, false : -1
	int makeImgFile;// true : 1, false : -1
	int writeLog;// true : 1, false : -1

	double roughnessCoeff;
	double imperviousR;
	double domainOutBedSlope;
	conditionDataType icDataType;
	fileOrConstant icType;
	string icFPN;
	int usingicFile;
	double icValue_m; // ic는 height와 depth만 사용함
	double froudeNumberCriteria;
	double courantNumber;
	int applyVNC;

	vector<vector<cellPosition>> bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	//map <int, vector<cellPosition> bcCellXY;
	vector<string> bcDataFile;
	vector<conditionDataType> bcDataType;
	int bcCount;

	vector<double> timeToChangeDEM_min;
	vector<string> fpnDEMtoChange;
	int DEMtoChangeCount;
};


int deleteAlloutputFiles();
void setGenEnv();
int setupDomainAndCVinfo();
map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat);
int changeDomainElevWithDEMFileUsingArray(string demfpn, domaininfo indm, domainCell **indmcells, cvatt *incvs);


