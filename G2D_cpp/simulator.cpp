#include "stdafx.h"

#include "gentle.h"
#include "g2d.h"
#include "g2d_cuda.cuh"

using namespace std;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;

extern projectFile prj;
extern generalEnv ge;
extern domaininfo di;
extern domainCell **dmcells;
extern cvatt *cvs;
extern cvattAddAtt *cvsAA;
extern double* cvsele;
extern bcAppinfo* bcAppinfos;

extern thisProcess ps;
extern thisProcessInner psi;
extern globalVinner gvi;
extern minMaxCVidx mnMxCVidx;
extern dataForCalDT dataForDT;

int simulationControl_CPU()
{
	double simDur_min = 0.;
	if (prj.printOutInterval_min > 1) {
		simDur_min = prj.simDuration_min + 1.0;
	}
	else {
		simDur_min = prj.simDuration_min;
	}
	int bcDataOrder = 0;
	int rfDataOrder = 0;
	int demToChangeEnded = 1;
	double tnow_min_bak = 0;
	int dtbc_min = prj.bcDataInterval_min;
	int* iGSmax;
	iGSmax = new int[0];
	if (prj.isDEMtoChangeApplied == 1) {
		demToChangeEnded = 0;
	}
	initThisProcess();
	initGlobalVinner();
	initFloodingThresholds();
	if (setStartingConditionCVs_CPU() == 0) { return 0; }
	do { //���� ������ �� t �� �ʱ� ����, t+dt�� �ҽ� �ϳ��� ����� ���
		psi.tnow_min = psi.tnow_sec / 60.0;
		if (prj.isbcApplied == 1) {//������� ��
			int bc_min = bcDataOrder * dtbc_min;
			if (((tnow_min_bak < bc_min) & (psi.tnow_min >= bc_min))
				|| bc_min == 0) {
				bcDataOrder++;
				getCellCD(bcDataOrder, dtbc_min);

			}
		}
		if (prj.isRainfallApplied == 1 && psi.rfEnded == 0) {//����
			int rf_min = rfDataOrder * prj.rainfallDataInterval_min;
			if (((tnow_min_bak < rf_min) & (psi.tnow_min >= rf_min))
				|| rf_min == 0) {
				rfDataOrder++; //1���� ����. �迭�� rainfallDataOrder-1
				psi.rfEnded = readRainfallAndGetIntensity(rfDataOrder);
			}
		}
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == 0) {//dem file ��ü
			demToChangeEnded = changeDomainElevWithDEMFile(psi.tnow_min, tnow_min_bak);
		}
		initilizeThisStep_CPU();
		runSolver_CPU(iGSmax);
		updateMinMaxInThisStep_CPU();
		if (psi.tnow_sec >= psi.tsec_targetToprint) {
			updateSummaryAndSetAllFalse();// ����Ҷ� ���� �� ���� ������Ʈ
			makeOutputFiles(psi.tnow_sec, iGSmax[0]);
			int progressRatio = (int)(psi.tnow_min / prj.simDuration_min * 100);
			printf("\rCurrent progress[min]: %d/%d[%d%%]..", (int)psi.tnow_min,
				(int)prj.simDuration_min, progressRatio);
			//�ѹ� ����Ҷ� ���� ���Ǻ��� ������Ʈ
			if (updateProjectParameters() == 0) { return 0; }
			else if (prj.parChanged == 1) {
				initThisProcess();
				initGlobalVinner();
				initFloodingThresholds();
				if (prj.printOutInterval_min > 1) {
					simDur_min = prj.simDuration_min + 1.0;
				}
				else {
					simDur_min = prj.simDuration_min;
				}
			}
			psi.tsec_targetToprint = psi.tsec_targetToprint + ps.dt_printout_sec;
			ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
		}
		tnow_min_bak = psi.tnow_min;
		psi.tnow_sec = psi.tnow_sec + gvi.dt_sec;
		if (prj.isFixedDT == 0) {
			gvi.dt_sec = getDTsecWithConstraints(dataForDT, gvi, psi.tnow_sec, bcAppinfos,
				mnMxCVidx);
		}
	} while (psi.tnow_min < simDur_min);
	return 1;
}


void runSolver_CPU(int* iGSmax)
{
	int nCells = gvi.nCellsInnerDomain;
	// ���⼭�� �迭, critical �ӵ� ����...
	int convergedinGS;
	iGSmax[0] = 0;
	omp_set_num_threads(ps.mdp);
	// ���⼭�� �迭, critical �ӵ� ����... �׷��� mdp 1���� ���ſ��� �۵� �ߵȴ�. critical������ �� �ȵȴ�.
	//	for (int igs = 0; igs < gvi.iGSmaxLimit; igs++) {
	//		convergedinGS = 1;
	//#pragma omp parallel 
	//		{
	//			int converged = 1;
	//			//int nchunk = gvi.nCellsInnerDomain / gvi.mdp;
	//			// reduction���� max, min ã�� ���� openMP 3.1 �̻���� ����, �׷��Ƿ� critical ����Ѵ�.
	//#pragma omp for schedule(guided) //, nchunk) // null�� �ƴ� ���̾, ��ȿ�� ������ ���ϹǷ�, ������ chunck�� ������� �ʴ� ���� ����.
	//			for (int i = 0; i < nCells; ++i) {
	//				if (cvs[i].isSimulatingCell == 1) {
	//					converged = calCEqUsingNR(cvs, gvi, bcAppinfos, cvsele, i);
	//					if (cvs[i].dp_tp1 > dMinLimit) {
	//						setEffCells(cvs, i);
	//					}
	//				}
	//			}
	//#pragma omp critical(getMaxNR) 
	//			{
	//				if (converged != 1) {
	//					convergedinGS = 0;
	//				}
	//			}
	//		}
	//		iGSmax[0] += 1;
	//		if (convergedinGS == 1) {
	//			break;
	//		}
	//	}//������� gs iteration    

	// ���⼭�� �迭, critical �ӵ� ����... �׷��� mdp 1���� ���ſ��� �۵� �ߵȴ�. critical������ �� �ȵȴ�.
	int* converged_eachThread;
	converged_eachThread = new int[ps.mdp]; // ���� ���� ������ �ʱ�ȭ ��.
	for (int igs = 0; igs < gvi.iGSmaxLimit; igs++) {
		convergedinGS = 1;
#pragma omp parallel
		{
			int tid = omp_get_thread_num();
			converged_eachThread[tid] = 1;
			// reduction���� max, min ã�� ���� openMP 3.1 �̻���� ����, 
#pragma omp for schedule(guided) // private(nrMax, tid) schedule(guided) // null�� �ƴ� ���̾, ��ȿ�� ������ ���ϹǷ�, ������ chunck�� ������� �ʴ� ���� ����.
			for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
				int converged = 1;
				if (cvs[i].isSimulatingCell == 1) {
					converged = calCEqUsingNR_CPU(cvs, gvi, bcAppinfos, cvsele, i);
					if (cvs[i].dp_tp1 > dMinLimit) {
						setEffCells(cvs, i);
					}
					if (converged != 1) {
						converged_eachThread[tid] = 0;
					}
				}
			}
		}
		for (int i = 0; i < ps.mdp; ++i) {
			if (converged_eachThread[i] != 1) {
				convergedinGS = 0;
			}
		}
		iGSmax[0] += 1;
		if (convergedinGS == 1) {
			break;
		}
	}//������� gs iteration    
	delete[] converged_eachThread;
}

// 0 : �̼���, 1: ����. __syncthreads(); �� ���ؼ� device ���� �ϳ��� �������. OnGPU ������ CPU ��� �����ϵ��� ��
int calCEqUsingNR_CPU(cvatt* cvs_L, globalVinner gvi_L,
	bcAppinfo* bcAppinfos_L, double* cvsele_L, int i) {
	double bcdepth = 0.0;
	double applyBCdepth = 0; // 1 : true, 0 : false
	if (cvs_L[i].isBCcell == 1) {
		int bcidx = getBcAppinfoidx(bcAppinfos_L, gvi_L.bcCellCountAll, i);
		if (bcAppinfos_L[bcidx].bctype == 2 || bcAppinfos_L[bcidx].bctype == 3) {// 1:Discharge, 2:Depth, 3:Height, 4:None
			bcdepth = bcAppinfos_L[bcidx].bcDepth_dt_m_tp1;
			applyBCdepth = 1; // ���� ���� �� ������ �����ϸ�, ������� ������ �����Ѵ�. 
		}
	}
	double dp_old = cvs_L[i].dp_tp1;
	for (int inr = 0; inr < gvi_L.iNRmaxLimit; ++inr) {
		double c1_IM = gvi_L.dt_sec / gvi_L.dx;
		double dn = cvs_L[i].dp_tp1;
		if (gvi_L.nCols > 1) {
			calWFlux(cvs_L, cvsele_L, gvi_L, i);
			calEFlux(cvs_L, cvsele_L, gvi_L, i);
		}
		if (gvi_L.nRows > 1) {
			calNFlux(cvs_L, cvsele_L, gvi_L, i);
			calSFlux(cvs_L, cvsele_L, gvi_L, i);
		}
		// ���� ���� ������ �ö󰡷���  -> qe-, qw+, qs-, qn+
		double fn = dn - cvs_L[i].dp_t + (cvs_L[i].qe_tp1 - cvs_L[i].qw_tp1
			+ cvs_L[i].qs_tp1 - cvs_L[i].qn_tp1) * c1_IM;//- sourceTerm; //�̰� ���ع�
		double eElem = pow(cvs_L[i].dfe, 2 / 3.0) * sqrt(abs(cvs_L[i].slpe)) / cvs_L[i].rc;
		double sElem = pow(cvs_L[i].dfs, 2 / 3.0) * sqrt(abs(cvs_L[i].slps)) / cvs_L[i].rc;
		double dfn = 1 + (eElem + sElem) * (5.0 / 3.0) * c1_IM;// �̰� ���ع�
		if (dfn == 0) { break; }
		double dnp1 = 0.0;
		if(applyBCdepth==1){
		//if (cvs_L[i].isBCcell == 1) {
			//int bcidx = getBcAppinfoidx(bcAppinfos_L, gvi_L.bcCellCountAll, i);
			//if (bcAppinfos_L[bcidx].bctype == 2 || bcAppinfos_L[bcidx].bctype == 3) {// 1:Discharge, 2:Depth, 3:Height, 4:None
			//	dnp1 = bcAppinfos_L[bcidx].bcDepth_dt_m_tp1;
			//}
			dnp1 = bcdepth;
		}
		else {
			dnp1 = dn - fn / dfn;
		}
		if (dnp1 < 0) { dnp1 = 0; }
		double resd = dnp1 - dn;
		cvs_L[i].dp_tp1 = dnp1;
		cvs_L[i].hp_tp1 = cvs_L[i].dp_tp1 + cvsele_L[i];
		if (abs(resd) <= CCh) { break; }
	}
	cvs_L[i].resd = abs(cvs_L[i].dp_tp1 - dp_old);
	if (cvs_L[i].resd > CCh) {
		return 0;
	}
	return 1;
}


double getDTsecWithConstraints(dataForCalDT dataForDT_L,
	globalVinner gvi_L, double tnow_sec,
	bcAppinfo* bcAppinfos_L, minMaxCVidx mnMxCVidx_L) {
	double dtsecCFL = 0.0;
	double dtsecCFLusingDepth = 0.0;
	double dtsecCFLusingV = 0.0;
	double half_dtPrint_sec = dataForDT_L.printOutInterval_min * 30.0;
	double half_bcdt_sec = dataForDT_L.bcDataInterval_min * 30.0;
	double half_rfdt_sec = dataForDT_L.rainfallDataInterval_min * 30.0;
	//==================================
	//�̰� cfl ����
	if (mnMxCVidx_L.dflowmaxInThisStep > 0) {
		dtsecCFLusingDepth = dataForDT_L.courantNumber * gvi_L.dx
			/ sqrt(GRAVITY * mnMxCVidx_L.dflowmaxInThisStep);
		//  �Ʒ�  �Ͱ� ����� �� ���� ����..
		//   dtsecCFL = cfln * dm.dx / Math.Sqrt(gravity * depthMax);
		dtsecCFL = dtsecCFLusingDepth;
	}
	if (mnMxCVidx_L.vmaxInThisStep > 0.0) {
		dtsecCFLusingV = dataForDT_L.courantNumber * gvi_L.dx / mnMxCVidx_L.vmaxInThisStep;
		dtsecCFL = dtsecCFLusingV;
	}
	if (dtsecCFLusingDepth > 0 && dtsecCFLusingV > 0) {
		dtsecCFL = min(dtsecCFLusingDepth, dtsecCFLusingV);
	}
	//==================================
	//==================================
	//�̰� Von Neuman ������ ����
	double dtsecVN = 0.0;
	if (dataForDT_L.applyVNC == 1) {
		dtsecVN = (mnMxCVidx_L.VNConMinInThisStep * gvi_L.dx * gvi_L.dx) / 4.0;
	}
	double dtsec = 0.0;
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
	if (half_bcdt_sec > 0.0 && dtsec > half_bcdt_sec) {
		dtsec = half_bcdt_sec;
	} //bc�� ������� ������ half_bcdt_sec=0
	if (half_rfdt_sec > 0 && dtsec > half_rfdt_sec) {
		dtsec = half_rfdt_sec;
	}  //rf�� ������� ������, half_rfdt_sec=0
	if (dtsec == 0) {
		dtsec = gvi_L.dt_sec * 1.5;
		if (dtsec > dtMAX_sec) { dtsec = dtMAX_sec; }
	}
	double maxSourceDepth = 0.0;
	double dtsecCFLusingBC = 0.0;
	//int bcdt_sec = dataForDT_L.bcDataInterval_min * 60;
	for (int n = 0; n < dataForDT_L.bcCellCountAll; ++n) {
		double bcDepth_dt_m = bcAppinfos_L[n].bcDepth_dt_m_tp1;
		if (bcDepth_dt_m > maxSourceDepth) {
			maxSourceDepth = bcDepth_dt_m;
		}
	}
	if (maxSourceDepth > 0) {
		dtsecCFLusingBC = dataForDT_L.courantNumber * gvi_L.dx
			/ sqrt(GRAVITY * (maxSourceDepth + mnMxCVidx_L.dflowmaxInThisStep));
		if (dtsecCFLusingBC < dtsec) { dtsec = dtsecCFLusingBC; }
	}
	if (dtsec < dtMIN_sec) { dtsec = dtMIN_sec; }
	else if (dtsec > dtMAX_sec) { dtsec = dtMAX_sec; }
	if (dtsec > 30) {
		double intpart;
		double realpart_t = modf(tnow_sec, &intpart);
		double fpart = modf(dtsec, &intpart); //dtsec�� ������ �����
		dtsec = intpart;
		dtsec = dtsec - realpart_t;  // �̷��� �ϸ� t+dt�� ������ �ȴ�.
	}
	return dtsec;
}


