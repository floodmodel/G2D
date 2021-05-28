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
	do { //모의 시작할 때 t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
		psi.tnow_min = psi.tnow_sec / 60.0;
		if (prj.isbcApplied == 1) {//경계조건 등
			int bc_min = bcDataOrder * dtbc_min;
			if (((tnow_min_bak < bc_min) & (psi.tnow_min >= bc_min))
				|| bc_min == 0) {
				bcDataOrder++;
				getCellCD(bcDataOrder, dtbc_min);

			}
		}
		if (prj.isRainfallApplied == 1 && psi.rfEnded == 0) {//강우
			int rf_min = rfDataOrder * prj.rainfallDataInterval_min;
			if (((tnow_min_bak < rf_min) & (psi.tnow_min >= rf_min))
				|| rf_min == 0) {
				rfDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
				psi.rfEnded = readRainfallAndGetIntensity(rfDataOrder);
			}
		}
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == 0) {//dem file 교체
			demToChangeEnded = changeDomainElevWithDEMFile(psi.tnow_min, tnow_min_bak);
		}
		initilizeThisStep_CPU();
		runSolver_CPU(iGSmax);
		updateMinMaxInThisStep_CPU();
		if (psi.tnow_sec >= psi.tsec_targetToprint) {
			updateSummaryAndSetAllFalse();// 출력할때 마다 이 정보 업데이트
			makeOutputFiles(psi.tnow_sec, iGSmax[0]);
			int progressRatio = (int)(psi.tnow_min / prj.simDuration_min * 100);
			printf("\rCurrent progress[min]: %d/%d[%d%%]..", (int)psi.tnow_min,
				(int)prj.simDuration_min, progressRatio);
			//한번 출력할때 마다 모의변수 업데이트
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
	// 여기서는 배열, critical 속도 같다...
	int convergedinGS;
	iGSmax[0] = 0;
	omp_set_num_threads(ps.mdp);
	// 여기서는 배열, critical 속도 같다... 그러나 mdp 1에서 수렴여부 작동 잘된다. critical에서는 잘 안된다.
	//	for (int igs = 0; igs < gvi.iGSmaxLimit; igs++) {
	//		convergedinGS = 1;
	//#pragma omp parallel 
	//		{
	//			int converged = 1;
	//			//int nchunk = gvi.nCellsInnerDomain / gvi.mdp;
	//			// reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 그러므로 critical 사용한다.
	//#pragma omp for schedule(guided) //, nchunk) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
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
	//	}//여기까지 gs iteration    

	// 여기서는 배열, critical 속도 같다... 그러나 mdp 1에서 수렴여부 작동 잘된다. critical에서는 잘 안된다.
	int* converged_eachThread;
	converged_eachThread = new int[ps.mdp]; // 아주 작은 값으로 초기화 됨.
	for (int igs = 0; igs < gvi.iGSmaxLimit; igs++) {
		convergedinGS = 1;
#pragma omp parallel
		{
			int tid = omp_get_thread_num();
			converged_eachThread[tid] = 1;
			// reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 
#pragma omp for schedule(guided) // private(nrMax, tid) schedule(guided) // null이 아닌 셀이어도, 유효셀 개수가 변하므로, 고정된 chunck를 사용하지 않는 것이 좋다.
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
	}//여기까지 gs iteration    
	delete[] converged_eachThread;
}

// 0 : 미수렴, 1: 수렴. __syncthreads(); 를 위해서 device 용을 하나더 만들었다. OnGPU 에서도 CPU 사용 가능하도록 함
int calCEqUsingNR_CPU(cvatt* cvs_L, globalVinner gvi_L,
	bcAppinfo* bcAppinfos_L, double* cvsele_L, int i) {
	double bcdepth = 0.0;
	double applyBCdepth = 0; // 1 : true, 0 : false
	if (cvs_L[i].isBCcell == 1) {
		int bcidx = getBcAppinfoidx(bcAppinfos_L, gvi_L.bcCellCountAll, i);
		if (bcAppinfos_L[bcidx].bctype == 2 || bcAppinfos_L[bcidx].bctype == 3) {// 1:Discharge, 2:Depth, 3:Height, 4:None
			bcdepth = bcAppinfos_L[bcidx].bcDepth_dt_m_tp1;
			applyBCdepth = 1; // 현재 셀이 이 조건을 만족하면, 경계조건 수심을 적용한다. 
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
		// 현재 셀의 수위가 올라가려면  -> qe-, qw+, qs-, qn+
		double fn = dn - cvs_L[i].dp_t + (cvs_L[i].qe_tp1 - cvs_L[i].qw_tp1
			+ cvs_L[i].qs_tp1 - cvs_L[i].qn_tp1) * c1_IM;//- sourceTerm; //이건 음해법
		double eElem = pow(cvs_L[i].dfe, 2 / 3.0) * sqrt(abs(cvs_L[i].slpe)) / cvs_L[i].rc;
		double sElem = pow(cvs_L[i].dfs, 2 / 3.0) * sqrt(abs(cvs_L[i].slps)) / cvs_L[i].rc;
		double dfn = 1 + (eElem + sElem) * (5.0 / 3.0) * c1_IM;// 이건 음해법
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
	//이건 cfl 조건
	if (mnMxCVidx_L.dflowmaxInThisStep > 0) {
		dtsecCFLusingDepth = dataForDT_L.courantNumber * gvi_L.dx
			/ sqrt(GRAVITY * mnMxCVidx_L.dflowmaxInThisStep);
		//  아래  것과 결과에 별 차이 없다..
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
	//이건 Von Neuman 안정성 조건
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
	} //bc가 적용되지 않으면 half_bcdt_sec=0
	if (half_rfdt_sec > 0 && dtsec > half_rfdt_sec) {
		dtsec = half_rfdt_sec;
	}  //rf가 적용되지 않으면, half_rfdt_sec=0
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
		double fpart = modf(dtsec, &intpart); //dtsec를 정수로 만들고
		dtsec = intpart;
		dtsec = dtsec - realpart_t;  // 이렇게 하면 t+dt가 정수가 된다.
	}
	return dtsec;
}


