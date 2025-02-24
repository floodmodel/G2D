#include "stdafx.h"
#include "g2d.h"
#include "g2d_cuda.cuh"

//using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAddAtt* cvsAA;
extern cvattMaxValueAndFD* cvsMVnFD;
extern double* cvsele;
extern double* rfi_read_mPs;
extern domaininfo di;
extern projectFile prj;
extern bcAppinfo* bcAppinfos;

extern globalVinner gvi;
extern thisProcessInner psi;
extern thisProcess ps;

extern minMaxFlux mnMxFluxFromAllcells;
extern dataForCalDT dataForDT;

void initThisProcess() {
	ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
	ps.simulationStartTime = COleDateTime::GetCurrentTime();
	ps.dt_printout_sec = (int)(prj.printOutInterval_min * 60);
	ps.mdp = prj.maxDegreeOfParallelism;
	ps.threadsPerBlock = prj.threadsPerBlock;
	ps.tTag_length = prj.tTag_length;
	psi.rfEnded = 1;
	psi.rfAccMAP = 0.0;
	psi.saturatedByMAP = 0;
	psi.tsec_targetToprint = 0;
	psi.tnow_sec = 0;
	psi.effCellCount = 0;
	if (prj.isRainfallApplied == 1) {
		psi.rfEnded = 0;
		psi.isRFApplied = 1;
		psi.rfType = prj.rainfallDataType;
	}
	else {
		psi.isRFApplied = 0;
		psi.rfType = weatherDataType::None;
	}
}

void initGlobalVinner(){
	gvi.dx = di.dx;
	gvi.dy = di.dy;
	gvi.min_dx_dy = di.dx;
	if (di.dx > di.dy) {
		gvi.min_dx_dy = di.dy;
	}
	gvi.dt_sec = ge.dtStart_sec;
	gvi.nCols = di.nCols;
	gvi.nRows = di.nRows;
	gvi.nCellsInnerDomain = di.cellNnotNull;
	gvi.bcCellCountAll = prj.bcCellCountAll;
	gvi.dtbc_sec = prj.bcDataInterval_min * 60.0;
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
			setEffCells(cvs, i);
		}
	}
}


int setGenEnv()
{
	ge.modelSetupIsNormal = 1;
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

void updateGlobalMinMaxInThisStep_CPU()
{
	initMinMax();
	omp_set_num_threads(ps.mdp);
	int numThread = omp_get_max_threads();
	// 이건 배열 이용
	//double* maxDflowL = new double[numThread];
	double* maxDflowL = new double[numThread]();// 이렇게 하면 0 으로 초기화 된다. 2022.09.06
	double* maxvL = new double[numThread]();
	double* minvncL = new double[numThread]();
#pragma omp parallel
	{
		int nth = omp_get_thread_num();
#pragma omp for schedule(guided)//, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				fluxNfd flxmax;
				flxmax = get_maxFlux_FD(cvs, i); // 셀별 max 값을 찾아서 global max 찾는데 이용
				if (flxmax.dflow > maxDflowL[nth]) {
					maxDflowL[nth] = flxmax.dflow;
				}
				if (flxmax.v > maxvL[nth]) {
					maxvL[nth] = flxmax.v;
				}
				double vnCon = 0;
				if (gvi.isApplyVNC == 1) {
					vnCon = getVNConditionValue(cvs, i);
					if (vnCon < minvncL[nth]) {
						minvncL[nth] = vnCon;
					}
				}
			}
		}
	}
	for (int i = 0; i < numThread; ++i) {
		if (mnMxFluxFromAllcells.dflowmaxInThisStep < maxDflowL[i]) {
			mnMxFluxFromAllcells.dflowmaxInThisStep = maxDflowL[i];
		}
		if (mnMxFluxFromAllcells.vmaxInThisStep < maxvL[i]) {
			mnMxFluxFromAllcells.vmaxInThisStep = maxvL[i];
		}
		if (mnMxFluxFromAllcells.VNConMinInThisStep > minvncL[i]) {
			mnMxFluxFromAllcells.VNConMinInThisStep = minvncL[i];
		}
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



void updateGlobalMinMaxInThisStep_CPU_serial()
{
	initMinMax();
	for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
		if (cvs[i].isSimulatingCell == 1) {
			fluxNfd flxmax;
			flxmax = get_maxFlux_FD(cvs, i); // 셀별 max 값을 찾아서 global max 찾는데 이용
			if (flxmax.dflow > mnMxFluxFromAllcells.dflowmaxInThisStep) {
				mnMxFluxFromAllcells.dflowmaxInThisStep = flxmax.dflow;
			}
			if (flxmax.v > mnMxFluxFromAllcells.vmaxInThisStep) {
				mnMxFluxFromAllcells.vmaxInThisStep = flxmax.v;
			}
			double vnCon = 0;
			if (gvi.isApplyVNC == 1) {
				vnCon = getVNConditionValue(cvs, i);
				if (vnCon < mnMxFluxFromAllcells.VNConMinInThisStep) {
					mnMxFluxFromAllcells.VNConMinInThisStep = vnCon;
				}
			}
		}
	}
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
	int nDepthClass = (int)ps.floodingCellDepthThresholds_m.size();

	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
		ps.FloodingCellCounts.push_back(0);//0으로 초기화 한다.
		ps.FloodingCellMeanDepth.push_back(0);
		FloodingCellSumDepth.push_back(0);
	}
	omp_set_num_threads(ps.mdp);
	int* effCellCountL = new int[ps.mdp]();// 이렇게 하면 0 으로 초기화 된다. 2022.09.06
	double* maxDepthL = new double[ps.mdp]();
	depthClassInfo* depthClassL = new depthClassInfo[ps.mdp];
	double* maxResdL = new double[ps.mdp]();
	int* maxResdCVIDL = new int[ps.mdp]; // cvid는 0부터 시작하므로, 0으로 초기화 하면 안된다. 
#pragma omp parallel
	{
		int nth = omp_get_thread_num();
		//effCellCountL[nth] = 0;  // 2022.09.06
		//maxDepthL[nth] = -9999;
		depthClassL[nth].floodingCellCount = new int[nDepthClass](); // 0 으로 초기화
		depthClassL[nth].floodingDepthSum = new double[nDepthClass]();
		//maxResdL[nth] = -9999;
		maxResdCVIDL[nth] = -1; // cvid는 0부터 시작하므로, 0으로 초기화 하면 안된다. 
#pragma omp for
		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				fluxNfd flxMax;
				flxMax=get_maxFlux_FD(cvs, i);

				cvsMVnFD[i].vmax = flxMax.v;
				cvsMVnFD[i].fdmaxV = flxMax.fd_maxv;
				if (flxMax.fd_maxv == 1 || flxMax.fd_maxv == 5) {
					cvsMVnFD[i].Qmax_cms = flxMax.q * gvi.dx;
				}
				else {
					cvsMVnFD[i].Qmax_cms = flxMax.q * gvi.dy;
				}

				cvsMVnFD[i].fdmaxQ = flxMax.fd_maxq;

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
			fluxNfd flxMax;
			flxMax = get_maxFlux_FD(cvs, i);
			cvsMVnFD[i].vmax = flxMax.v;
			if (flxMax.fd_maxv == 1 || flxMax.fd_maxv == 5) {
				cvsMVnFD[i].Qmax_cms = flxMax.q * gvi.dx;
			}
			else {
				cvsMVnFD[i].Qmax_cms = flxMax.q * gvi.dy;
			}
			cvsMVnFD[i].fdmaxV = flxMax.fd_maxv;
			cvsMVnFD[i].fdmaxQ = flxMax.fd_maxq;
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
	mnMxFluxFromAllcells.dflowmaxInThisStep = -9999;
	mnMxFluxFromAllcells.vmaxInThisStep = -9999;
	mnMxFluxFromAllcells.VNConMinInThisStep = 9999;
}
