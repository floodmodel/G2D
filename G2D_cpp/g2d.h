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


typedef struct _bcCellinfo
{
	int cvid = 0;
	double bcDepth_dt_m_tp1 = 0.0;
	int bctype = 0; //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
} bcCellinfo;

typedef struct _cvatt
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

	double dp_tp1 = 0.0;  // ���� ���� ���� t+dt
	double dp_t = 0.0; //���� ���� ����
	double hp_tp1 = 0.0;//z+d

	double dfe = 0.0; //e���� �帧����
	double dfs = 0.0; //s���� �帧����

	double ve_tp1 = 0.0;
	double vs_tp1 = 0.0;
	/// <summary>
	/// water surface slope. dh/dx���� ������ i+1. �׷��Ƿ�, +�� i ���� ������ �� ���ٴ� �ǹ�
	/// </summary>
	double slpe = 0.0;

	/// <summary>
	/// water surface slope. dh/dx���� ������ i+1. �׷��Ƿ�, +�� i ���� ������ �� ���ٴ� �ǹ�
	/// </summary>
	double slps = 0.0;

	double qe_tp1 = 0.0;
	double qw_tp1 = 0.0; //w ��輿�� ������ ���⿡ �����ؾ� �Ѵ�. w���� e ������ ����.
	double qs_tp1 = 0.0;
	double qn_tp1 = 0.0; //n ��輿�� ������ ���⿡ �����ؾ� �Ѵ�. n���� s ������ ����.

	double qe_t = 0.0;
	double qw_t = 0.0;
	double qs_t = 0.0;
	double qn_t = 0.0;

	double resd=0.0; //residual
} cvatt;


//GPU parameter �� �ѱ�� �Ű������� �ּ�ȭ �ϱ� ���ؼ� �̰��� �߰��� ����Ѵ�. ���⿡ ���Ե� ���� gpu�� �ȳѱ��.
typedef struct _cvattAdd
{// -1 : false, 1: true
	//int cvid;
	double rfReadintensity_mPsec = 0.0;
	double sourceRFapp_dt_meter = 0.0;
	double bcData_curOrder = 0.0;
	double bcData_nextOrder = 0.0;
	int bcData_curOrderStartedTime_sec = 0;
	double initialConditionDepth_m = 0.0;

	/// <summary>
	/// cms
	/// </summary>
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
	double gravity= 9.80665;
	double dMinLimitforWet_ori = 0.000001; // �̰ź��� ���ų� ������ ���� ���̴�.
	double slpMinLimitforFlow = 0.0; //�̰ź��� ������ ��簡 ���� ���̴�. 
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

typedef struct _globalVinner // ��� ������ �����ϱ� ���� �ּ����� ���� ����. gpu ����
{
	// -1 : false, 1: true
	double dx = 0.0;
	int nCols = 0;
	int nRows = 0;
	int nCellsInnerDomain = 0;
	int bcCellCountAll = 0;
	//int isparallel = 1;
	double dMinLimitforWet = 0.0;
	//double dMinLimitforWet_ori = 0.0;
	double slpMinLimitforFlow = 0.0;
	double domainOutBedSlope = 0.0;
	double ConvgC_h = 0.0;
	double froudeNCriteria = 0.0;
	int iNRmaxLimit = 0;
	int iGSmaxLimit = 0;
	//int iNR = 0;
	//int iGS = 0;
	double gravity = 0.0;
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
	//double dt_sec=0.0;
	//int isfixeddt = 0;// -1 : false, 1: true
	//int isparallel = 0;// -1 : false, 1: true
	double tsec_targetToprint = 0.0;
	double tnow_min = 0.0;
	double tnow_sec = 0.0;
	int effCellCount = 0;
	vector<int> FloodingCellCounts; // the number of cells that have water depth.
	vector<double> FloodingCellMeanDepth; //���⼭ �ʱ�ȭ�ϸ� �ʱ�ȭ ���� push_back �ȴ�.
	double FloodingCellMaxDepth = 0.0;
	vector<double> floodingCellDepthThresholds_m;
	COleDateTime simulationStartTime;
	COleDateTime thisPrintStepStartTime;
	//double dt_printout_min = 0.0;
	int dt_printout_sec=0;
} thisProcess;

typedef struct _thisProcessInner
{
	//double* subregionVmax;
	//double* subregionDflowmax;
	//double* subregionVNCmin;
	double dt_sec = 0.0;
	int bAllConvergedInThisGSiteration=-1;// 1:true, -1: false
	//int maxNR_inME = 0;

	int iNRmax = 0;
	int iGSmax = 0;
	double maxResd = 0;
	int maxResdCVID = -1;
	//int maxResdCellxCol =0;
	//int maxResdCellyRow = 0;
	//double* subregionMaxResd;
	//string* subregionMaxResdCell;
	double dflowmaxInThisStep = 0.0; // courant number ����
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
	//int isParallel=0;// true : 1, false : 0
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
	string startDateTime=""; // ������� �Է� ������  2017-11-28 23:10 ���� ���
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
	double icValue_m = 0.0; // ic�� height�� depth�� �����
	double froudeNumberCriteria = 0.0;
	double courantNumber = 0.0;
	int applyVNC = 0;

	int isbcApplied = 0;// true : 1, false : -1
	vector<vector<cellPosition>> bcCellXY; // �ϳ��� bc�� �������� ���� ������ �� �ִ�.
	//map <int, vector<cellPosition> bcCellXY;
	vector<string> bcDataFile;
	vector<conditionDataType> bcDataType;
	vector<vector<double>> bcValues;
	int bcCount = 0;
	int bcCellCountAll = 0;

	int isDEMtoChangeApplied = 0;// true : 1, false : -1
	vector<double> timeToChangeDEM_min;
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
	const string StartDateTime = "StartDateTime"; // ������� �Է� ������  2017-11-28 23:10 ���� ���
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
void initilizeThisStep(double dt_sec, double nowt_sec, int bcdt_sec, int rfEnded);
void initializeThisStepAcell(int idx, double dt_sec, int dtbc_sec, double nowt_sec, int rfEnded);
void g2dHelp();
int getbcCellArrayIndex(int cvid);
void getCellConditionData(int dataOrder, int dataInterval_min);
double getConditionDataAsDepthWithLinear(int bctype, double elev_m,
	double dx, cvattAdd cvaa, double dtsec,
	int dtsec_cdata, double nowt_sec);
double getDTsecWithConstraints(	double dflowmax, double vMax, 
	double vonNeumanCon);
fluxData getFD4MaxValues(cvatt cell, cvatt wcell, cvatt ncell);
fluxData getFluxToEastOrSouthUsing1DArray(cvatt curCell,
	cvatt tarCell, int targetCellDir);
fluxData getFluxUsingFluxLimitBetweenTwoCell(fluxData inflx, double dflow,
	double dx, double dt_sec);
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
int NRinner(int idx, int isBCCell, double dbdtpth, int bctype);
int openProjectFile();
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



//int changeDomainElevWithDEMFileUsingArray(string demfpn, domaininfo indm, domainCell **indmcells, cvatt *incvs); �̰� prj open �Ҷ� ������

