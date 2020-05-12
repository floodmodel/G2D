#include <stdio.h>
#include <omp.h>
#include "g2d.h"

using namespace std;

extern generalEnv ge;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern domaininfo di;
extern projectFile prj;
extern map <int, bcAppinfo> bcApp; //<cvid, bcCellinfo>

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
	gv.nCellsInnerDomain = di.cellNnotNull;
	gv.bcCellCountAll = prj.bcCellCountAll;
	gv.dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
	gv.slpMinLimitforFlow = ge.slpMinLimitforFlow;
	gv.domainOutBedSlope = prj.domainOutBedSlope;
	gv.ConvgC_h = ge.convergenceConditionh;
	gv.froudeNCriteria = prj.froudeNumberCriteria;
	gv.iNRmaxLimit = prj.maxIterationACellOnCPU; //ó������ cpu�� ���� ������ ����
	gv.iGSmaxLimit = prj.maxIterationAllCellsOnCPU;
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
	gv.mdp = prj.maxDegreeOfParallelism;
	return gv;
}

void initilizeThisStep()
{
	omp_set_num_threads(gvi[0].mdp);
	//prj.isParallel == 1 �� ��쿡�� gvi[0].mdp > 0 �� �����
#pragma omp parallel for //schedule(guided)//, nchunk) 
	for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
		initializeThisStepAcell(i);
	}
}

void initializeThisStepAcell(int idx)
{
	double h = cvs[idx].dp_tp1 + cvs[idx].elez; //elev �� ����Ǵ� ��찡 �����Ƿ�, �̷��� ��������
	if (cvs[idx].hp_tp1 <= h) {
		// dem  �� ����Ǹ�, ������ �ٲ��. ������ ����.
		// cvs[idx].hp_t=cvs[idx].elez + cvs[idx].dp_t �̹Ƿ�, cvs[idx].dp_t �̰��� cvs[idx].dp_tp1  ��� ������Ʈ ����� �Ѵ�.
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
	if (cvs[idx].isBCcell == 1) { // prj.isbcApplied == 1 ������ �����
		bcApp[idx].bcDepth_dt_m_tp1 = getCDasDepthWithLinear(bcApp[idx].bctype,
			cvs[idx].elez, gvi[0].dx, idx);
		if (bcApp[idx].bctype == 1)//1:  Discharge,  2: Depth, 3: Height,  4: None
		{//��������� ������ ���, �ҽ��׿� �־ ȫ�������Ѵ�. �������� ȯ��� ����..
			sourceAlltoRoute_tp1_dt_m = bcApp[idx].bcDepth_dt_m_tp1;
		}
		else
		{//��������� ������ �ƴҰ��, ȫ������ ���� �ʰ�, ������ �� ����.
			cvs[idx].dp_tp1 = bcApp[idx].bcDepth_dt_m_tp1;
			if (ps.tnow_sec == 0) {
				cvs[idx].dp_t = cvs[idx].dp_tp1;
			}
		}
	}
	cvsAA[idx].sourceRFapp_dt_meter = 0;
	//-1:false, 1: true
	if (prj.isRainfallApplied == 1 && ps.rfEnded == -1)
	{
		if (prj.rainfallDataType == rainfallDataType::TextFileASCgrid) {
			cvsAA[idx].sourceRFapp_dt_meter = cvsAA[idx].rfReadintensity_mPsec * psi.dt_sec;
		}
		else {
			cvsAA[idx].rfReadintensity_mPsec = psi.rfReadintensityForMAP_mPsec;
			cvsAA[idx].sourceRFapp_dt_meter = psi.rfReadintensityForMAP_mPsec * psi.dt_sec;
		}
	}
	sourceAlltoRoute_tp1_dt_m = sourceAlltoRoute_tp1_dt_m + cvsAA[idx].sourceRFapp_dt_meter;
	cvs[idx].dp_t = cvs[idx].dp_t + sourceAlltoRoute_tp1_dt_m;
	cvs[idx].dp_tp1 = cvs[idx].dp_tp1 + sourceAlltoRoute_tp1_dt_m;
	cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
	if (cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet) {
		setEffCells(idx);
	}
}

int setGenEnv()
{
	// ���⼭ omp_set_num_threads(gvi[0].mdp); �ѹ��� �ϸ�, ���߿� �ַ�����.
	// omp parallel �������� omp_set_num_threads(gvi[0].mdp); ���ָ� mdp�� �� ���� �ȴ�. 
	//omp_set_num_threads(prj.maxDegreeOfParallelism);
	ge.modelSetupIsNormal = 1;
	ge.gravity = 9.80665; // 1;
	ge.dMinLimitforWet_ori = 0.000001;
	// 0.00001;// �̰� 0�̸�, ���� ���� �������� ������ �κп��� �߻�. ������ ũ�� ���ȴ�..
	   // �� ���� 1. �ֺ������� �帧 ����� �� ��(effective ��) ������ ���ǰ�,
	   //            2. �� ������ ���� ���� �� ������ �ܺη��� ������ ���� �ȴ�. �ܺο��� �� ������ ������ ����
	   //            3. ������(����, ���� ��)�� ���� ���� �߰��� �����ϴ�.
	//slpMinLimitforFlow = 0.0001; //����
	ge.slpMinLimitforFlow = 0;// ����
	if (ge.isAnalyticSolution == 1) {
		ge.dtMaxLimit_sec = 2;// 600; //�ؼ��� �ϰ� ���Ҷ��� 1 �� ���� �� �´´�..
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
	ge.convergenceConditionh = 0.00001;// ���� 0.00001;//0.00001; // 0.00000001; //
	ge.convergenceConditionhr = 0.001;// ���� 0.00001;//0.00001; // 0.00000001; //
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
	//prj.isParallel == 1 �� ��쿡�� gvi[0].mdp > 0 �� �����
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
	psi.maxResdCVidx = -1;
	omp_set_num_threads(gvi[0].mdp);
#pragma omp parallel
	{
		double maxDflowL = 0;
		double maxvL = 0;
		double minvncL = 9999;
		cellResidual maxResL;
		maxResL.residual = 0.0;
#pragma omp for schedule(guided)//, nchunk) // null�� �ƴ� ���̾, ��ȿ�� ������ ���ϹǷ�, ������ chunck�� ������� �ʴ� ���� ����.
		for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
			if (cvs[i].isSimulatingCell == 1) {
				fluxData flxmax;
				if (cvs[i].cvidx_atW >= 0 && cvs[i].cvidx_atN >= 0) {
					//  �̰��� 4�� ���� ���п��� max �� ���
					flxmax = getFD4MaxValues(i,
						cvs[i].cvidx_atW,
						cvs[i].cvidx_atN);
					//flxmax = getFD4MaxValues(cvs[i],
					//	cvs[cvs[i].cvidx_atW],
					//	cvs[cvs[i].cvidx_atN]);
				}
				else if (cvs[i].cvidx_atW >= 0 && cvs[i].cvidx_atN < 0) {
					flxmax = getFD4MaxValues(i,
						cvs[i].cvidx_atW, i);
					//flxmax = getFD4MaxValues(cvs[i],
					//	cvs[cvs[i].cvidx_atW], cvs[i]);
				}
				else  if (cvs[i].cvidx_atW < 0 && cvs[i].cvidx_atN >= 0) {
					flxmax = getFD4MaxValues(i,
						i, cvs[i].cvidx_atN);
					//flxmax = getFD4MaxValues(cvs[i],
					//	cvs[i], cvs[cvs[i].cvidx_atN]);
				}
				else {//w, n�� ���� ���� ���
					flxmax = getFD4MaxValues(i, i, i);
					//flxmax = getFD4MaxValues(cvs[i], cvs[i], cvs[i]);
				}
				cvsAA[i].fdmax = flxmax.fd;
				cvsAA[i].vmax = flxmax.v;
				cvsAA[i].Qmax_cms = flxmax.q * gvi[0].dx;
				if (flxmax.dflow > maxDflowL) {
					maxDflowL = flxmax.dflow;
				}
				if (cvsAA[i].vmax > maxvL) {
					maxvL = cvsAA[i].vmax;
				}
				double vnCon = 0;
				if (gvi[0].isApplyVNC == 1) {
					vnCon = getVNConditionValue(i);
					if (vnCon < minvncL) {
						minvncL = vnCon;
					}
				}
				if (cvs[i].resd > maxResL.residual) {
					maxResL.residual = cvs[i].resd;
					maxResL.cvidx = i;
				}
			}
		}
#pragma omp critical(updatePSv)
		{
			if (psi.dflowmaxInThisStep < maxDflowL) {
				psi.dflowmaxInThisStep = maxDflowL;
			}
			if (psi.vmaxInThisStep < maxvL) {
				psi.vmaxInThisStep = maxvL;
			}
			if (psi.VNConMinInThisStep > minvncL) {
				psi.VNConMinInThisStep = minvncL;
			}
			if (psi.maxResd < maxResL.residual) {
				psi.maxResd = maxResL.residual;
				psi.maxResdCVidx = maxResL.cvidx;
			}
		}
	}
		////serial ========================
		//for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
		//	if (cvs[i].isSimulatingCell == 1) {
		//		fluxData flxmax;
		//		if (cvs[i].cvidx_atW >= 0 && cvs[i].cvidx_atN >= 0) {
		//			//  �̰��� 4�� ���� ���п��� max �� ���
		//			flxmax = getFD4MaxValues(cvs[i],
		//				cvs[cvs[i].cvidx_atW],
		//				cvs[cvs[i].cvidx_atN]);
		//		}
		//		else if (cvs[i].cvidx_atW >= 0 && cvs[i].cvidx_atN < 0) {
		//			flxmax = getFD4MaxValues(cvs[i],
		//				cvs[cvs[i].cvidx_atW], cvs[i]);
		//		}
		//		else  if (cvs[i].cvidx_atW < 0 && cvs[i].cvidx_atN >= 0) {
		//			flxmax = getFD4MaxValues(cvs[i],
		//				cvs[i], cvs[cvs[i].cvidx_atN]);
		//		}
		//		else {//w, n�� ���� ���� ���
		//			flxmax = getFD4MaxValues(cvs[i], cvs[i], cvs[i]);
		//		}
		//		cvsAA[i].fdmax = flxmax.fd;
		//		cvsAA[i].vmax = flxmax.v;
		//		cvsAA[i].Qmax_cms = flxmax.q * gvi[0].dx;
		//		if (flxmax.dflow > psi.dflowmaxInThisStep) {
		//			psi.dflowmaxInThisStep = flxmax.dflow;
		//		}
		//		if (cvsAA[i].vmax > psi.vmaxInThisStep) {
		//			psi.vmaxInThisStep = cvsAA[i].vmax;
		//		}
		//		double vnCon = 0;
		//		if (gvi[0].isApplyVNC == 1) {
		//			vnCon = getVNConditionValue(i);
		//			if (vnCon < psi.VNConMinInThisStep) {
		//				psi.VNConMinInThisStep = vnCon;
		//			}
		//		}
		//		if (cvs[i].resd > psi.maxResd) {
		//			psi.maxResd = cvs[i].resd;
		//			psi.maxResdCVidx = i;
		//		}
		//	}
		//}
		////serial ========================
}



fluxData getFD4MaxValues2(int i)
{	
	fluxData flxmax; 
	//cvatt cell=cvs[i]
//	double ve = abs(cell.ve_tp1);
//	double vw = abs(cell.vw_tp1);
//	double vs = abs(cell.vs_tp1);
//	double vn = abs(cell.vn_tp1);
//	int fdx = 1; ;//E = 1, S = 3, W = 5, N = 7, NONE = 0
//	int fdy = 3; ;//E = 1, S = 3, W = 5, N = 7, NONE = 0
//	double maxvX = 0.0;
//	double maxvY = 0.0;
//	double maxv = 0.0;
//	double fd = 0;
//	if (ve > vw) {
//		maxvX = ve;
//		if (cell.ve_tp1 < 0) {
//			fdx = 5;
//		}
//	}
//	else {
//		maxvX = vw;
//		if (cell.vw_tp1 < 0) {
//			fdx = 5;
//		}
//	}
//	if (vs > vn) {
//		maxvY = vs;
//		if (cell.vs_tp1 < 0) {
//			fdy = 7;
//		}
//	}
//	else {
//		maxvY = vn;
//		if (cell.vn_tp1 < 0) {
//			fdy = 7;
//		}
//	}
//	if (maxvX > maxvY) {
//		flxmax.v = maxvX;
//		flxmax.fd = fdx;
//	}
//	else {
//		flxmax.v = maxvY;
//		flxmax.fd= fdy;
//	}
//
//	
//	double qmaxX = 0;
//	double qmaxY = 0;
//	double maxq = 0;
//	qmaxX = max(abs(cell.qw_tp1), cell.qe_tp1);
//	qmaxY = max(cell.qn_tp1, cell.qs_tp1);
//	flxmax.q = max(qmaxX, qmaxY);
//
//	double dmaxX = 0;
//	double dmaxY = 0;
//	dmaxX = max(cell.dfw, cell.dfe);
//	dmaxY = max(cell.dfn, cell.dfs);
//	flxmax.dflow = max(dmaxX, dmaxY);
	return flxmax;
}

fluxData getFD4MaxValues(int ip, int iw, int in)
{	// cell�� ���� �޴� �� ����, index�� �޾Ƽ� ���������� cell�� �����ϴ°� �� ������..2020.05.12
	fluxData flxmax;
	cvatt wcell = cvs[iw];
	cvatt cell = cvs[ip];
	cvatt ncell = cvs[in];
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

double getVNConditionValue(int i)
{
	double searchMIN = DBL_MAX;
	double curValue = 0;
	double rc = cvs[i].rc;
	// e ���� �ߺ��ǹǷ�, w�� ������� �ʴ´�.
	if (cvs[i].dfe > 0) {
		searchMIN = 2 * rc * sqrt(abs(cvs[i].slpe))
			/ pow(cvs[i].dfe, 5.0 / 3.0);
		//if (curValue < searchMIN) {
		//	searchMIN = curValue;
		//}
	}
	// s ���� �ߺ��ǹǷ�, n�� ������� �ʴ´�.
	if (cvs[i].dfs > 0) {
		curValue = 2 * rc * sqrt(abs(cvs[i].slps))
			/ pow(cvs[i].dfs, 5.0 / 3.0);
		if (curValue < searchMIN) {
			searchMIN = curValue;
		}
	}
	return searchMIN;
}

//double getVonNeumanConditionValue(cvatt cell)
//{
//	double searchMIN = DBL_MAX;
//	double curValue = 0;
//	double rc = cell.rc;
//	// e ���� �ߺ��ǹǷ�, w�� ������� �ʴ´�.
//	if (cell.dfe > 0) {
//		searchMIN = 2 * rc * sqrt(abs(cell.slpe))
//			/ pow(cell.dfe, 5.0 / 3.0);
////		if (curValue < searchMIN) {
////			searchMIN = curValue;
////		}
//	}
//	// s ���� �ߺ��ǹǷ�, n�� ������� �ʴ´�.
//	if (cell.dfs > 0) {
//		curValue = 2 * rc * sqrt(abs(cell.slps))
//			/ pow(cell.dfs, 5.0 / 3.0);
//		if (curValue < searchMIN) {
//			searchMIN = curValue;
//		}
//	}
//	return searchMIN;
//}

void checkEffCellNandSetAllFalse()
{
	ps.effCellCount = 0;
	ps.FloodingCellCounts.clear();// = new vector<int>();
	//cThisProcess.FloodingCellMaxDepth = new List<double>();
	ps.FloodingCellMeanDepth.clear();// = new List<double>();
	vector<double> FloodingCellSumDepth;
	ps.FloodingCellMaxDepth = 0;
	for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
		ps.FloodingCellCounts.push_back(0);//0�������� �ʱ�ȭ �Ѵ�.
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
	//�̰� cfl ����
	if (dflowmax > 0) {
		dtsecCFLusingDepth = prj.courantNumber * di.dx 
			/ sqrt(gvi[0].gravity * dflowmax);
		//  �Ʒ�  �Ͱ� ����� �� ���� ����..
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
	//�̰� Von Neuman ������ ����
	double dtsecVN = 0;
	if (prj.applyVNC == 1) {
		dtsecVN = (vonNeumanCon * di.dx * di.dx) / 4;
	}
	double dtsec = 0;
	if (dtsecVN > 0 && dtsecCFL > 0) { 
		dtsec = min(dtsecCFL, dtsecVN); 
	}
	else { 
		dtsec = max(dtsecCFL, dtsecVN); 
	}
	//===================================
	if (dtsec > half_dtPrint_sec) { 
		dtsec = half_dtPrint_sec; 
	}
	if (half_bcdt_sec > 0 && dtsec > half_bcdt_sec) { 
		dtsec = half_bcdt_sec; 
	} //bc�� ������� ������ half_bcdt_sec=0
	if (half_rfdt_sec > 0 && dtsec > half_rfdt_sec) { 
		dtsec = half_rfdt_sec; 
	}  //rf�� ������� ������, half_rfdt_sec=0
	if (dtsec == 0) {
		dtsec = psi.dt_sec * 1.5;
		if (dtsec > ge.dtMaxLimit_sec) { dtsec = ge.dtMaxLimit_sec; }
	}
	double maxSourceDepth = 0;
	double dtsecCFLusingBC = 0;
	int bcdt_sec = prj.bcDataInterval_min * 60;
	for (int idx:prj.bcCVidxList){
		double bcDepth_dt_m_tp1 = 0;
		bcDepth_dt_m_tp1 = getCDasDepthWithLinear(bcApp[idx].bctype,
			cvs[idx].elez, di.dx, idx);
		if (bcDepth_dt_m_tp1 > maxSourceDepth) {
			maxSourceDepth = bcDepth_dt_m_tp1; }
	}
	if (maxSourceDepth > 0) {
		dtsecCFLusingBC = prj.courantNumber * di.dx 
			/ sqrt(ge.gravity * (maxSourceDepth + dflowmax));
		if (dtsecCFLusingBC < dtsec) { dtsec = dtsecCFLusingBC; }
	}
	if (dtsec < ge.dtMinLimit_sec) { dtsec = ge.dtMinLimit_sec; }
	if (dtsec > ge.dtMaxLimit_sec) { dtsec = ge.dtMaxLimit_sec; }
	if (dtsec > 5) {
		double intpart;
		double realpart_t = modf(ps.tnow_sec, &intpart);
		double fpart = modf(dtsec, &intpart); //dtsec�� ������ �����
		dtsec = intpart;
		dtsec = dtsec - realpart_t;  // �̷��� �ϸ� t+dt�� ������ �ȴ�.
	}
	return dtsec;
}


