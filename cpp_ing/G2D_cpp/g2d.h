#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include<ATLComTime.h>
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
	int isSimulatingCell=0;  // -1 : false, 1: true
	int colx = -1;
	int rowy = -1;
	double elez=0.0;
	int cvaryNum_atW = -1;
	int cvaryNum_atE = -1;
	int cvaryNum_atN = -1;
	int cvaryNum_atS = -1;

	double rc = 0.0;
	double impervR = 0.0;

	double dp_tp1 = 0.0;  // 새로 계산될 수심 t+dt
	double dp_t = 0.0; //현재 기존 수심
	double hp_tp1 = 0.0;//z+d

	double dfe = 0.0; //e로의 흐름수심
	double dfs = 0.0; //s로의 흐름수심

	double ve_tp1 = 0.0;
	double vs_tp1 = 0.0;
	/// <summary>
	/// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	/// </summary>
	double slpe = 0.0;

	/// <summary>
	/// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	/// </summary>
	double slps = 0.0;

	double qe_tp1 = 0.0;
	double qw_tp1 = 0.0;
	double qs_tp1 = 0.0;
	double qn_tp1 = 0.0;

	double qe_t = 0.0;
	double qw_t = 0.0;
	double qs_t = 0.0;
	double qn_t = 0.0;

	double resd; //residual
};


//GPU parameter 로 넘기는 매개변수를 최소화 하기 위해서 이것을 추가로 사용한다. 여기에 포함된 값은 gpu로 안넘긴다.
typedef struct cvattAdd
{// -1 : false, 1: true
	//int cvid;
	double rfReadintensity_mPsec = 0.0;
	double sourceRFapp_dt_meter = 0.0;
	double bcData_curOrder = 0.0;
	double bcData_nextOrder = 0.0;
	double bcData_curOrderStartedTime_sec = 0.0;
	double initialConditionDepth_m = 0.0;

	/// <summary>
	/// cms
	/// </summary>
	double Qmax_cms = 0.0;
	double vmax = 0.0;
	int fdmax=0; // N = 1, E = 4, S = 16, W = 64, NONE = 0
};

typedef struct domaininfo
{
	float dx=0.0;
	int nRows=0;
	int nCols=0;
	double xll=0.0;
	double yll=0.0;
	float cellSize=0.0;
	int nodata_value=-9999;
	string headerStringAll = "";
};

typedef struct domainCell
{
	int isInDomain=0;
	int cvid = 0;
	//double elez;
};

typedef struct LCInfo
{
	int LCCode = 0;
	string LCname = "";
	double roughnessCoeff = 0.0;
	double imperviousRatio = 0.0;
};

typedef struct rainfallinfo
{
	int order = 0;
	string rainfall="";
	string dataFile = "";
	string dataTime = "";
};
typedef struct bcCellinfo
{
	int cvid = 0;
	double bcDepth_dt_m_tp1;
	int bctype = 0; //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
};


typedef struct generalEnv
{
	int modelSetupIsNormal=-1;// -1 : false, 1: true
	double gravity=9.81;
	double dMinLimitforWet = 0.0; // 이거보다 같거나 작으면 마른 것이다.
	double dMinLimitforWet_ori = 0.0;
	double slpMinLimitforFlow = 0.0; //이거보다 작으면 경사가 없는 것이다.
	double dtMaxLimit_sec=300;
	double dtMinLimit_sec=0.01;
	double dtStart_sec=0.01;
	double dflowmaxInThisStep = 0.0; // courant number 계산용
	double vmaxInThisStep=0.0;
	double VNConMinInThisStep = 0.01;
	double convergenceConditionh=0.00001;
	double convergenceConditionhr=0.001;
	double convergenceConditionq=0.0001;
	int iNRmax;
	//int iNRmax_forME;
	int iGSmax;
	const int isAnalyticSolution = -1;// -1 : false, 1: true
	const int isDWE = -1;// -1 : false, 1: true
	const int vdtest = -1;// -1 : false, 1: true
	const int movingDomain = 1;// -1 : false, 1: true
	int iGS = 0;
	int iNR = 0;
	int cellCountNotNull=0;
	//int iGSmax_GPU = 0;
	//int iNRmax_GPU = 0;
	//vector<double> floodingCellDepthThresholds_m;// 수렴 조건 적용
};

typedef struct globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	// -1 : false, 1: true
	float dx;
	int nCols;
	int nRows;
	double dMinLimitforWet;
	double dMinLimitforWet_ori;
	double slpMinLimitforFlow;
	double domainOutBedSlope;
	double ConvgC_h;
	double froudNCriteria;
	int iNRmax;
	int iGSmax;
	int iNR;
	int iGS;
	double gravity;
	int isDWE;
	int isAnalyticSolution;
	int isApplyVNC;
	double dt_sec;
	int bAllConvergedInThisGSiteration;
};

typedef struct thisProcess
{
	float dt_sec=0.0;
	int isfixeddt = 0;// -1 : false, 1: true
	int isparallel = 0;// -1 : false, 1: true
	double tsec_targetToprint = 0.0;
	double tnow_min = 0.0;
	double tnow_sec = 0.0;
	COleDateTime simulationStartTime;
	COleDateTime thisPrintStepStartTime;
	double dt_printout_min = 0.0;
	int dt_printout_sec=0;
};

typedef struct thisProcessInner
{
	//double* subregionVmax;
	//double* subregionDflowmax;
	//double* subregionVNCmin;
	bool bAllConvergedInThisGSiteration;
	int maxNR_inME = 0;
	double maxResd = 0;
	string maxResdCell = "";
	//double* subregionMaxResd;
	//string* subregionMaxResdCell;
	int effCellCount = 0;
	vector<int> FloodingCellCounts; // the number of cells that have water depth.
	vector<double> FloodingCellMeanDepth;
	double FloodingCellMaxDepth=0.0;
	double rfReadintensityForMAP_mPsec = 0.0;
	int rfisGreaterThanZero = 1; // 1:true, -1: false
};

typedef struct projectFile
{
	string fpnDEM="";
	string fpnDEMprjection="";
	string fpnLandCover="";
	string fpnLandCoverVat="";
	int usingLCFile=0;
	int isFixedDT=0;// true : 1, false : -1
	double calculationTimeStep_sec=0.0;
	int isParallel=0;// true : 1, false : -1
	int maxDegreeOfParallelism=0;
	int usingGPU=0;// true : 1, false : -1
	int effCellThresholdForGPU=0;
	int maxIterationAllCellsOnCPU=0;
	int maxIterationACellOnCPU=0;
	int maxIterationAllCellsOnGPU=0;
	int maxIterationACellOnGPU=0;
	double printOUTinterval_min=0.0;
	double simDuration_hr = 0.0;
	double simDuration_min = 0.0;
	string startDateTime=""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	int isDateTimeFormat=0;

	rainfallDataType rainfallDataType;
	int rainfallDataInterval_min = 0;;
	string rainfallFPN="";
	int isRainfallApplied=0;
	
	int bcDataInterval_min=0;
	vector<double> floodingCellDepthThresholds_cm;

	int outputDepth = 0;// true : 1, false : -1
	int outputHeight = 0;// true : 1, false : -1	
	int outputVelocityMax = 0;// true : 1, false : -1	
	int outputFDofMaxV = 0;// true : 1, false : -1
	int outputDischargeMax = 0;// true : 1, false : -1	
	int outputRFGrid = 0;// true : 1, false : -1

	double depthImgRendererMaxV = 0.0;
	double heightImgRendererMaxV = 0.0;
	double velocityMaxImgRendererMaxV = 0.0;
	double dischargeImgRendererMaxV = 0.0;
	double rfImgRendererMaxV = 0.0;

	int makeASCFile = 0; // true : 1, false : -1
	int makeImgFile = 0;// true : 1, false : -1
	int writeLog = 0;// true : 1, false : -1

	double roughnessCoeff = 0.0;
	double imperviousR = 0.0;
	double domainOutBedSlope = 0.0;

	int isicApplied = 0;// true : 1, false : -1
	conditionType icType = conditionType::NoneCD;
	fileOrConstant icDataType=fileOrConstant::None;
	string icFPN="";
	int usingicFile = 0;
	double icValue_m = 0.0; // ic는 height와 depth만 사용함
	double froudeNumberCriteria = 0.0;
	double courantNumber = 0.0;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : -1
	vector<vector<cellPosition>> bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	//map <int, vector<cellPosition> bcCellXY;
	vector<string> bcDataFile;
	vector<conditionType> bcDataType;
	vector<vector<double>> bcValues;
	int bcCount = 0;
	int bcCellCountAll = 0;

	int isDEMtoChangeApplied = 0;// true : 1, false : -1
	vector<double> timeToChangeDEM_min;
	vector<string> fpnDEMtoChange;
	int DEMtoChangeCount = 0;

	string fpnTest_willbeDeleted="";
	string fpniterAcell_willbeDeleted="";
	string hvalues_Acell_willbeDeleted="";
};

typedef struct projectFileFieldName
{
	const string DomainDEMFile = "DomainDEMFile";
	const string LandCoverFile = "LandCoverFile";
	const string LandCoverVatFile = "LandCoverVatFile";
	const string CalculationTimeStep_sec = "CalculationTimeStep_sec";
	const string IsFixedDT = "IsFixedDT";
	const string IsParallel = "IsParallel";
	const string MaxDegreeOfParallelism = "MaxDegreeOfParallelism";
	const string UsingGPU = "UsingGPU";
	const string EffCellThresholdForGPU = "EffCellThresholdForGPU";
	const string MaxIterationAllCellsOnCPU = "MaxIterationAllCellsOnCPU";
	const string MaxIterationACellOnCPU = "MaxIterationACellOnCPU";
	const string MaxIterationAllCellsOnGPU = "MaxIterationAllCellsOnGPU";
	const string MaxIterationACellOnGPU = "MaxIterationACellOnGPU";
	const string PrintoutInterval_min = "PrintoutInterval_min";
	const string SimulationDuration_hr = "SimulationDuration_hr";
	const string StartDateTime = "StartDateTime"; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	const string RainfallDataType = "RainfallDataType";
	const string RainfallDataInterval_min = "RainfallDataInterval_min";
	const string RainfallFile = "RainfallFile";
	const string BCDataInterval_min = "BCDataInterval_min";
	const string FloodingCellDepthThresholds_cm = "FloodingCellDepthThresholds_cm";
	const string OutputDepth = "OutputDepth";
	const string OutputHeight = "OutputHeight";
	const string OutputVelocityMax = "OutputVelocityMax";
	const string OutputFDofMaxV = "OutputFDofMaxV";
	const string OutputDischargeMax = "OutputDischargeMax";
	const string OutputBCData = "OutputBCData";
	const string OutputRFGrid = "OutputRFGrid";
	const string DepthImgRendererMaxV = "DepthImgRendererMaxV";
	const string HeightImgRendererMaxV = "HeightImgRendererMaxV";
	const string VelocityMaxImgRendererMaxV = "VelocityMaxImgRendererMaxV";
	const string DischargeImgRendererMaxV = "DischargeImgRendererMaxV";
	const string RFImgRendererMaxV = "RFImgRendererMaxV";
	const string MakeASCFile = "MakeASCFile";
	const string MakeImgFile = "MakeImgFile";
	const string WriteLog = "WriteLog";
	const string RoughnessCoeff = "RoughnessCoeff";
	const string DomainOutBedSlope = "DomainOutBedSlope";
	const string InitialConditionType = "InitialConditionType";
	const string InitialCondition = "InitialCondition";
	const string FroudeNumberCriteria = "FroudeNumberCriteria";
	const string CourantNumber = "CourantNumber";
	const string ApplyVNC = "ApplyVNC";
	const string bcCellXY = "bcCellXY";
	const string bcDataFile = "bcDataFile";
	const string bcDataType = "bcDataType";
	const string TimeMinuteToChangeDEM = "TimeMinuteToChangeDEM";
	const string DEMFileToChange = "DEMFileToChange";
};


int deleteAlloutputFiles();
void disposePublicVars();
int setBCinfo();
globalVinner initGlobalVinner();
void g2dHelp();
void getCellConditionData(int dataOrder, int dataInterval_min);
int openPrjAndSetupModel();
int runG2D();
int setGenEnv();
int setRainfallinfo();
map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat);
int setupDomainAndCVinfo();
int setStartingConditionUsingCPU();
void setStartingCondidtionInACell(cvatt* cvsL, int idx, cvattAdd* cvsaddL);
int simulationControlUsingCPUnGPU();
//int changeDomainElevWithDEMFileUsingArray(string demfpn, domaininfo indm, domainCell **indmcells, cvatt *incvs); 이건 prj open 할때 설정됨


