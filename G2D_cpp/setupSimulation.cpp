#include "stdafx.h"
#include "g2d.h"
#include "g2d_cuda.cuh"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAddAtt* cvsAA;
extern double* cvsele;
extern double* rfi_read_mPs;
extern domaininfo di;
extern projectFile prj;
extern bcAppinfo* bcAppinfos;

extern globalVinner gvi;
extern thisProcessInner psi;
extern thisProcess ps;

extern minMaxCVidx mnMxCVidx;
extern dataForCalDT dataForDT;

void initThisProcess() {
	ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
	ps.simulationStartTime = COleDateTime::GetCurrentTime();
	ps.dt_printout_sec = (int)(prj.printOutInterval_min * 60);
	ps.mdp = prj.maxDegreeOfParallelism;
	ps.threadsPerBlock = prj.threadsPerBlock;
	ps.tTag_length = prj.tTag_length;
	psi.rfEnded = 1;
	psi.tsec_targetToprint = 0;
	psi.tnow_sec = 0;
	psi.effCellCount = 0;
	if (prj.isRainfallApplied == 1) {
		psi.rfEnded = 0;
	}
}

void initGlobalVinner(){
	gvi.dx = di.dx;
	gvi.dt_sec = ge.dtStart_sec;
	gvi.nCols = di.nCols;
	gvi.nRows = di.nRows;
	gvi.nCellsInnerDomain = di.cellNnotNull;
	gvi.bcCellCountAll = prj.bcCellCountAll;
	gvi.dtbc_sec = prj.bcDataInterval_min * 60;
	// 0보다 큰 강우가 하나라도 있으면...
	if (prj.isRainfallApplied == 1) {
		//gvi.dMinLimitforWet = ge.dMinLimitforWet_ori;
		gvi.isRFApplied = 1;
	}
	else {
		//강우가 없을때는 최소수심을 좀 크게 잡아도 된다.
		//gvi.dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
		gvi.isRFApplied = 0;
	}
	gvi.domainOutBedSlope = prj.domainOutBedSlope;
	gvi.froudeNCriteria = prj.froudeNumberCriteria;
	gvi.iNRmaxLimit = prj.maxIterationACellOnCPU; 
	gvi.iGSmaxLimit = prj.maxIterationAllCellsOnCPU;
	if (prj.applyVNC == 1) {
		gvi.isApplyVNC = 1;
	}
	else {
		gvi.isApplyVNC = 0;
	}
	dataForDT.applyVNC = prj.applyVNC;
	dataForDT.bcCellCountAll = prj.bcCellCountAll;
	dataForDT.bcDataInterval_min = prj.bcDataInterval_min;
	dataForDT.courantNumber = prj.courantNumber;
	dataForDT.printOutInterval_min = prj.printOutInterval_min;
	dataForDT.rainfallDataInterval_min = prj.rainfallDataInterval_min;
}

void initilizeThisStep_CPU()
{
	omp_set_num_threads(ps.mdp);
#pragma omp parallel for 
	for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
		initializeThisStepAcell(cvs, cvsAA, bcAppinfos, cvsele[i], rfi_read_mPs[i], i, psi, gvi);
		if (cvs[i].dp_tp1 > dMinLimit) {
			//if (cvs[i].dp_tp1 > gvi.dMinLimitforWet) {
			setEffCells(cvs, i);
		}
	}
}


int setGenEnv()
{
	ge.modelSetupIsNormal = 1;
	//ge.dMinLimitforWet_ori = 0.000001;
	// 0.00001;// 이게 0이면, 유량 계산시 수심으로 나누는 부분에서 발산. 유속이 크게 계산된다..
	   // 이 값은 1. 주변셀과의 흐름 계산을 할 셀(effective 셀) 결정시 사용되고,
	   //            2. 이 값보다 작은 셀은 이 셀에서 외부로의 유출은 없게 된다. 외부에서 이 셀로의 유입은 가능
	   //            3. 생성항(강우, 유량 등)에 의한 유량 추가는 가능하다.
	if (prj.isFixedDT == 1) {
		ge.dtStart_sec = prj.calculationTimeStep_sec;
	}
	else {
		ge.dtStart_sec = dtMIN_sec;
	}
	return 1;
}

void initFloodingThresholds() {
	ps.floodingCellDepthThresholds_m.clear();
	if (prj.floodingCellDepthThresholds_cm.size() < 1) {
		ps.floodingCellDepthThresholds_m.push_back(dMinLimit);
	}
	else {
		for (int i = 0; i < prj.floodingCellDepthThresholds_cm.size(); ++i) {
			double v = prj.floodingCellDepthThresholds_cm[i] / 100.0;
			ps.floodingCellDepthThresholds_m.push_back(v);
		}
	}
}

int setStartingConditionCVs_CPU(){
	omp_set_num_threads(ps.mdp);
#pragma omp parallel for schedule(guided)
	for (int i = 0; i < gvi.nCellsInnerDomain; i++) {
		setStartingConditionCVs_inner(cvs, cvsAA, cvsele, i);
	}
	return 1;
}

void updateMinMaxInThisStep_CPU()
{
	initMinMax();
	omp_set_num_threads(ps.mdp);
	int numThread = omp_get_max_threads();
	// 이건 배열 이용
	double* maxDflowL = new double[numThread];
	double* maxvL = new double[numThread];
	double* minvncL = new double[numThread];
	//cellResidual* maxResL = new cellResidual[numThread];
#pragma omp parallel
	{
		int nth = omp_get_thread_num();
		maxDflowL[nth] = -9999;
		maxvL[nth] = -9999;
		minvncL[nth] = 9999;
		//maxResL[nth].residual = 0;
		//maxResL[nth].cvidx = -1;
#pragma omp for schedule(guided)//, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				fluxData flxmax;
				flxmax = getFD4MaxValues(cvs, i);
				cvsAA[i].fdmaxV = flxmax.fd;
				cvsAA[i].vmax = flxmax.v;
				cvsAA[i].Qmax_cms = flxmax.q * gvi.dx;
				if (flxmax.dflow > maxDflowL[nth]) {
					maxDflowL[nth] = flxmax.dflow;
				}
				if (cvsAA[i].vmax > maxvL[nth]) {
					maxvL[nth] = cvsAA[i].vmax;
				}
				double vnCon = 0;
				if (gvi.isApplyVNC == 1) {
					vnCon = getVNConditionValue(cvs, i);
					if (vnCon < minvncL[nth]) {
						minvncL[nth] = vnCon;
					}
				}
				//if (cvs[i].resd > maxResL[nth].residual) {
				//	maxResL[nth].residual = cvs[i].resd;
				//	maxResL[nth].cvidx = i;
				//}
			}
		}
	}
	for (int i = 0; i < numThread; ++i) {
		if (mnMxCVidx.dflowmaxInThisStep < maxDflowL[i]) {
			mnMxCVidx.dflowmaxInThisStep = maxDflowL[i];
		}
		if (mnMxCVidx.vmaxInThisStep < maxvL[i]) {
			mnMxCVidx.vmaxInThisStep = maxvL[i];
		}
		if (mnMxCVidx.VNConMinInThisStep > minvncL[i]) {
			mnMxCVidx.VNConMinInThisStep = minvncL[i];
		}
		//if (mnMxCVidx.maxResd < maxResL[i].residual) {
		//	mnMxCVidx.maxResd = maxResL[i].residual;
		//	mnMxCVidx.maxResdCVidx = maxResL[i].cvidx;
		//}
	}
	delete[] maxDflowL;
	delete[] maxvL;
	delete[] minvncL;

	// 이건 critical 이용
//#pragma omp parallel
//	{
//		double maxDflowL = 0;
//		double maxvL = 0;
//		double minvncL = 9999;
//		cellResidual maxResL;
//		maxResL.residual = 0.0;
//#pragma omp for schedule(guided)//, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
//		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
//			if (cvs[i].isSimulatingCell == 1) {
//				fluxData flxmax;
//				flxmax = getFD4MaxValues(cvs, i);
//				cvsAA[i].fdmaxV = flxmax.fd;
//				cvsAA[i].vmax = flxmax.v;
//				cvsAA[i].Qmax_cms = flxmax.q * gvi.dx;
//				if (flxmax.dflow > maxDflowL) {
//					maxDflowL = flxmax.dflow;
//				}
//				if (cvsAA[i].vmax > maxvL) {
//					maxvL = cvsAA[i].vmax;
//				}
//				double vnCon = 0;
//				if (gvi.isApplyVNC == 1) {
//					vnCon = getVNConditionValue(cvs, i);
//					if (vnCon < minvncL) {
//						minvncL = vnCon;
//					}
//				}
//				if (cvs[i].resd > maxResL.residual) {
//					maxResL.residual = cvs[i].resd;
//					maxResL.cvidx = i;
//				}
//			}
//		}
//#pragma omp critical(updatePSv)
//		{
//			if (mnMxCVidx.dflowmaxInThisStep < maxDflowL) {
//				mnMxCVidx.dflowmaxInThisStep = maxDflowL;
//			}
//			if (mnMxCVidx.vmaxInThisStep < maxvL) {
//				mnMxCVidx.vmaxInThisStep = maxvL;
//			}
//			if (mnMxCVidx.VNConMinInThisStep > minvncL) {
//				mnMxCVidx.VNConMinInThisStep = minvncL;
//			}
//			if (mnMxCVidx.maxResd < maxResL.residual) {
//				mnMxCVidx.maxResd = maxResL.residual;
//				mnMxCVidx.maxResdCVidx = maxResL.cvidx;
//			}
//		}
//	}		
}

// parallel
void updateSummaryAndSetAllFalse() {
	psi.effCellCount = 0;
	ps.FloodingCellCounts.clear();
	ps.FloodingCellMeanDepth.clear();
	vector<double> FloodingCellSumDepth;
	ps.FloodingCellMaxDepth = 0;
	ps.maxResd = 0;
	ps.maxResdCVidx = -1;
	int nDepthClass = ps.floodingCellDepthThresholds_m.size();

	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
		ps.FloodingCellCounts.push_back(0);//0으로 초기화 한다.
		ps.FloodingCellMeanDepth.push_back(0);
		FloodingCellSumDepth.push_back(0);
	}
	omp_set_num_threads(ps.mdp);
	int* effCellCountL = new int[ps.mdp];
	double* maxDepthL = new double[ps.mdp];
	depthClassInfo* depthClassL = new depthClassInfo[ps.mdp];
	double* maxResdL = new double[ps.mdp];
	int* maxResdCVIDL = new int[ps.mdp];
#pragma omp parallel
	{
		int nth = omp_get_thread_num();
		effCellCountL[nth] = 0;
		maxDepthL[nth] = -9999;
		depthClassL[nth].floodingCellCount = new int[nDepthClass](); // 0 으로 초기화
		depthClassL[nth].floodingDepthSum = new double[nDepthClass]();
		maxResdL[nth] = -9999;
		maxResdCVIDL[nth] = -1;
#pragma omp for
		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				effCellCountL[nth]++;
				cvs[i].isSimulatingCell = 0;
				if (cvs[i].dp_tp1 > maxDepthL[nth]) {
					maxDepthL[nth] = cvs[i].dp_tp1;
				}
				for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
					if (cvs[i].dp_tp1 >= ps.floodingCellDepthThresholds_m[n]) {
						depthClassL[nth].floodingCellCount[n] ++;
						depthClassL[nth].floodingDepthSum[n] += cvs[i].dp_tp1;
					}
				}
				if (maxResdL[nth] < cvs[i].resd) {
					maxResdL[nth] = cvs[i].resd;
					maxResdCVIDL[nth] = i;
				}
			}
		}
	}
	// reduction
	for (int nth = 0; nth < ps.mdp; ++nth) {
		psi.effCellCount += effCellCountL[nth];
		if (ps.FloodingCellMaxDepth < maxDepthL[nth]) {
			ps.FloodingCellMaxDepth = maxDepthL[nth];
		}
		for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
			ps.FloodingCellCounts[n] += depthClassL[nth].floodingCellCount[n];
			FloodingCellSumDepth[n] += depthClassL[nth].floodingDepthSum[n];
		}
		if (ps.maxResd < maxResdL[nth]) {
			ps.maxResd = maxResdL[nth];
			ps.maxResdCVidx = maxResdCVIDL[nth];
		}
	}
	//==========================
	for (int n = 0; n < nDepthClass; ++n) {
		if (ps.FloodingCellCounts[n] > 0) {
			ps.FloodingCellMeanDepth[n] = FloodingCellSumDepth[n] / ps.FloodingCellCounts[n];
		}
	}

	delete[] effCellCountL;
	delete[] maxDepthL;
	if (depthClassL != NULL) {
			if (depthClassL->floodingCellCount != NULL) {
				delete[] depthClassL->floodingCellCount;
			}
			if (depthClassL->floodingDepthSum != NULL) {
				delete[] depthClassL->floodingDepthSum;
		}
		delete[] depthClassL;
	}
	delete[] maxResdL;
	delete[] maxResdCVIDL;
}

 //serial
void updateSummaryAndSetAllFalse_serial() {
	psi.effCellCount = 0;
	ps.FloodingCellCounts.clear();
	ps.FloodingCellMeanDepth.clear();
	vector<double> FloodingCellSumDepth;
	ps.FloodingCellMaxDepth = 0;
	ps.maxResd = 0;
	ps.maxResdCVidx = -1;
	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
		ps.FloodingCellCounts.push_back(0);//0으로 초기화 한다.
		ps.FloodingCellMeanDepth.push_back(0);
		FloodingCellSumDepth.push_back(0);
	}
	for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
		if (cvs[i].isSimulatingCell == 1) {
			psi.effCellCount += 1;
			if (cvs[i].dp_tp1 > ps.FloodingCellMaxDepth) {
				ps.FloodingCellMaxDepth = cvs[i].dp_tp1;
			}
			for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
				if (cvs[i].dp_tp1 >= ps.floodingCellDepthThresholds_m[n]) {
					ps.FloodingCellCounts[n] += 1;
					FloodingCellSumDepth[n] += cvs[i].dp_tp1;
				}
			}
			cvs[i].isSimulatingCell = 0;
			if (ps.maxResd < cvs[i].resd) {
				ps.maxResd = cvs[i].resd;
				ps.maxResdCVidx = i;
			}
		}
	}

	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); ++n) {
		if (ps.FloodingCellCounts[n] > 0) {
			ps.FloodingCellMeanDepth[n] = FloodingCellSumDepth[n] / ps.FloodingCellCounts[n];
		}
	}
}


inline void initMinMax() {
	mnMxCVidx.dflowmaxInThisStep = -9999;
	mnMxCVidx.vmaxInThisStep = -9999;
	mnMxCVidx.VNConMinInThisStep = 9999;
	//mnMxCVidx.maxResd = 0;
	//mnMxCVidx.maxResdCVidx = -1;
}
