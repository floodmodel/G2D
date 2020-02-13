#include <stdio.h>
//#include <fstream>
//#include <filesystem>
//#include <io.h>
#include <omp.h>

//#include "gentle.h"
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
	gv.dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
	gv.slpMinLimitforFlow = ge.slpMinLimitforFlow;
	gv.domainOutBedSlope = prj.domainOutBedSlope;
	gv.ConvgC_h = ge.convergenceConditionh;
	gv.froudeNCriteria = prj.froudeNumberCriteria;
	gv.iNRmax = prj.maxIterationACellOnCPU; //처음에는 cpu에 대한 값으로 설정
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

void initilizeThisStep(double dt_sec, double nowt_sec, int bcdt_sec, int rfEnded)
{
	//int nchunk;
	omp_set_num_threads(gvi[0].mdp);
	//prj.isParallel == 1 인 경우에는 gvi[0].mdp > 0 이 보장됨
	int nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel for schedule(guided)//, nchunk) 
	for (int i = 0; i < gvi[0].nCellsInnerDomain; i++) {
		initializeThisStepAcell(i, dt_sec, bcdt_sec, nowt_sec, rfEnded);
	}
}

void initializeThisStepAcell(int idx, double dt_sec, int dtbc_sec, double nowt_sec, int rfEnded)
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
	ge.gravity = 9.80665; // 1;
	ge.dMinLimitforWet_ori = 0.000001;
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
		ge.dtMaxLimit_sec = 300;// 600;
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
	return 1;
}

int setStartingConditionUsingCPU()
{
	ps.floodingCellDepthThresholds_m.clear();
	if (prj.floodingCellDepthThresholds_cm.size() < 1) {
		ps.floodingCellDepthThresholds_m.push_back(ge.dMinLimitforWet_ori);
	}
	else {
		for (int i = 0; i < prj.floodingCellDepthThresholds_cm.size(); ++i) {
			double v = prj.floodingCellDepthThresholds_cm[i] / 100.0;
			ps.floodingCellDepthThresholds_m.push_back(v);
		}
	}
	//int nchunk;
	omp_set_num_threads(gvi[0].mdp);
	//prj.isParallel == 1 인 경우에는 gvi[0].mdp > 0 이 보장됨
	//int nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel for schedule(guided)//, nchunk)
	for (int i = 0; i < gvi[0].nCellsInnerDomain; i++) {
		setStartingCondidtionInACell(i);
	}
	return 1;
}

void setStartingCondidtionInACell(int i)
{
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


void updateValuesInThisStepResults()
{
	psi.dflowmaxInThisStep = -9999;
	psi.vmaxInThisStep = -9999;
	psi.VNConMinInThisStep = DBL_MAX;
	psi.maxResd = 0;
#pragma omp parallel
	{
		double maxdflow = 0;
		double maxv = 0;
		double minvnc = 9999;
		cellResidual maxRes;
		maxRes.residual = 0.0;
		//int nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp for schedule(guided)//, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
		for (int idx = 0; idx < gvi[0].nCellsInnerDomain; ++idx)
		{
			if (cvs[idx].isSimulatingCell == 1)
			{
				fluxData flxmax;
				if (cvs[idx].cvaryNum_atW >= 0 && cvs[idx].cvaryNum_atN >= 0) {
					//  이경우는 4개 방향 성분에서 max 값 얻고
					flxmax = getFD4MaxValues(cvs[idx],
						cvs[cvs[idx].cvaryNum_atW],
						cvs[cvs[idx].cvaryNum_atN]);
				}
				else if (cvs[idx].cvaryNum_atW >= 0 && cvs[idx].cvaryNum_atN < 0) {
					flxmax = getFD4MaxValues(cvs[idx],
						cvs[cvs[idx].cvaryNum_atW], cvs[idx]);
				}
				else  if (cvs[idx].cvaryNum_atW < 0 && cvs[idx].cvaryNum_atN >= 0) {
					flxmax = getFD4MaxValues(cvs[idx],
						cvs[idx], cvs[cvs[idx].cvaryNum_atN]);
				}
				else {//w, n에 셀이 없는 경우
					flxmax = getFD4MaxValues(cvs[idx], cvs[idx], cvs[idx]);
				}
				cvsAA[idx].fdmax = flxmax.fd;
				cvsAA[idx].vmax = flxmax.v;
				cvsAA[idx].Qmax_cms = flxmax.q * gvi[0].dx;
				if (flxmax.dflow > maxdflow) {
					maxdflow = flxmax.dflow;
				}
				if (cvsAA[idx].vmax > maxv) {
					maxv = cvsAA[idx].vmax;
				}
				double vnCon = 0;
				if (gvi[0].isApplyVNC == 1) {
					vnCon = getVonNeumanConditionValue(cvs[idx]);
				}
				if (vnCon < minvnc) {
					minvnc = vnCon;
				}
				if (cvs[idx].resd > maxRes.residual) {
					maxRes.residual = cvs[idx].resd;
					maxRes.cvid = idx;
				}
			}
		}
#pragma omp critical
		{
			if (psi.dflowmaxInThisStep < maxdflow) {
				psi.dflowmaxInThisStep = maxdflow;
			}
			if (psi.vmaxInThisStep < maxv) {
				psi.vmaxInThisStep = maxv;
			}
			if (psi.VNConMinInThisStep > minvnc) {
				psi.VNConMinInThisStep = minvnc;
			}
			if (psi.maxResd < maxRes.residual) {
				psi.maxResd = maxRes.residual;
				psi.maxResdCVID = maxRes.cvid;
			}
		}
	}
}


fluxData getFD4MaxValues(cvatt cell, cvatt wcell, cvatt ncell)
{
	fluxData flxmax;
	double vw = abs(wcell.ve_tp1);
	double ve = abs(cell.ve_tp1);
	double vn = abs(ncell.vs_tp1);
	double vs = abs(cell.vs_tp1);
	double vmaxX = max(vw, ve);
	double vmaxY = max(vn, vs);
	double vmax = max(vmaxX, vmaxY);
	if (vmax == 0) {
		flxmax.fd = 0;// cVars.FlowDirection4.NONE;
		flxmax.v = 0;
		flxmax.dflow = 0;
		flxmax.q = 0;
		return flxmax;
	}
	else {
		flxmax.v = vmax;//E = 1, S = 3, W = 5, N = 7, NONE = 0
		if (vmax == vw) {
			flxmax.fd = 5;
		}
		else if (vmax == ve) {
			flxmax.fd = 1;
		}
		else if (vmax == vn) {
			flxmax.fd = 7;
		}
		else if (vmax == vs) {
			flxmax.fd = 3;
		}
	}
	double dmaxX = max(wcell.dfe, cell.dfe);
	double dmaxY = max(ncell.dfs, cell.dfs);
	flxmax.dflow = max(dmaxX, dmaxY);
	double qw = abs(wcell.qe_tp1);
	double qe = abs(cell.qe_tp1);
	double qn = abs(ncell.qs_tp1);
	double qs = abs(cell.qs_tp1);
	double qmaxX = max(qw, qe);
	double qmaxY = max(qn, qs);
	flxmax.q = max(qmaxX, qmaxY);
	return flxmax;
}

double getVonNeumanConditionValue(cvatt cell)
{
	double searchMIN = DBL_MAX;
	double curValue = 0;
	double rc = cell.rc;
	// e 값과 중복되므로, w는 계산하지 않는다.
	if (cell.dfe > 0) {
		curValue = 2 * rc * sqrt(abs(cell.slpe))
			/ pow(cell.dfe, 5.0 / 3.0);
		if (curValue < searchMIN) {
			searchMIN = curValue;
		}
	}
	// s 값과 중복되므로, n는 계산하지 않는다.
	if (cell.dfs > 0) {
		curValue = 2 * rc * sqrt(abs(cell.slps))
			/ pow(cell.dfs, 5.0 / 3.0);
		if (curValue < searchMIN) {
			searchMIN = curValue;
		}
	}
	return searchMIN;
}

void checkEffetiveCellNumberAndSetAllFlase()
{
	ps.effCellCount = 0;
	ps.FloodingCellCounts.clear();// = new vector<int>();
	//cThisProcess.FloodingCellMaxDepth = new List<double>();
	ps.FloodingCellMeanDepth.clear();// = new List<double>();
	vector<double> FloodingCellSumDepth;
	ps.FloodingCellMaxDepth = 0;
	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
		ps.FloodingCellCounts.push_back(0);//0개씩으로 초기화 한다.
		ps.FloodingCellMeanDepth.push_back(0);
		FloodingCellSumDepth.push_back(0);
	}
	for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
		if (cvs[i].isSimulatingCell == 1) {
			ps.effCellCount += 1;
			if (cvs[i].dp_tp1 > ps.FloodingCellMaxDepth) {
				ps.FloodingCellMaxDepth = cvs[i].dp_tp1;
			}
		}
		cvs[i].isSimulatingCell = -1;
		for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
			if (cvs[i].dp_tp1 >= ps.floodingCellDepthThresholds_m[n]) {
				ps.FloodingCellCounts[n] += 1;
				FloodingCellSumDepth[n] += cvs[i].dp_tp1;
			}
		}
	}
	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
		if (ps.FloodingCellCounts[n] > 0) {
			ps.FloodingCellMeanDepth[n] = FloodingCellSumDepth[n] / ps.FloodingCellCounts[n];
		}
	}
}


double getDTsecWithConstraints(double dflowmax, double vMax, double vonNeumanCon)
{
	double dtsecCFL = 0.0;
	double dtsecCFLusingDepth = 0.0;
	double dtsecCFLusingV = 0.0;
	double half_dtPrint_sec = prj.printOutInterval_min * 30.0;
	double half_bcdt_sec = prj.bcDataInterval_min * 30.0;
	double half_rfdt_sec = prj.rainfallDataInterval_min * 30.0;
	//==================================
	//이건 cfl 조건
	if (dflowmax > 0) {
		dtsecCFLusingDepth = prj.courantNumber * di.dx / sqrt(gvi[0].gravity * dflowmax);
		//  아래  것과 결과에 별 차이 없다..
		//   dtsecCFL = cfln * dm.dx / Math.Sqrt(gravity * depthMax);
		dtsecCFL = dtsecCFLusingDepth;
	}
	if (vMax > 0) {
		dtsecCFLusingV = prj.courantNumber * di.dx / vMax;
		dtsecCFL = dtsecCFLusingV;
	}
	if (dtsecCFLusingDepth > 0 && dtsecCFLusingV > 0) {
		dtsecCFL = min(dtsecCFLusingDepth, dtsecCFLusingV);
	}
	//==================================
	//==================================
	//이건 Von Neuman 안정성 조건
	double dtsecVN = 0;
	if (prj.applyVNC == 1) {
		dtsecVN = (vonNeumanCon * di.dx * di.dx) / 4;
	}
	double dtsec = 0;
	if (dtsecVN > 0 && dtsecCFL > 0) { dtsec = min(dtsecCFL, dtsecVN); }
	else { dtsec = max(dtsecCFL, dtsecVN); }
	//===================================
	if (dtsec > half_dtPrint_sec) { dtsec = half_dtPrint_sec; }
	if (half_bcdt_sec > 0 && dtsec > half_bcdt_sec) { dtsec = half_bcdt_sec; } //bc가 적용되지 않으면 half_bcdt_sec=0
	if (half_rfdt_sec > 0 && dtsec > half_rfdt_sec) { dtsec = half_rfdt_sec; }  //rf가 적용되지 않으면, half_rfdt_sec=0
	if (dtsec == 0) {
		dtsec = psi.dt_sec * 1.5;
		if (dtsec > ge.dtMaxLimit_sec) { dtsec = ge.dtMaxLimit_sec; }
	}
	double maxSourceDepth = 0;
	double dtsecCFLusingBC = 0;
	int bcdt_sec = prj.bcDataInterval_min * 60;
	for (int ib = 0; ib < prj.bcCount; ib++) {
		double bcDepth_dt_m_tp1 = 0;
		int cvidx = bci[ib].cvid;
		bcDepth_dt_m_tp1 = getConditionDataAsDepthWithLinear(bci[ib].bctype,
			cvs[cvidx].elez, di.dx, cvsAA[cvidx], dtsec, bcdt_sec, ps.tnow_sec);
		if (bcDepth_dt_m_tp1 > maxSourceDepth) { maxSourceDepth = bcDepth_dt_m_tp1; }
	}
	if (maxSourceDepth > 0) {
		dtsecCFLusingBC = prj.courantNumber * di.dx / sqrt(ge.gravity * (maxSourceDepth + dflowmax));
		if (dtsecCFLusingBC < dtsec) { dtsec = dtsecCFLusingBC; }
	}
	if (dtsec < ge.dtMinLimit_sec) { dtsec = ge.dtMinLimit_sec; }
	if (dtsec > ge.dtMaxLimit_sec) { dtsec = ge.dtMaxLimit_sec; }
	//if (dtsec > 5) {
	//	double intpart;
	//	double realpart_t = modf(ps.tnow_sec, &intpart);
	//	double fpart = modf(dtsec, &intpart); //dtsec를 정수로 만들고
	//	dtsec = intpart;
	//	dtsec = dtsec - realpart_t;  // 이렇게 하면 t+dt가 정수가 된다.
	//}
	return dtsec;
}


