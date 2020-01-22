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


typedef struct _bcCellinfo
{
	int cvid = 0;
	float bcDepth_dt_m_tp1 = 0.0f;
	int bctype = 0; //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
} bcCellinfo;

typedef struct _cvatt
{// -1 : false, 1: true
	int isSimulatingCell=0;  // -1 : false, 1: true
	int colx = -1;
	int rowy = -1;
	float elez=0.0;
	int cvaryNum_atW = -1;
	int cvaryNum_atE = -1;
	int cvaryNum_atN = -1;
	int cvaryNum_atS = -1;

	float rc = 0.0;
	float impervR = 0.0;

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
typedef struct _cvattAdd
{// -1 : false, 1: true
	//int cvid;
	float rfReadintensity_mPsec = 0.0;
	float sourceRFapp_dt_meter = 0.0;
	float bcData_curOrder = 0.0;
	float bcData_nextOrder = 0.0;
	int bcData_curOrderStartedTime_sec = 0;
	float initialConditionDepth_m = 0.0;

	/// <summary>
	/// cms
	/// </summary>
	double Qmax_cms = 0.0;
	double vmax = 0.0;
	int fdmax=0; // N = 1, E = 4, S = 16, W = 64, NONE = 0
} cvattAdd;

typedef struct _domaininfo
{
	float dx=0.0;
	int nRows=0;
	int nCols=0;
	double xll=0.0;
	double yll=0.0;
	float cellSize=0.0;
	int nodata_value=-9999;
	string headerStringAll = "";
	int cellCountNotNull = 0;
} domaininfo;

typedef struct _domainCell
{
	int isInDomain=0;
	int cvid = -1;
	//double elez;
} domainCell;

typedef struct _cellResidual
{
	double residual=0.0;
	int cvid=-1;
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
	int modelSetupIsNormal=1;// -1 : false, 1: true
	float gravity= 9.80665f;
	float dMinLimitforWet_ori = 0.000001f; // 이거보다 같거나 작으면 마른 것이다.
	double slpMinLimitforFlow = 0.0; //이거보다 작으면 경사가 없는 것이다. 
	float dtMaxLimit_sec=300.0f;
	float dtMinLimit_sec=0.01f;
	float dtStart_sec=0.01f;
	double convergenceConditionh=0.00001;
	double convergenceConditionhr=0.001;
	double convergenceConditionq=0.0001;
	const int isAnalyticSolution = -1;// -1 : false, 1: true
	const int isDWE = -1;// -1 : false, 1: true
	const int vdtest = -1;// -1 : false, 1: true
	const int movingDomain = 1;// -1 : false, 1: true

	//int cellCountNotNull=0;
	//int iGSmax_GPU = 0;
	//int iNRmax_GPU = 0;
} generalEnv;

typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	// -1 : false, 1: true
	float dx = 0.0f;
	int nCols = 0;
	int nRows = 0;
	int nCellsInnerDomain = 0;
	int bcCellCountAll = 0;
	//int isparallel = 1;
	float dMinLimitforWet = 0.0f;
	//float dMinLimitforWet_ori = 0.0f;
	double slpMinLimitforFlow = 0.0;
	float domainOutBedSlope = 0.0f;
	double ConvgC_h = 0.0;
	float froudeNCriteria = 0.0f;
	int iNRmax = 0;
	int iGSmax = 0;
	//int iNR = 0;
	//int iGS = 0;
	float gravity = 0.0f;
	int isDWE = 0;
	int isAnalyticSolution = 0;
	int isApplyVNC = 0;
	//int bAllConvergedInThisGSiteration = 0;
	int mdp = 0;
	//int isParallel = 0;
} globalVinner;

typedef struct _LCInfo
{
	int LCCode = 0;
	string LCname = "";
	float roughnessCoeff = 0.0;
	float imperviousRatio = 0.0;
} LCInfo;

typedef struct _rainfallinfo
{
	int order = 0;
	string rainfall = "";
	string dataFile = "";
	string dataTime = "";
} rainfallinfo;

typedef struct _thisProcess
{
	//float dt_sec=0.0;
	//int isfixeddt = 0;// -1 : false, 1: true
	//int isparallel = 0;// -1 : false, 1: true
	double tsec_targetToprint = 0.0;
	double tnow_min = 0.0;
	double tnow_sec = 0.0;
	int effCellCount = 0;
	vector<int> FloodingCellCounts; // the number of cells that have water depth.
	vector<double> FloodingCellMeanDepth; //여기서 초기화하면 초기화 값이 push_back 된다.
	double FloodingCellMaxDepth = 0.0;
	vector<double> floodingCellDepthThresholds_m;
	COleDateTime simulationStartTime;
	COleDateTime thisPrintStepStartTime;
	float dt_printout_min = 0.0;
	int dt_printout_sec=0;
} thisProcess;

typedef struct _thisProcessInner
{
	//double* subregionVmax;
	//double* subregionDflowmax;
	//double* subregionVNCmin;
	float dt_sec = 0.0f;
	int bAllConvergedInThisGSiteration=-1;// 1:true, -1: false
	//int maxNR_inME = 0;

	int iNR = 0;
	int iGS = 0;
	double maxResd = 0;
	int maxResdCVID = -1;
	//int maxResdCellxCol =0;
	//int maxResdCellyRow = 0;
	//double* subregionMaxResd;
	//string* subregionMaxResdCell;
	double dflowmaxInThisStep = 0.0f; // courant number 계산용
	double vmaxInThisStep = 0.0f;
	double VNConMinInThisStep = DBL_MAX;
	float rfReadintensityForMAP_mPsec = 0.0;
	int rfisGreaterThanZero = 1; // 1:true, -1: false
} thisProcessInner;

typedef struct _projectFile
{
	string fpnDEM="";
	string fpnDEMprjection="";
	string fpnLandCover="";
	string fpnLandCoverVat="";
	int usingLCFile=0;
	int isFixedDT=0;// true : 1, false : -1
	float calculationTimeStep_sec=0.0;
	//int isParallel=0;// true : 1, false : 0
	int maxDegreeOfParallelism=0;
	int usingGPU=0;// true : 1, false : -1
	int effCellThresholdForGPU=0;
	int maxIterationAllCellsOnCPU=0;
	int maxIterationACellOnCPU=0;
	int maxIterationAllCellsOnGPU=0;
	int maxIterationACellOnGPU=0;
	float printOutInterval_min=0.0;
	double simDuration_hr = 0.0;
	double simDuration_min = 0.0;
	string startDateTime=""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	int isDateTimeFormat=0;

	rainfallDataType rainfallDataType;
	int rainfallDataInterval_min = 0;;
	string rainfallFPN="";
	int isRainfallApplied=0;
	
	int bcDataInterval_min=0;
	vector<float> floodingCellDepthThresholds_cm;

	int outputDepth = 0;// true : 1, false : -1
	int outputHeight = 0;// true : 1, false : -1	
	int outputVelocityMax = 0;// true : 1, false : -1	
	int outputFDofMaxV = 0;// true : 1, false : -1
	int outputDischargeMax = 0;// true : 1, false : -1	
	//int outputRFGrid = 0;// true : 1, false : -1

	float rendererMaxVdepthImg = 0.0;
	float rendererMaxVheightImg = 0.0;
	double rendererMaxVMaxVImg = 0.0;
	double rendererMaxVDischargeImg = 0.0;
	//float rfImgRendererMaxV = 0.0;

	int makeASCFile = 0; // true : 1, false : -1
	int makeImgFile = 0;// true : 1, false : -1
	int writeLog = 0;// true : 1, false : -1

	float roughnessCoeff = 0.0;
	float imperviousR = 0.0;
	float domainOutBedSlope = 0.0;

	int isicApplied = 0;// true : 1, false : -1
	conditionDataType icType = conditionDataType::NoneCD;
	fileOrConstant icDataType=fileOrConstant::None;
	string icFPN="";
	int usingicFile = 0;
	float icValue_m = 0.0; // ic는 height와 depth만 사용함
	float froudeNumberCriteria = 0.0;
	float courantNumber = 0.0;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : -1
	vector<vector<cellPosition>> bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	//map <int, vector<cellPosition> bcCellXY;
	vector<string> bcDataFile;
	vector<conditionDataType> bcDataType;
	vector<vector<float>> bcValues;
	int bcCount = 0;
	int bcCellCountAll = 0;

	int isDEMtoChangeApplied = 0;// true : 1, false : -1
	vector<float> timeToChangeDEM_min;
	vector<string> fpnDEMtoChange;
	int DEMtoChangeCount = 0;

	CPUsInfo cpusi;

	string fpnTest_willbeDeleted="";
	string fpniterAcell_willbeDeleted="";
	string hvalues_Acell_willbeDeleted="";
} projectFile;

typedef struct _projectFileFieldName
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
	//const string OutputBCData = "OutputBCData";
	//const string OutputRFGrid = "OutputRFGrid";
	const string DepthImgRendererMaxV = "DepthImgRendererMaxV";
	const string HeightImgRendererMaxV = "HeightImgRendererMaxV";
	const string VelocityMaxImgRendererMaxV = "VelocityMaxImgRendererMaxV";
	const string DischargeImgRendererMaxV = "DischargeImgRendererMaxV";
	//const string RFImgRendererMaxV = "RFImgRendererMaxV";
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
} projectFileFieldName;

int calculateContinuityEqUsingNRforCPU(int idx, int isBCCell, double dcdtpth, int bctype);
fluxData calculateMomentumEQ_DWE_Deterministric(double qt, double dflow,
	double slp, double gravity, double rc, double dx, double dt_sec, 
	double q_ip1, double u_ip1);
fluxData calculateMomentumEQ_DWEm_Deterministric(
	double qt, double gravity, double dt_sec, double slp,
	double rc, double dflow, double qt_ip1);
void calEFlux(int idx, int isBCcell);
void calNFlux(int idx, int isBCcell);
void calSFlux(int idx, int isBCcell);
void calWFlux(int idx, int isBCcell);
int changeDomainElevWithDEMFile(double tnow_min, double tbefore_min);
void checkEffetiveCellNumberAndSetAllFlase();
int deleteAlloutputFiles();
void disposeDynamicVars();
globalVinner initGlobalVinner();
int initializeOutputArray();
void initilizeThisStep(float dt_sec, double nowt_sec, int bcdt_sec, int rfEnded);
void initializeThisStepAcell(int idx, float dt_sec, int dtbc_sec, double nowt_sec, int rfEnded);
void g2dHelp();
int getbcCellArrayIndex(int cvid);
void getCellConditionData(int dataOrder, int dataInterval_min);
float getConditionDataAsDepthWithLinear(int bctype, float elev_m,
	float dx, cvattAdd cvaa, float dtsec,
	int dtsec_cdata, double nowt_sec);
fluxData getFD4MaxValues(cvatt cell, cvatt wcell, cvatt ncell);
fluxData getFluxToEastOrSouthUsing1DArray(cvatt curCell,
	cvatt tarCell, int targetCellDir);
fluxData getFluxUsingFluxLimitBetweenTwoCell(fluxData inflx, double dflow,
	double dx, double dt_sec);
fluxData getFluxUsingSubCriticalCon(fluxData inflx,
	double gravity, double froudNCriteria);
double getVonNeumanConditionValue(cvatt cell);
void join_threads();

void makeASCTextFileDepth();
void makeASCTextFileDischargeMax();
void makeASCTextFileFDofVMax();
void makeASCTextFileHeight();
void makeASCTextFileVelocityMax();

void makeImgFileDepth();
void makeImgFileHeight();
void makeImgFileDischargeMax();
void makeImgFileVelocityMax();
//void makeImgFDofVMax();

int makeOutputFiles(double nowTsec);
fluxData noFlx();
int NRinner(int idx, int isBCCell, double dbdtpth, int bctype);
int openPrjAndSetupModel();
int readRainfallAndGetIntensity(int rforder);
int runG2D();
int runSolverUsingGPU();
int runSolverUsingCPU();
int setBCinfo();
void setEffectiveCells(int idx);
int setGenEnv();
int setOutputArray();
int setRainfallinfo();
map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat);
void setStartingCondidtionInACell(int i);
int setupDomainAndCVinfo();
int setStartingConditionUsingCPU();
//void setStartingCondidtionInACell(cvatt* cvsL, int idx, cvattAdd* cvsaddL);
int simulationControlUsingCPUnGPU();

void updateValuesInThisStepResults();
int updateProjectParameters();



//int changeDomainElevWithDEMFileUsingArray(string demfpn, domaininfo indm, domainCell **indmcells, cvatt *incvs); 이건 prj open 할때 설정됨


