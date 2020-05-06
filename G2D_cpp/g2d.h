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
	int bctype =  0; //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
} bcAppinfo;


typedef struct _bcinfo
{
	vector<cellPosition> bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	string bcDataFile="";
	conditionDataType bcDataType= conditionDataType::NoneCD;
	vector<double> bcValues;
} bcinfo;

typedef struct _demToChangeinfo
{
	double timeToChangeDEM_min = 0;;
	string fpnDEMtoChange="";
} demToChangeinfo;

typedef struct _cvatt
{// -1 : false, 1: true
	int isSimulatingCell=0;  // -1 : false, 1: true
	int colx = -1;
	int rowy = -1;
	int isBCcell = -1;
	double elez=0.0;
	int cvidx_atW = -1;
	int cvdix_atE = -1;
	int cvidx_atN = -1;
	int cvidx_atS = -1;

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
	double rfReadintensity_mPsec = 0.0;
	double sourceRFapp_dt_meter = 0.0;
	double bcData_curOrder = 0.0;
	double bcData_nextOrder = 0.0;
	int bcData_curOrderStartedTime_sec = 0;
	double initialConditionDepth_m = 0.0;

	double Qmax_cms = 0.0;
	double vmax = 0.0;
	int fdmax=0; // N = 1, E = 4, S = 16, W = 64, NONE = 0
} cvattAdd;

typedef struct _domaininfo
{
	double dx=0.0;
	int nRows=0;
	int nCols=0;
	double xll=0.0;
	double yll=0.0;
	double cellSize=0.0;
	int nodata_value=-9999;
	string headerStringAll = "";
	int cellNnotNull = 0;
} domaininfo;

typedef struct _domainCell
{
	int isInDomain=0;
	int cvidx = -1;
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
	double gravity= 9.80665;
	double dMinLimitforWet_ori = 0.000001; // 이거보다 같거나 작으면 마른 것이다.
	double slpMinLimitforFlow = 0.0; //이거보다 작으면 경사가 없는 것이다. 
	double dtMaxLimit_sec=300.0;
	double dtMinLimit_sec=0.01;
	double dtStart_sec=0.01;
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
	double dx = 0.0;
	int nCols = 0;
	int nRows = 0;
	int nCellsInnerDomain = 0;
	int bcCellCountAll = 0;
	double dMinLimitforWet = 0.0;
	double slpMinLimitforFlow = 0.0;
	double domainOutBedSlope = 0.0;
	double ConvgC_h = 0.0;
	double froudeNCriteria = 0.0;
	int iNRmaxLimit = 0;
	int iGSmaxLimit = 0;
	double gravity = 0.0;
	int isDWE = 0;
	int isAnalyticSolution = 0;
	int isApplyVNC = 0;
	//int mdp = 0;
} globalVinner;

typedef struct _LCInfo
{
	int LCCode = 0;
	string LCname = "";
	double roughnessCoeff = 0.0;
	double imperviousRatio = 0.0;
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
	//double dt_printout_min = 0.0;
	int dt_printout_sec=0;
} thisProcess;

typedef struct _thisProcessInner
{
	double dt_sec = 0.0;
	int bAllConvergedInThisGSiteration=-1;// 1:true, -1: false
	//int maxNR_inME = 0;

	int iNRmax = 0;
	int iGSmax = 0;
	double maxResd = 0;
	int maxResdCVID = -1;
	double dflowmaxInThisStep = 0.0; // courant number 계산용
	double vmaxInThisStep = 0.0;
	double VNConMinInThisStep = DBL_MAX;
	double rfReadintensityForMAP_mPsec = 0.0;
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
	double calculationTimeStep_sec=0.0;
	int maxDegreeOfParallelism=0;
	int usingGPU=0;// true : 1, false : -1
	int effCellThresholdForGPU=0;
	int maxIterationAllCellsOnCPU=0;
	int maxIterationACellOnCPU=0;
	int maxIterationAllCellsOnGPU=0;
	int maxIterationACellOnGPU=0;
	double printOutInterval_min=0.0;
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
	//int outputRFGrid = 0;// true : 1, false : -1

	double rendererMaxVdepthImg = 0.0;
	double rendererMaxVheightImg = 0.0;
	double rendererMaxVMaxVImg = 0.0;
	double rendererMaxVDischargeImg = 0.0;
	//double rfImgRendererMaxV = 0.0;

	int makeASCFile = 0; // true : 1, false : -1
	int makeImgFile = 0;// true : 1, false : -1
	int writeLog = 0;// true : 1, false : -1

	double roughnessCoeff = 0.0;
	double imperviousR = 0.0;
	double domainOutBedSlope = 0.0;

	int isicApplied = -1;// true : 1, false : -1
	conditionDataType icType = conditionDataType::NoneCD;
	fileOrConstant icDataType=fileOrConstant::None;
	string icFPN="";
	int usingicFile = -1;
	double icValue_m = 0.0; // ic는 height와 depth만 사용함
	double froudeNumberCriteria = 0.0;
	double courantNumber = 0.0;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : -1
	int bcCount = 0;
	int bcCellCountAll = 0;
	vector<int> bcCVidxList;
	vector<bcinfo> bcis;
	//vector<vector<cellPosition>> bcCellXY; // 하나의 bc에 여러개의 셀을 지정할 수 있다.
	//vector<string> bcDataFile;
	//vector<conditionDataType> bcDataType;
	//vector<vector<double>> bcValues;



	int isDEMtoChangeApplied = 0;// true : 1, false : -1	
	int DEMtoChangeCount = 0;
	vector<demToChangeinfo> dcs;
	//vector<double> timeToChangeDEM_min;
	//vector<string> fpnDEMtoChange;

	CPUsInfo cpusi;

	string fpnTest_willbeDeleted="";
	string fpniterAcell_willbeDeleted="";
	string hvalues_Acell_willbeDeleted="";
} projectFile;


int calculateContinuityEqUsingNRforCPU(int idx);
fluxData calculateMomentumEQ_DWE_Deterministric(double qt, 
	double dflow, double slp, double gravity, double rc, 
	double dx, double dt_sec, double q_ip1, double u_ip1);
fluxData calculateMomentumEQ_DWEm_Deterministric(
	double qt, double gravity, double dt_sec, double slp,
	double rc, double dflow, double qt_ip1);
void calEFlux(int idx);
void calNFlux(int idx);
void calSFlux(int idx);
void calWFlux(int idx);
int changeDomainElevWithDEMFile(double tnow_min, 
	double tbefore_min);
void checkEffetiveCellNumberAndSetAllFlase();
int deleteAlloutputFiles();
void disposeDynamicVars();

globalVinner initGlobalVinner();
int initializeOutputArray();
void initilizeThisStep(double dt_sec, double nowt_sec,
	int bcdt_sec, int rfEnded);
void initializeThisStepAcell(int idx, double dt_sec, 
	int dtbc_sec, double nowt_sec, int rfEnded);
int isNormalBCinfo(bcinfo* bci);
int isNormalDEMtoChangeinfo(demToChangeinfo* dci);

void g2dHelp();
//int getbcCellArrayIndex(int cvid);
void getCellConditionData(int dataOrder, int dataInterval_min);
double getConditionDataAsDepthWithLinear(int bctype,
	double elev_m, 	double dx, cvattAdd cvaa, double dtsec,
	int dtsec_cdata, double nowt_sec);
double getDTsecWithConstraints(	double dflowmax,
	double vMax, double vonNeumanCon);
fluxData getFD4MaxValues(cvatt cell, cvatt wcell, cvatt ncell);
fluxData getFluxToEastOrSouthUsing1DArray(cvatt curCell,
	cvatt tarCell, int targetCellDir);
fluxData getFluxUsingFluxLimitBetweenTwoCell(fluxData inflx, 
	double dflow, double dx, double dt_sec);
fluxData getFluxUsingSubCriticalCon(fluxData inflx,
	double gravity, double froudNCriteria);
double getVonNeumanConditionValue(cvatt cell);
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
//void makeImgFDofVMax();

int makeOutputFiles(double nowTsec);
fluxData noFlx();
int NRinner(int idx);

int openProjectFile();
int openPrjAndSetupModel();

int readRainfallAndGetIntensity(int rforder);
int readXmlRowProjectSettings(string aline);
int readXmlRowHydroPars(string aline);
int readXmlRowBoundaryConditionData(string aline,
	bcinfo* bci);
int readXmlRowDEMFileToChange(string aline, 
	demToChangeinfo* dc);

int runG2D();
int runSolverUsingGPU();
void runSolverUsingCPU();
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


