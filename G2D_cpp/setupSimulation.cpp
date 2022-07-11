#include "stdafx.h"
#include "g2d.h"
#include "g2d_cuda.cuh"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAddAtt* cvsAA;
extern cvattMaxValue* cvsMV;
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
	double* maxDflowL = new double[numThread];
	double* maxvL = new double[numThread];
	double* minvncL = new double[numThread];
#pragma omp parallel
	{
		int nth = omp_get_thread_num();
		maxDflowL[nth] = -9999;
		maxvL[nth] = -9999;
		minvncL[nth] = 9999;
#pragma omp for schedule(guided)//, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
		for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				flux flxmax;
				flxmax = getMaxValues(cvs, i);
				//cvsMV[i].fdmaxV = flxmax.fd_maxV;
				//cvsMV[i].vmax = flxmax.v;
				//cvsMV[i].Qmax_cms = flxmax.q * gvi.dx;
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
		if (mnMxCVidx.dflowmaxInThisStep < maxDflowL[i]) {
			mnMxCVidx.dflowmaxInThisStep = maxDflowL[i];
		}
		if (mnMxCVidx.vmaxInThisStep < maxvL[i]) {
			mnMxCVidx.vmaxInThisStep = maxvL[i];
		}
		if (mnMxCVidx.VNConMinInThisStep > minvncL[i]) {
			mnMxCVidx.VNConMinInThisStep = minvncL[i];
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
				fluxfd flxfd;
				flxfd = getMaxValuesAndFD(cvs, i);
				cvsMV[i].vmax = flxfd.v;
				cvsMV[i].Qmax_cms = flxfd.q * gvi.dx;
				cvsMV[i].fdmaxV = flxfd.fd_maxV;
				cvsMV[i].fdmaxQ = flxfd.fd_maxQ;

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

fluxfd getMaxValuesAndFD(cvatt* cvs_L, int i) {
	fluxfd flxFDmax;
	if (cvs_L[i].cvidx_atW >= 0 && cvs_L[i].cvidx_atN >= 0) {
		//  이경우는 4개 방향 성분에서 max 값 얻고
		flxFDmax = getMaxValuesAndFD_inner(cvs_L, i,
			cvs_L[i].cvidx_atW,
			cvs_L[i].cvidx_atN);
	}
	else if (cvs_L[i].cvidx_atW >= 0 && cvs_L[i].cvidx_atN < 0) {
		flxFDmax = getMaxValuesAndFD_inner(cvs_L, i,
			cvs_L[i].cvidx_atW, i);
	}
	else  if (cvs_L[i].cvidx_atW < 0 && cvs_L[i].cvidx_atN >= 0) {
		flxFDmax = getMaxValuesAndFD_inner(cvs_L, i,
			i, cvs_L[i].cvidx_atN);
	}
	else {//w, n에 셀이 없는 경우
		flxFDmax = getMaxValuesAndFD_inner(cvs_L, i, i, i);
	}
	return flxFDmax;
}

fluxfd getMaxValuesAndFD_inner(cvatt* cvs_L, int ip, int iw, int in)
{	// cell을 전달 받는 것 보다, index를 받아서 지역변수로 cell을 선언하는게 더 빠르다..2020.05.12
	fluxfd flxmxfd;
	cvatt wcell = cvs_L[iw];
	cvatt cell = cvs_L[ip];
	cvatt ncell = cvs_L[in];
	
	// 여기서부터 vmax, vmaxFD 계산
	fdir fdX = fdir::NONE;
	fdir fdY = fdir::NONE;
	fdir fd_maxV = fdir::NONE;
	// EnS: E와 S로 같은 flux로 흐르는 경우.. 등등등, 
	//EWSN은 4방향으로 같은 flux로 흐르는 경우
	// NONE = 0, //흐름 없음
	// E = 1, EN = 8, ES = 2, S = 3, SW = 4, W = 5, WN = 6, N = 7,
	//	EnW = 15, EnS = 13, EnN = 17, WnS = 53, WnN = 57, SnN = 37,
	//	EnSnN = 137, SnEnW = 315, WnSnN = 537, NnEnW = 715, EWSN = 1537
	double vmaxX = 0.0;
	double vmaxY = 0.0;
	double vmax = 0.0;

	double vw = fabs(wcell.ve_tp1);
	double ve = fabs(cell.ve_tp1);
	double vn = fabs(ncell.vs_tp1);
	double vs = fabs(cell.vs_tp1);
	// x 방향
	if (vw == ve) {
		vmaxX = vw;
		if (vw == 0.0) {
			fdX = fdir::NONE;
		}
		else {
			fdX = fdir::EnN;
		}
	}
	else {
		if (vw > ve) {
			vmaxX = vw;
			fdX = fdir::W;
		}
		else {
			vmaxX = ve;
			fdX = fdir::E;
		}
	}
	// y 방향
	if (vn == vs) {
		vmaxY = vn;
		if (vn == 0.0) {
			fdY = fdir::NONE;
		}
		else {
			fdY = fdir::SnN;
		}
	}
	else {
		if (vn > vs) {
			vmaxY = vn;
			fdY = fdir::N;
		}
		else {
			vmaxY = vs;
			fdY = fdir::S;
		}
	}
	// x, y 비교
	if (vmaxX == vmaxY) {
		vmax = vmaxX;
		switch (fdX)
		{
		case fdir::EnW:
			if (fdY == fdir::SnN) {
				fd_maxV = fdir::EWSN;
			}
			else if (fdY == fdir::S) {
				fd_maxV = fdir::SnEnW;
			}
			else if (fdY == fdir::N) {
				fd_maxV = fdir::NnEnW;
			}
			break;
		case fdir::E:
			if (fdY == fdir::SnN) {
				fd_maxV = fdir::EnSnN;
			}
			else if (fdY == fdir::S) {
				fd_maxV = fdir::EnS;
			}
			else if (fdY == fdir::N) {
				fd_maxV = fdir::EnN;
			}
			break;
		case fdir::W:
			if (fdY == fdir::SnN) {
				fd_maxV = fdir::WnSnN;
			}
			else if (fdY == fdir::S) {
				fd_maxV = fdir::WnS;
			}
			else if (fdY == fdir::N) {
				fd_maxV = fdir::WnN;
			}
			break;
		}
	}
	else {
		if (vmaxX > vmaxY) {
			vmax = vmaxX;
			fd_maxV = fdX;
		}
		else {
			vmax = vmaxY;
			fd_maxV = fdY;
		}
	}
	flxmxfd.v = vmax;
	flxmxfd.fd_maxV = static_cast<int>(fd_maxV);
	if (vmax == 0) {
		flxmxfd.fd_maxV = 0;// cVars.FlowDirection4.NONE;
		flxmxfd.fd_maxQ = 0;
		flxmxfd.dflow = 0;
		flxmxfd.q = 0;
		return flxmxfd;
	}
	else {
		double dmaxX = fmax(wcell.dfe, cell.dfe);
		double dmaxY = fmax(ncell.dfs, cell.dfs);
		flxmxfd.dflow = fmax(dmaxX, dmaxY);
		// 여기서부터 qmax, qmaxFD 계산
		fdir fd_maxQ = fdir::NONE;
		fdX = fdir::NONE;
		fdY = fdir::NONE;
		double qmaxX = 0.0;
		double qmaxY = 0.0;
		double qmax = 0.0;
		double qw = fabs(wcell.qe_tp1);
		double qe = fabs(cell.qe_tp1);
		double qn = fabs(ncell.qs_tp1);
		double qs = fabs(cell.qs_tp1);
		// x 방향
		if (qw == qe) {
			qmaxX = qw;
			if (qw == 0.0) {
				fdX = fdir::NONE;
			}
			else {
				fdX = fdir::EnN;
			}
		}
		else {
			if (qw > qe) {
				qmaxX = qw;
				fdX = fdir::W;
			}
			else {
				qmaxX = qe;
				fdX = fdir::E;
			}
		}
		// y 방향
		if (qn == qs) {
			qmaxY = qn;
			if (qn == 0.0) {
				fdY = fdir::NONE;
			}
			else {
				fdY = fdir::SnN;
			}
		}
		else {
			if (qn > qs) {
				qmaxY = qn;
				fdY = fdir::N;
			}
			else {
				qmaxY = qs;
				fdY = fdir::S;
			}
		}
		// x, y 비교
		if (qmaxX == qmaxY) {
			qmax = qmaxX;
			switch (fdX)
			{
			case fdir::EnW:
				if (fdY == fdir::SnN) {
					fd_maxQ = fdir::EWSN;
				}
				else if (fdY == fdir::S) {
					fd_maxQ = fdir::SnEnW;
				}
				else if (fdY == fdir::N) {
					fd_maxQ = fdir::NnEnW;
				}
				break;
			case fdir::E:
				if (fdY == fdir::SnN) {
					fd_maxQ = fdir::EnSnN;
				}
				else if (fdY == fdir::S) {
					fd_maxQ = fdir::EnS;
				}
				else if (fdY == fdir::N) {
					fd_maxQ = fdir::EnN;
				}
				break;
			case fdir::W:
				if (fdY == fdir::SnN) {
					fd_maxQ = fdir::WnSnN;
				}
				else if (fdY == fdir::S) {
					fd_maxQ = fdir::WnS;
				}
				else if (fdY == fdir::N) {
					fd_maxQ = fdir::WnN;
				}
				break;
			}
		}
		else {
			if (qmaxX > qmaxY) {
				qmax = qmaxX;
				fd_maxQ = fdX;
			}
			else {
				qmax = qmaxY;
				fd_maxQ = fdY;
			}
		}
		flxmxfd.q = qmax;
		flxmxfd.fd_maxQ = static_cast<int>(fd_maxQ);
		return flxmxfd;
	}
	return flxmxfd;
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
			fluxfd flxfd;
			flxfd = getMaxValuesAndFD(cvs, i);
			cvsMV[i].vmax = flxfd.v;
			cvsMV[i].Qmax_cms = flxfd.q * gvi.dx;
			cvsMV[i].fdmaxV = flxfd.fd_maxV;
			cvsMV[i].fdmaxQ = flxfd.fd_maxQ;
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
}
