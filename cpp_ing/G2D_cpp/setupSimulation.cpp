#include <stdio.h>
#include <omp.h>
#include "g2d.h"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern domaininfo di;
extern projectFile prj;
extern bcCellinfo* bci;

extern globalVinner gvi[1];
extern thisProcessInner psi;;
extern thisProcess ps;

globalVinner initGlobalVinner()
{
	globalVinner gv;
	//gv.dt_sec = ge.dtStart_sec;
	gv.dx = di.dx;
	gv.nCols = di.nCols;
	gv.nRows = di.nRows;
	gv.nCellsInnerDomain = di.cellCountNotNull;
	gv.bcCellCountAll = prj.bcCellCountAll;
	gv.dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0f;
	gv.slpMinLimitforFlow = ge.slpMinLimitforFlow;
	gv.domainOutBedSlope = prj.domainOutBedSlope;
	gv.ConvgC_h = ge.convergenceConditionh;
	gv.froudeNCriteria = prj.froudeNumberCriteria;
	gv.iNRmax = prj.maxIterationACellOnCPU;
	gv.iGSmax = prj.maxIterationAllCellsOnCPU;
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
	gv.mdp = 1;
	if (prj.maxDegreeOfParallelism == -1) {
		gv.mdp = prj.cpusi.totalNumberOfLogicalProcessors;
	}
	else if (prj.maxDegreeOfParallelism > 0) {
		gv.mdp = prj.maxDegreeOfParallelism;
	}
	return gv;
}

void initilizeThisStep(float dt_sec, double nowt_sec, int bcdt_sec, int rfEnded)
{
	int nchunk;
	omp_set_num_threads(gvi[0].mdp);
	//prj.isParallel == 1 인 경우에는 gvi[0].mdp > 0 이 보장됨
	nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel for schedule(guided, nchunk) 
	for (int i = 0; i < gvi[0].nCellsInnerDomain; i++) {
		initializeThisStepAcell(i, dt_sec, bcdt_sec, nowt_sec, rfEnded);
	}
}

void initializeThisStepAcell(int idx, float dt_sec, int dtbc_sec, double nowt_sec, int rfEnded)
{
	double h = cvs[idx].dp_tp1 + cvs[idx].elez; //elev 가 변경되는 경우가 있으므로, 이렇게 수위설정
	if (cvs[idx].hp_tp1 <= h) {
		// dem  고도 변경되면, 수심이 바뀐다. 수위는 유지.
		// cvs[idx].hp_t=cvs[idx].elez + cvs[idx].dp_t 이므로, cvs[idx].dp_t 이값과 cvs[idx].dp_tp1  모두 업데이트 해줘야 한다.
		cvs[idx].dp_tp1 = cvs[idx].hp_tp1 - cvs[idx].elez;
		if (cvs[idx].dp_tp1 < 0) { cvs[idx].dp_tp1 = 0; }
		cvs[idx].dp_t = cvs[idx].dp_tp1;
	}
	else {
		cvs[idx].dp_t = cvs[idx].dp_tp1;
	}
	cvs[idx].qe_t = cvs[idx].qe_tp1;
	cvs[idx].qw_t = cvs[idx].qw_tp1;
	cvs[idx].qs_t = cvs[idx].qs_tp1;
	cvs[idx].qn_t = cvs[idx].qn_tp1;
	double sourceAlltoRoute_tp1_dt_m = 0.0;
	int bid = -1;
	if (prj.isbcApplied == 1) {
		bid = getbcCellArrayIndex(idx);
	}
	if (bid >= 0)// 현재의 idx에 bc 가 부여되어 있으면..
	{
		bci[bid].bcDepth_dt_m_tp1 = getConditionDataAsDepthWithLinear(bci[bid].bctype,
			cvs[idx].elez, gvi[0].dx, cvsAA[idx], psi.dt_sec, dtbc_sec, nowt_sec);
		if (bci[bid].bctype == 1)
		{//경계조건이 유량일 경우, 소스항에 넣어서 홍수추적한다. 수심으로 환산된 유량..
			sourceAlltoRoute_tp1_dt_m = bci[bid].bcDepth_dt_m_tp1;
		}
		else
		{//경계조건이 유량이 아닐경우, 홍수추적 하지 않고, 고정된 값 적용.
			cvs[idx].dp_tp1 = bci[bid].bcDepth_dt_m_tp1;
			if (ps.tnow_sec == 0) {
				cvs[idx].dp_t = cvs[idx].dp_tp1;
			}
		}
	}
	cvsAA[idx].sourceRFapp_dt_meter = 0;
	//-1:false, 1: true
	if (prj.isRainfallApplied == 1 && rfEnded == -1)
	{
		if (prj.rainfallDataType == rainfallDataType::TextFileASCgrid) {
			cvsAA[idx].sourceRFapp_dt_meter = cvsAA[idx].rfReadintensity_mPsec * dt_sec;
		}
		else {
			cvsAA[idx].rfReadintensity_mPsec = psi.rfReadintensityForMAP_mPsec;
			cvsAA[idx].sourceRFapp_dt_meter = psi.rfReadintensityForMAP_mPsec * dt_sec;
		}
	}
	sourceAlltoRoute_tp1_dt_m = sourceAlltoRoute_tp1_dt_m + cvsAA[idx].sourceRFapp_dt_meter;
	cvs[idx].dp_t = cvs[idx].dp_t + sourceAlltoRoute_tp1_dt_m;
	cvs[idx].dp_tp1 = cvs[idx].dp_tp1 + sourceAlltoRoute_tp1_dt_m;
	cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
	if (cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet) {
		setEffectiveCells(idx);
	}
}

int setGenEnv()
{
	ge.modelSetupIsNormal = 1;
	ge.gravity = 9.80665f; // 1;
	ge.dMinLimitforWet_ori = 0.000001f;
	// 0.00001;// 이게 0이면, 유량 계산시 수심으로 나누는 부분에서 발산. 유속이 크게 계산된다..
	   // 이 값은 1. 주변셀과의 흐름 계산을 할 셀(effective 셀) 결정시 사용되고,
	   //            2. 이 값보다 작은 셀은 이 셀에서 외부로의 유출은 없게 된다. 외부에서 이 셀로의 유입은 가능
	   //            3. 생성항(강우, 유량 등)에 의한 유량 추가는 가능하다.
	//slpMinLimitforFlow = 0.0001; //음해
	ge.slpMinLimitforFlow = 0;// 양해
	if (ge.isAnalyticSolution == 1) {
		ge.dtMaxLimit_sec = 2;// 600; //해석해 하고 비교할때는 1 이 아주 잘 맞는다..
		ge.dtMinLimit_sec = 1;// 0.1; 
		//ge.dtStart_sec = ge.dtMinLimit_sec;// 0.1;//1 ;
	}
	else {
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
	ge.convergenceConditionh = 0.00001;// 양해 0.00001;//0.00001; // 0.00000001; //
	ge.convergenceConditionhr = 0.001;// 양해 0.00001;//0.00001; // 0.00000001; //
	ge.convergenceConditionq = 0.0001;//0.0000001;//0.00001; //0.1% 
	return 1;
}

int setStartingConditionUsingCPU()
{
	int nchunk;
	omp_set_num_threads(gvi[0].mdp);
	//prj.isParallel == 1 인 경우에는 gvi[0].mdp > 0 이 보장됨
	nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel for schedule(guided, nchunk)
	for (int i = 0; i < gvi[0].nCellsInnerDomain; i++) {
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


