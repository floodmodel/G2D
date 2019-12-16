#include <stdio.h>
#include <omp.h>
#include "g2d.h"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern domaininfo di;
extern projectFile prj;
extern globalVinner gvi[1];


globalVinner initGlobalVinner()
{
	globalVinner gv;
	gv.dt_sec = ge.dtStart_sec;
	gv.dx = di.dx;
	gv.nCols = di.nCols;
	gv.nRows = di.nRows;
	gv.dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
	gv.slpMinLimitforFlow = ge.slpMinLimitforFlow;
	gv.domainOutBedSlope = prj.domainOutBedSlope;
	gv.ConvgC_h = ge.convergenceConditionh;
	gv.froudNCriteria = prj.froudeNumberCriteria;
	gv.iNRmax = ge.iNRmax;
	gv.iGSmax = ge.iGSmax;
	gv.gravity = ge.gravity;
	if (ge.isDWE == 1) {
		gv.isDWE = 1;
	}
	else {
		gv.isDWE = -1;
	}
	if (ge.isAnalyticSolution == 1) {
		gv.isAnalyticSolution = 1;
	}
	else {
		gv.isAnalyticSolution = -1;
	}
	if (prj.applyVNC == 1) {
		gv.isApplyVNC = 1;
	}
	else {
		gv.isApplyVNC = -1;
	}
	gv.mdp = 0;
	if (prj.isParallel == 1) {
		if (prj.maxDegreeOfParallelism == -1) {
			gv.mdp = prj.cpusi.totalNumberOfLogicalProcessors;
		}
		else if (prj.maxDegreeOfParallelism > 0) {
			gv.mdp = prj.maxDegreeOfParallelism;
		}
	}
	return gv;
}

void initilizeThisStep(float nowt_sec, int bcdt_sec, int rainfallisEnded)
{
	int nchunk;
	if (prj.isParallel == 1)	{
		omp_set_num_threads(gvi[0].mdp);
		//prj.isParallel == 1 �� ��쿡�� gvi[0].mdp > 0 �� �����
		nchunk = ge.cellCountNotNull / gvi[0].mdp;
	}
#pragma omp parallel for schedule(guided, nchunk) if(prj.isParallel)
	for (int i = 0; i < ge.cellCountNotNull; i++) {
		initializeAcellUsingArray(cvs, i, cvsadd, bcinfo, dt_sec, bcdt_sec, nowt_sec, rainfallisEnded);
	}
}

int setGenEnv()
{
	ge.modelSetupIsNormal = 1;
	ge.gravity = 9.80665f; // 1;
	ge.dMinLimitforWet_ori = 0.000001f;// 0.00001;// �̰� 0�̸�, ���� ���� �������� ������ �κп��� �߻�. ������ ũ�� ���ȴ�..
									// �� ���� 1. �ֺ������� �帧 ����� �� ��(effective ��) ������ ���ǰ�,
									//            2. �� ������ ���� ���� �� ������ �ܺη��� ������ ���� �ȴ�. �ܺο��� �� ������ ������ ����
									//            3. ������(����, ���� ��)�� ���� ���� �߰��� �����ϴ�.
	 //ge.dMinLimitforWet_ori;
	//slpMinLimitforFlow = 0.0001; //����
	ge.slpMinLimitforFlow = 0;// ����

	if (ge.isAnalyticSolution == 1)
	{
		ge.dtMaxLimit_sec = 2;// 600; //�ؼ��� �ϰ� ���Ҷ��� 1 �� ���� �� �´´�..
		ge.dtMinLimit_sec = 1;// 0.1; 
		//ge.dtStart_sec = ge.dtMinLimit_sec;// 0.1;//1 ;
	}
	else
	{
		ge.dtMaxLimit_sec = 30;// 600;
		ge.dtMinLimit_sec = 0.01f;
		//ge.dtStart_sec = ge.dtMinLimit_sec;
	}

	if (prj.isFixedDT == 1) {
		ge.dtStart_sec = prj.calculationTimeStep_sec;
	}
	else {
		ge.dtStart_sec = ge.dtMinLimit_sec;
	}
	ge.convergenceConditionh = 0.00001;// ���� 0.00001;//0.00001; // 0.00000001; //
	ge.convergenceConditionhr = 0.001;// ���� 0.00001;//0.00001; // 0.00000001; //
	ge.convergenceConditionq = 0.0001;//0.0000001;//0.00001; //0.1% 
	//maxDegreeParallelism = Environment.ProcessorCount * 2;
	ge.dflowmaxInThisStep = -9999;
	ge.vmaxInThisStep = -9999;
	ge.VNConMinInThisStep = DBL_MAX;

	ge.iNRmax = prj.maxIterationACellOnCPU;
	ge.iGSmax = prj.maxIterationAllCellsOnCPU;
	return 1;
}

int setStartingConditionUsingCPU()
{
	int nchunk;
	if (prj.isParallel == 1) {
		omp_set_num_threads(gvi[0].mdp);
		//prj.isParallel == 1 �� ��쿡�� gvi[0].mdp > 0 �� �����
		nchunk = ge.cellCountNotNull / gvi[0].mdp;
	}
#pragma omp parallel for schedule(guided, nchunk) if (prj.isParallel)
	for (int i = 0; i < ge.cellCountNotNull; i++) {
		//setStartingCondidtionInACell(cvs, i, cvsAA);
		cvs[i].dp_t = cvsAA[i].initialConditionDepth_m;
		cvs[i].dp_tp1 = cvs[i].dp_t;
		cvs[i].ve_tp1 = 0;
		cvs[i].qe_tp1 = 0;
		cvs[i].qw_tp1 = 0;
		cvs[i].qn_tp1 = 0;
		cvs[i].qs_tp1 = 0;
		cvs[i].hp_tp1 = cvs[i].dp_tp1 + cvs[i].elez;
		cvsAA[i].fdmax = 0;// N = 1, E = 4, S = 16, W = 64, NONE = 0
		cvsAA[i].bcData_curOrder = 0;
		cvsAA[i].sourceRFapp_dt_meter = 0;
		cvsAA[i].rfReadintensity_mPsec = 0;
		cvs[i].isSimulatingCell = -1;
	}
	return 1;
}


//void setStartingCondidtionInACell(cvatt *cvsL, int idx, cvattAdd* cvsaddL)
//{
//	cvsL[idx].dp_t = cvsaddL[idx].initialConditionDepth_m;
//	cvsL[idx].dp_tp1 = cvsL[idx].dp_t;
//	cvsL[idx].ve_tp1 = 0;
//	cvsL[idx].qe_tp1 = 0;
//	cvsL[idx].qw_tp1 = 0;
//	cvsL[idx].qn_tp1 = 0;
//	cvsL[idx].qs_tp1 = 0;
//	cvsL[idx].hp_tp1 = cvsL[idx].dp_tp1 + cvsL[idx].elez;
//	cvsaddL[idx].fdmax = 0;// N = 1, E = 4, S = 16, W = 64, NONE = 0
//	cvsaddL[idx].bcData_curOrder = 0;
//	cvsaddL[idx].sourceRFapp_dt_meter = 0;
//	cvsaddL[idx].rfReadintensity_mPsec = 0;
//	cvsL[idx].isSimulatingCell = -1;
//}


