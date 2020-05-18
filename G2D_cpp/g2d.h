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

#define GRAVITY		9.80665
#define slpMIN		0.0 //이거보다 작으면 경사가 없는 것이다. 
#define CCh			0.00001 // convergence condition h
#define isAS			0//analytic Solution ?,  0 : false, 1: true
#if isAS == 0			//해석해 모의가 아니면, 아래의 조건 적용
	#define dtMAX_sec	300.0
	#define dtMIN_sec	0.01
#else
//// 만일 해석해 모의(isAS==1) 이면, 아래의 조건이 잘 맞는다
	#define dtMAX_sec  2
	#define dtMIN_sec   1
#endif

#define isDWE			0//using dynamic wave equation ?  0 : false, 1: true
#define isVD			0// virtual domain?  0 : false, 1: true


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
{// 0 : false, 1: true
	int isSimulatingCell=0;  // 0 : false, 1: true
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
typedef struct _cvattAdd
{
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
	double dMinLimitforWet_ori = 0.000001; // 이거보다 같거나 작으면 마른 것이다.
	//double slpMinLimitforFlow = 0.0; //이거보다 작으면 경사가 없는 것이다. 
	double dtStart_sec=0.01;
	////const int movingDomain = 1;// -1 : false, 1: true
} generalEnv;

typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	// 0 : false, 1: true
	double dx = 0.0;
	int nCols = 0;
	int nRows = 0;
	int nCellsInnerDomain = 0;
	int bcCellCountAll = 0;
	double dMinLimitforWet = 0.0;
	double domainOutBedSlope = 0.0;
	double froudeNCriteria = 0.0;
	int iNRmaxLimit = 0;
	int iGSmaxLimit = 0;
	int isApplyVNC = 0;
	int mdp = 0;
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
	double dtbc_sec = 0.0;
	int rfEnded = 0;
	int effCellCount = 0;
	vector<int> FloodingCellCounts; // the number of cells that have water depth.
	vector<double> FloodingCellMeanDepth; //여기서 초기화하면 초기화 값이 push_back 된다.
	double FloodingCellMaxDepth = 0.0;
	vector<double> floodingCellDepthThresholds_m;
	COleDateTime simulationStartTime;
	COleDateTime thisPrintStepStartTime;
	int dt_printout_sec=0;
} thisProcess;

typedef struct _thisProcessInner
{
	double dt_sec = 0.0;
	int bAllConvergedInThisGSiteration=0;// 1:true, 0: false
	int iNRmax = 0;
	int iGSmax = 0;
	double maxResd = 0;
	int maxResdCVidx = -1;
	double dflowmaxInThisStep = 0.0; // courant number 계산용
	double vmaxInThisStep = 0.0;
	double VNConMinInThisStep = DBL_MAX;
	double rfReadintensityForMAP_mPsec = 0.0;
	int rfisGreaterThanZero = 1; // 1:true, 0: false
} thisProcessInner;

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

	int outputDepth = 0;// true : 1, false : 0
	int outputHeight = 0;// true : 1, false : 0	
	int outputVelocityMax = 0;// true : 1, false : 0	
	int outputFDofMaxV = 0;// true : 1, false : 0
	int outputDischargeMax = 0;// true : 1, false : 0	
	//int outputRFGrid = 0;// true : 1, false : -1

	double rendererMaxVdepthImg = 0.0;
	double rendererMaxVheightImg = 0.0;
	double rendererMaxVMaxVImg = 0.0;
	double rendererMaxVDischargeImg = 0.0;
	//double rfImgRendererMaxV = 0.0;

	int makeASCFile = 0; // true : 1, false : 0
	int makeImgFile = 0;// true : 1, false : 0
	int writeLog = 0;// true : 1, false : 0

	double roughnessCoeff = 0.0;
	double imperviousR = 0.0;
	double domainOutBedSlope = 0.0;

	int isicApplied = 0;// true : 1, false : 0
	conditionDataType icType = conditionDataType::NoneCD;
	fileOrConstant icDataType=fileOrConstant::None;
	string icFPN="";
	int usingicFile = 0;
	double icValue_m = 0.0; // ic는 height와 depth만 사용함
	double froudeNumberCriteria = 0.0;
	double courantNumber = 0.0;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : 0
	int bcCount = 0;
	int bcCellCountAll = 0;
	vector<int> bcCVidxList;
	vector<bcinfo> bcis;
	
	int isDEMtoChangeApplied = 0;// true : 1, false : 0	
	int DEMtoChangeCount = 0;
	vector<demToChangeinfo> dcs;

	CPUsInfo cpusi;

	string fpnTest_willbeDeleted="";
	string fpniterAcell_willbeDeleted="";
	string hvalues_Acell_willbeDeleted="";
} projectFile;


int calCEqUsingNRforCPU(int idx);
fluxData calMEq_DWE_Deterministric(double qt, 
	double dflow, double slp, double rc, 
	double dx, double dt_sec, double q_ip1, double u_ip1);
fluxData calMEq_DWEm_Deterministric(
	double qt, double dt_sec, double slp,
	double rc, double dflow, double qt_ip1);
void calEFlux(int idx);
void calNFlux(int idx);
void calSFlux(int idx);
void calWFlux(int idx);
int changeDomainElevWithDEMFile(double tnow_min, 
	double tbefore_min);
void checkEffCellNandSetAllFalse();
int deleteAlloutputFiles();
void disposeDynamicVars();

globalVinner initGlobalVinner();
int initializeOutputArray();
void initilizeThisStep();
void initializeThisStepAcell(int idx);
int isNormalBCinfo(bcinfo* bci);
int isNormalDEMtoChangeinfo(demToChangeinfo* dci);

void g2dHelp();
//int getbcCellArrayIndex(int cvid);
void getCellCD(int dataOrder, int dataInterval_min);
double getCDasDepthWithLinear(int bctype,
	double elev_m, double dx, int i_cvaa);
double getDTsecWithConstraints(	double dflowmax,
	double vMax, double vonNeumanCon);
fluxData getFD4MaxValues(int ip, int iw, int in);
fluxData getFD4MaxValues2(int i);
fluxData getFluxToEorS(int idxc,
	int idxt, int targetCellDir);
fluxData getFluxUsingFluxLimit(fluxData inflx, 
	double dflow, double dx, double dt_sec);
fluxData getFluxUsingSubCriticalCon(fluxData inflx,
	double froudNCriteria);
double getVNConditionValue(int i);
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
void setEffCells(int idx);
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


