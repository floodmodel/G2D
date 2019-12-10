#include <stdio.h>
#include <omp.h>
#include "g2d.h"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern domaininfo di;
extern projectFile prj;


globalVinner initGlobalVinner()
{
	globalVinner gv;
	gv.dt_sec = ge.dtStart_sec;
	gv.dx = di.dx;
	gv.nCols = di.nCols;
	gv.nRows = di.nRows;
	gv.slpMinLimitforFlow = ge.slpMinLimitforFlow;
	gv.domainOutBedSlope = prj.domainOutBedSlope;
	gv.ConvgC_h = ge.convergenceConditionh;
	gv.froudNCriteria = prj.froudeNumberCriteria;
	gv.iNRmax = ge.iNRmax;
	gv.iGSmax = ge.iGSmax;
	gv.gravity = ge.gravity;
	if (ge.isDWE == 1)
	{
		gv.isDWE = 1;
	}
	else
	{
		gv.isDWE = -1;
	}
	if (ge.isAnalyticSolution == 1)
	{
		gv.isAnalyticSolution = 1;
	}
	else
	{
		gv.isAnalyticSolution = -1;
	}
	if (prj.applyVNC == 1)
	{
		gv.isApplyVNC = 1;
	}
	else
	{
		gv.isApplyVNC = -1;
	}
	return gv;
}

int setGenEnv()
{
	ge.modelSetupIsNormal = 1;
	ge.gravity = 9.80665; // 1;
	ge.dMinLimitforWet_ori = 0.000001;// 0.00001;// 이게 0이면, 유량 계산시 수심으로 나누는 부분에서 발산. 유속이 크게 계산된다..
									// 이 값은 1. 주변셀과의 흐름 계산을 할 셀(effective 셀) 결정시 사용되고,
									//            2. 이 값보다 작은 셀은 이 셀에서 외부로의 유출은 없게 된다. 외부에서 이 셀로의 유입은 가능
									//            3. 생성항(강우, 유량 등)에 의한 유량 추가는 가능하다.
	ge.dMinLimitforWet = ge.dMinLimitforWet_ori;
	//slpMinLimitforFlow = 0.0001; //음해
	ge.slpMinLimitforFlow = 0;// 양해

	if (ge.isAnalyticSolution == 1)
	{
		ge.dtMaxLimit_sec = 2;// 600; //해석해 하고 비교할때는 1 이 아주 잘 맞는다..
		ge.dtMinLimit_sec = 1;// 0.1; 
		//ge.dtStart_sec = ge.dtMinLimit_sec;// 0.1;//1 ;
	}
	else
	{
		ge.dtMaxLimit_sec = 30;// 600;
		ge.dtMinLimit_sec = 0.01;
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
	if (prj.isParallel == 1) {
		if (prj.maxDegreeOfParallelism > 0) {
			omp_set_num_threads(prj.maxDegreeOfParallelism);
		}
#pragma omp parallel for
		for (int i = 0; i < ge.cellCountNotNull; i++) {
			setStartingCondidtionInACell(cvs, i, cvsAA);
		}
	}
	else {
		for (int i = 0; i < ge.cellCountNotNull; i++) {
			setStartingCondidtionInACell(cvs, i, cvsAA);
		}
	}
	return 1;
}


void setStartingCondidtionInACell(cvatt *cvsL, int idx, cvattAdd* cvsaddL)
{
	cvsL[idx].dp_t = cvsaddL[idx].initialConditionDepth_m;
	cvsL[idx].dp_tp1 = cvsL[idx].dp_t;
	cvsL[idx].ve_tp1 = 0;
	cvsL[idx].qe_tp1 = 0;
	cvsL[idx].qw_tp1 = 0;
	cvsL[idx].qn_tp1 = 0;
	cvsL[idx].qs_tp1 = 0;
	cvsL[idx].hp_tp1 = cvsL[idx].dp_tp1 + cvsL[idx].elez;
	cvsaddL[idx].fdmax = 0;// N = 1, E = 4, S = 16, W = 64, NONE = 0
	cvsaddL[idx].bcData_curOrder = 0;
	cvsaddL[idx].sourceRFapp_dt_meter = 0;
	cvsaddL[idx].rfReadintensity_mPsec = 0;
	cvsL[idx].isSimulatingCell = -1;
}