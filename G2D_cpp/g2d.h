#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <ATLComTime.h>
#include <list>
#include <map>
#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

//#define OnGPU // Define OnGPU to build the GPU optimized simulator

#define _ATL_DEBUG_INTERFACES
#define GRAVITY		9.80665
#define slpMIN		0.0 //이거보다 작으면 경사가 없는 것이다. 
#define CCh			0.00001 // convergence condition h
#define dMinLimit    0.000001 // 최소 수심 값. 이 값보다 작으면 dry.

//#define isDWE		//	0//using dynamic wave equation ?  0 : false, 1: true
#define isVD		//	0// virtual domain?  0 : false, 1: true
//#define isAS		//	0//analytic Solution ?,  0 : false, 1: true
#ifndef isAS //== 0			
//해석해 모의가 아니면, 아래의 조건 적용
	#define dtMAX_sec	300.0
	#define dtMIN_sec	0.01
#else
// 만일 해석해 모의이면, 아래의 조건 적용
	#define dtMAX_sec  2
	#define dtMIN_sec   1
#endif

const string CONST_FILENAME_TAG_DISCHARGE = "_Discharge";
const string CONST_FILENAME_TAG_DEPTH = "_Depth";
const string CONST_FILENAME_TAG_HEIGHT = "_Height";
const string CONST_FILENAME_TAG_VELOCITY = "_Velocity";
const string CONST_FILENAME_TAG_FLOWDIRECTION = "_FD";
//const string CONST_FILENAME_TAG_RFGRID = "_RFGrid";
//const string CONST_FILENAME_TAG_BCDATA = "_BC";
//const string CONST_FILENAME_TAG_SOURCEALL = "_SourceAll";
//const string CONST_FILENAME_TAG_SINKDATA = "_Sink";

const string CONST_OUTPUT_ASCFILE_EXTENSION = ".out";
const string CONST_OUTPUT_IMGFILE_EXTENSION = ".bmp";
const string CONST_OUTPUT_PROJECTIONFILE_EXTENSION = ".prj";
const string CONST_OUTPUT_QMLFILE_EXTENSION_LCASE = ".qml";
const string CONST_OUTPUT_QMLFILE_EXTENSION_UCASE = ".QML";

const string CONST_TIME_FIELD_NAME = "DataTime";

const int CONST_IMG_WIDTH = 600;
const int CONST_IMG_HEIGHT = 600;

typedef struct _projectFileTable
{
	const string nProjectSettings = "ProjectSettings";
	const string nHydroPars = "HydroPars";
	const string nBoundaryConditionData = "BoundaryConditionData";
	const string nDEMFileToChange = "DEMFileToChange";
	int sProjectSettings = 0; //0::비활성, 1: 활성
	int sHydroPars = 0; //0:비활성, 1: 활성
	int sBoundaryConditionData = 0; //0:비활성, 1: 활성
	int sDEMFileToChange = 0; //0:비활성, 1: 활성
} projectFileTable;

typedef struct _projectFileFieldName
{
	const string DomainDEMFile_01 = "DEMFile";
	const string DomainDEMFile_02 = "DomainDEMFile";
	const string LandCoverFile = "LandCoverFile";
	const string LandCoverVatFile = "LandCoverVatFile";
	const string CalculationTimeStep_sec_01 = "CalculationTimeStep_sec";
	const string CalculationTimeStep_sec_02 = "CalculationTimeInterval_sec";
	const string IsFixedDT = "IsFixedDT";
	const string MaxDegreeOfParallelism_01 = "MaxDegreeOfParallelism";
	const string MaxDegreeOfParallelism_02 = "MaxDegreeOfParallelismCPU";
	const string UsingGPU = "UsingGPU";
	const string ThreadsPerBlock = "ThreadsPerBlock";
	const string MaxIterationAllCells_01 = "MaxIterationAllCellsOnCPU";
	const string MaxIterationAllCells_02 = "MaxIterationAllCells";
	const string MaxIterationACell_01 = "MaxIterationACellOnCPU";
	const string MaxIterationACell_02 = "MaxIterationACell";
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
	const string DepthImgRendererMaxV = "DepthImgRendererMaxV";
	const string HeightImgRendererMaxV = "HeightImgRendererMaxV";
	const string VelocityMaxImgRendererMaxV = "VelocityMaxImgRendererMaxV";
	const string DischargeImgRendererMaxV = "DischargeImgRendererMaxV";
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

	const string bcCellXY_01 = "CellXY";
	const string bcCellXY_02 = "bcCellXY";
	const string bcDataFile_01 = "DataFile";
	const string bcDataFile_02 = "bcDataFile";
	const string bcDataType_01 = "DataType";
	const string bcDataType_02 = "bcDataType";
	const string TimeMinuteToChangeDEM_01 = "TimeMinute";
	const string TimeMinuteToChangeDEM_02 = "TimeMinuteToChangeDEM";
	const string DEMFileToChange_01 = "DEMFile";
	const string DEMFileToChange_02 = "DEMFileToChange";
} projectFileFieldName;


typedef struct _bcAppinfo
{
	int cvidx = 0;
	double bcDepth_dt_m_tp1 = 0.0;
	int bctype = 0; //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
	double bcData_curOrder = 0.0;
	double bcData_nextOrder = 0.0;
	int bcData_curOrderStartedTime_sec = 0;
} bcAppinfo;


 typedef struct _bcinfo
{
	cellPosition* bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	int nCellsInAbc=0;
	conditionDataType bcDataType= conditionDataType::NoneCD;
	double* bcValues;
	int bcValueCount=0;
} bcinfo;


typedef struct _demToChangeinfo
{
	double timeToChangeDEM_min = 0;;
	string fpnDEMtoChange="";
} demToChangeinfo;

typedef struct _cvatt
{// 0 : false, 1: true
	int isSimulatingCell=0;  // 0 : false, 1: true
	int colx = -1;
	int rowy = -1;
	int isBCcell = -1;
	int cvidx_atW = -1;
	int cvdix_atE = -1;
	int cvidx_atN = -1;
	int cvidx_atS = -1;

	double rc = 0.0; // 이거까지는 double로 해야 한다. 아니면, 잔차 커지고 runtime 길어진다.
	float impervR = 0.0f;

	double dp_tp1 = 0.0;  // 새로 계산될 수심 t+dt
	double dp_t = 0.0; //현재 기존 수심
	double hp_tp1 = 0.0;//z+d

	double dfe = 0.0; //e로의 흐름수심
	double dfs = 0.0; //s로의 흐름수심
	//double dfn = 0.0; 
	//double dfw = 0.0;

	double ve_tp1 = 0.0;
	double vs_tp1 = 0.0;
	//double vw_tp1 = 0.0;
	//double vn_tp1 = 0.0;

	// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	double slpe = 0.0;
	// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
	double slps = 0.0;

	double qe_tp1 = 0.0;
	double qw_tp1 = 0.0; //w 경계셀의 유량을 여기에 저장해야 한다. w셀의 e 성분이 없다.
	double qs_tp1 = 0.0;
	double qn_tp1 = 0.0; //n 경계셀의 유량을 여기에 저장해야 한다. n셀의 s 성분이 없다.

	double qe_t = 0.0;
	double qw_t = 0.0;
	double qs_t = 0.0;
	double qn_t = 0.0;

	double resd=0.0; //residual
} cvatt;


//GPU parameter 로 넘기는 매개변수를 최소화 하기 위해서 이것을 추가로 사용한다. 여기에 포함된 값은 gpu로 안넘긴다.
typedef struct _cvattAddAtt
{
	double sourceRFapp_dt_meter = 0.0;
	double initialConditionDepth_m = 0.0;
	double Qmax_cms = 0.0;
	double vmax = 0.0;
	int fdmaxV=0; //E = 1, S = 3, W = 5, N = 7, NONE = 0
} cvattAddAtt;

typedef struct _domaininfo
{
	float dx=0.0f;
	int nRows=0;
	int nCols=0;
	double xll=0.0;
	double yll=0.0;
	float cellSize=0.0f;
	int nodata_value=-9999;
	string headerStringAll = "";
	int cellNnotNull = 0;
} domaininfo;

typedef struct _domainCell
{
	int isInDomain=0;
	int cvidx = -1;
} domainCell;

typedef struct _cellResidual
{
	double residual=0.0;
	int cvidx=-1;
} cellResidual;

typedef struct _fluxData
{
	double v=0.0;
	double slp=0.0;
	double dflow=0.0;
	double q=0.0;
	int fd=0; //E = 1, S = 3, W = 5, N = 7, NONE = 0
} fluxData;

typedef struct _generalEnv
{
	int modelSetupIsNormal=1;// 0 : false, 1: true
	double dtStart_sec=0.01;
} generalEnv;

typedef struct _LCInfo
{
	int LCCode = 0;
	string LCname = "";
	double roughnessCoeff = 0.0;
	float imperviousRatio = 0.0f;
} LCInfo;

typedef struct _rainfallinfo
{
	int order = 0;
	string rainfall = "";
	string dataFile = "";
	string dataTime = "";
} rainfallinfo;

typedef struct _depthClassInfo
{
	int* floodingCellCount;
	double* floodingDepthSum;
}depthClassInfo;

typedef struct _thisProcess
{
	vector<int> FloodingCellCounts; // the number of cells that have water depth.
	vector<double> FloodingCellMeanDepth; //여기서 초기화하면 초기화 값이 push_back 된다.
	double FloodingCellMaxDepth = 0.0;
	vector<double> floodingCellDepthThresholds_m;
	COleDateTime simulationStartTime;
	COleDateTime thisPrintStepStartTime;
	int dt_printout_sec=0;
	int tTag_length = 0;
	int mdp = 0;
	int threadsPerBlock = 0;
	double maxResd;
int maxResdCVidx;
} thisProcess;

typedef struct _thisProcessInner
{
	double tsec_targetToprint = 0.0;
	double tnow_min = 0.0;
	double tnow_sec = 0.0;
	int rfEnded = 0;
	double rfReadintensityForMAP_mPsec = 0.0;
	int effCellCount = 0;
} thisProcessInner;

typedef struct _minMaxCVidx {
	double dflowmaxInThisStep;
	double vmaxInThisStep;
	double VNConMinInThisStep;
} minMaxCVidx;

typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	// 0 : false, 1: true
	double dt_sec = 0.0;
	float dx = 0.0f;
	int nCols = 0;
	int nRows = 0;
	int nCellsInnerDomain = 0;
	double domainOutBedSlope = 0.0;
	float froudeNCriteria = 0.0f;
	int iNRmaxLimit = 0;
	int iGSmaxLimit = 0;
	int bcCellCountAll = 0;

	// 아래는 다른 위치로 옮겨보자..	
	int isApplyVNC = 0;	
	int isRFApplied = 0;
	rainfallDataType rfType = rainfallDataType::NoneRF;
	double dtbc_sec = 0.0;
} globalVinner;

// dt 계산시 인수로 전달할 변수들
typedef struct _dataForCalDT {
	float printOutInterval_min;
	int bcDataInterval_min;
	int rainfallDataInterval_min;
	float courantNumber;
	int applyVNC;
	int bcCellCountAll;
}dataForCalDT;

typedef struct _projectFile
{
	string fpnDEM="";
	string fpnDEMprjection="";
	string fpnLandCover="";
	string fpnLandCoverVat="";
	int usingLCFile=0;
	int isFixedDT=0;// true : 1, false : 0
	double calculationTimeStep_sec=0.0;
	int maxDegreeOfParallelism=0;
	int usingGPU=0;// true : 1, false : 0
	int threadsPerBlock = 128;
	int maxIterationAllCellsOnCPU=0;
	int maxIterationACellOnCPU=0;
	float printOutInterval_min=0.0f;
	float simDuration_hr = 0.0f;
	float simDuration_min = 0.0f;
	string startDateTime=""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	int isDateTimeFormat=0;

	rainfallDataType rainfallDataType;
	int rainfallDataInterval_min = 0;;
	string rainfallFPN="";
	int isRainfallApplied=0;
	
	int bcDataInterval_min=0;
	vector<double> floodingCellDepthThresholds_cm;

	int outputDepth = 0;// true : 1, false : 0
	int outputHeight = 0;// true : 1, false : 0	
	int outputVelocityMax = 0;// true : 1, false : 0	
	int outputFDofMaxV = 0;// true : 1, false : 0
	int outputDischargeMax = 0;// true : 1, false : 0	

	double rendererMaxVdepthImg = 0.0;
	double rendererMaxVheightImg = 0.0;
	double rendererMaxVMaxVImg = 0.0;
	double rendererMaxVDischargeImg = 0.0;

	int makeASCFile = 0; // true : 1, false : 0
	int makeImgFile = 0;// true : 1, false : 0
	int writeLog = 0;// true : 1, false : 0

	float roughnessCoeff = 0.0f;
	float imperviousR = 0.0f;
	double domainOutBedSlope = 0.0;

	int isicApplied = 0;// true : 1, false : 0
	conditionDataType icType = conditionDataType::NoneCD;
	fileOrConstant icDataType=fileOrConstant::None;
	string icFPN="";
	int usingicFile = 0;
	double icValue_m = 0.0; // ic는 height와 depth만 사용함
	float froudeNumberCriteria = 0.0f;
	float courantNumber = 0.0f;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : 0
	int bcCount = 0;
	int bcCellCountAll = 0;
	vector<string> bcDataFiles;
	bcinfo* bcis;
	
	int isDEMtoChangeApplied = 0;// true : 1, false : 0	
	int DEMtoChangeCount = 0;
	vector<demToChangeinfo> dcs;

	CPUsInfo cpusi;
	int tTag_length = 0;
	int parChanged = 0;

	string fpnTest_willbeDeleted="";
   //string fpniterAcell_willbeDeleted="";
   //string hvalues_Acell_willbeDeleted="";
} projectFile;

int changeDomainElevWithDEMFile(double tnow_min, 
	double tbefore_min);
void updateSummaryAndSetAllFalse();
void updateSummaryAndSetAllFalse_serial();
int deleteAlloutputFiles();
void disposeDynamicVars();

void initGlobalVinner();
void initFloodingThresholds();
inline void initMinMax();
int initializeOutputArray();
void initThisProcess();
void initilizeThisStep_CPU();
int isNormalBCinfo(bcinfo* bci);
int isNormalDEMtoChangeinfo(demToChangeinfo* dci);

void g2dHelp();
void getCellCD(int dataOrder, int dataInterval_min);
double getDTsecWithConstraints(dataForCalDT dataForDT_L,
	globalVinner gvi_L, double tnow_sec,
	bcAppinfo* bcAppinfos_L, minMaxCVidx mnMxCVidx_L);

void joinOutputThreads();

void makeASCTextFileDepth();
void makeASCTextFileDischargeMax();
void makeASCTextFileFDofVMax();
void makeASCTextFileHeight();
void makeASCTextFileVelocityMax();

void makeImgFileDepth();
void makeImgFileHeight();
void makeImgFileDischargeMax();
void makeImgFileVelocityMax();
int makeOutputFiles(double nowTsec, int iGSmax);

int NRinner(int idx);

int openProjectFile();
int openPrjAndSetupModel();

int readRainfallAndGetIntensity(int rforder);
int readXmlRowProjectSettings(string aline);
int readXmlRowHydroPars(string aline);
int readXmlRowBoundaryConditionData(string aline,
	bcinfo* bci, string * bcDataFile);
int readXmlRowDEMFileToChange(string aline, 
	demToChangeinfo* dc);
int runG2D();
void runSolver_CPU(int* iGSmax);
int setBCinfo();
int setGenEnv();
int setOutputArray();
int setRainfallinfo();
map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat);
int setupDomainAndCVinfo();
int setStartingConditionCVs_CPU();
int simulationControl_CPU();
int simulationControl_GPU();

void updateMinMaxInThisStep_CPU();
int updateProjectParameters();

