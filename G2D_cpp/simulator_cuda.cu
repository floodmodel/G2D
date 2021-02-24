
#pragma once

#ifdef __INTELLISENSE___
void __syncthreads();
#else
void __syncthreads();
#endif

#include <cuda.h>
#include "cuda_runtime.h"
#include <cuda_runtime_api.h>
#include <device_functions.h>
#include "device_launch_parameters.h" // cuda에서 정의된 키워드 포함
#include <float.h>

#include "g2d_cuda.cuh"
#include "g2d.h"

extern projectFile prj;
extern domaininfo di;
extern generalEnv ge;

extern cvatt* cvs;
extern cvattAdd* cvsAA;

extern thisProcess ps;
extern thisProcessInner psi;
extern globalVinner gvi[1];

int simulationControl_GPU()
{
	cudaDeviceProp m_deviceProp;
	cudaGetDeviceProperties(&m_deviceProp, 0);
	if (!m_deviceProp.canMapHostMemory) {
		fprintf(stderr, "Device does not support mapping CPU host memory!\n");
		return 0;
	}
	int	nRows = di.nRows;
	int nCols = di.nCols;
	double dx = di.dx;
	double simDur_min = prj.simDuration_min + 1.0;
	int bcDataOrder = 0;
	int rfDataOrder = 0;
	int demToChangeEnded = 1;
	double tnow_min_bak = 0;
	ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
	ps.simulationStartTime = COleDateTime::GetCurrentTime();
	ps.rfEnded = 1;
	ps.tsec_targetToprint = 0;
	ps.dt_printout_sec = (int)(prj.printOutInterval_min * 60);
	ps.tnow_sec = 0;
	ps.effCellCount = 0;
	ps.dtbc_sec = prj.bcDataInterval_min * 60;
	ps.tTag_length = prj.tTag_length;
	int dtbc_min = prj.bcDataInterval_min;
	// gpu에서 구조체 변수 업데이트 안됨 (c#). 그래서 구조체 배열로 사용. 
	// C++에서 포인터 혹은 구조체 로 전환시 gpu 코딩에서 변수 업데이트 되는지 확인 필요
	int onCPU = 1;
	if (prj.isRainfallApplied == 1) {
		ps.rfEnded = 0;
	}
	if (prj.isDEMtoChangeApplied == 1) {
		demToChangeEnded = 0;
	}
	gvi[0] = initGlobalVinner();
	psi.dt_sec = ge.dtStart_sec;

	size_t  ms_cvs = di.cellNnotNull * sizeof(cvatt);
	size_t  ms_cvsAdd = di.cellNnotNull * sizeof(cvattAdd);
	cvatt* d_cvs;
	cvattAdd* d_cvsAA;

	cudaMalloc((void**)& d_cvs, ms_cvs);
	cudaMalloc((void**)& d_cvsAA, ms_cvsAdd);
	cudaMemcpy(d_cvs, cvs, ms_cvs, cudaMemcpyHostToDevice);
	cudaMemcpy(d_cvsAA, d_cvsAA, ms_cvsAdd, cudaMemcpyHostToDevice);
	dim3 threadsPerBlock(512, 1, 1); //blockDim    
	dim3 blocksPerGrid(di.cellNnotNull / (threadsPerBlock.x * threadsPerBlock.y) + 1, 1); //gridDim 


	setStartingCondition_GPU(d_cvs, d_cvsAA, di.cellNnotNull, blocksPerGrid, threadsPerBlock);
	do { //모의 시작할 때 t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
		ps.tnow_min = ps.tnow_sec / 60.0;
		if (prj.isbcApplied == 1) {//경계조건 등
			int bc_min = bcDataOrder * dtbc_min;
			if (((tnow_min_bak < bc_min) & (ps.tnow_min >= bc_min))
				|| bc_min == 0) {
				bcDataOrder++;
				getCellCD(bcDataOrder, dtbc_min);
			}
		}
		if (prj.isRainfallApplied == 1 && ps.rfEnded == 0) {//강우
			int rf_min = rfDataOrder * prj.rainfallDataInterval_min;
			if (((tnow_min_bak < rf_min) & (ps.tnow_min >= rf_min))
				|| rf_min == 0) {
				psi.rfisGreaterThanZero = 0;
				rfDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
				ps.rfEnded = readRainfallAndGetIntensity(rfDataOrder);
				// 0보다 큰 강우가 하나라도 있으면...
				if (psi.rfisGreaterThanZero == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori;
				}
				else if (psi.rfisGreaterThanZero == 0 || ps.rfEnded == 1) {
					//강우가 없을때는 최소수심을 좀 크게 잡아도 된다.
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
				}
			}
		}
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == 0) {//dem file 교체
			demToChangeEnded = changeDomainElevWithDEMFile(ps.tnow_min, tnow_min_bak);
		}
		initilizeThisStep_GPU << < blocksPerGrid, threadsPerBlock >> > (d_cvs, di.cellNnotNull);

		runSolver_GPU();
		updateValuesInThisStepResults();
		if (ps.tnow_sec >= ps.tsec_targetToprint) {
			checkEffCellNandSetAllFalse();// 출력할때 마다 이 정보 업데이트
			makeOutputFiles(ps.tnow_sec);
			int progressRatio = (int)(ps.tnow_min / prj.simDuration_min * 100);
			printf("\rCurrent progress[min]: %d/%d[%d%%]..", (int)ps.tnow_min,
				(int)prj.simDuration_min, progressRatio);
			//한번 출력할때 마다 모의변수 업데이트
			if (updateProjectParameters() == 0) {
				return 0;
			}
			ps.tsec_targetToprint = ps.tsec_targetToprint + ps.dt_printout_sec;
			ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
		}
		tnow_min_bak = ps.tnow_min;
		ps.tnow_sec = ps.tnow_sec + psi.dt_sec;
		if (prj.isFixedDT == 0) {
			psi.dt_sec = getDTsecWithConstraints(psi.dflowmaxInThisStep,
				psi.vmaxInThisStep, psi.VNConMinInThisStep);
		}
	} while (ps.tnow_min < simDur_min);
	return 1;
}

__global__ void initilizeThisStep_GPU(cvatt* cvs_k, int arraySize)
{
	int tid = blockDim.x * blockIdx.x + threadIdx.x;
	// Check if thread is within array bounds. threadid 보다 큰 array는 계산이 안된다.
	if (tid < arraySize) {
		initializeThisStepAcell(tid);
	}
}


void setStartingCondition_GPU(cvatt* cvs_k, cvattAdd* cvsadd_k, 
	int arraySize, dim3 blocksPerGrid, dim3 threadsPerBlock)
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
	setStartingConditionCVs_GPU << < blocksPerGrid, threadsPerBlock >> > (cvs_k, arraySize);
}

__global__ void setStartingConditionCVs_GPU(cvatt* cvs_k, cvattAdd* cvsadd_k,
	int arraySize) {
	int tid = blockDim.x * blockIdx.x + threadIdx.x;
	if (tid < arraySize) {
		cvs_k[tid].dp_t = cvsadd_k[tid].initialConditionDepth_m;
		cvs_k[tid].dp_tp1 = cvs_k[tid].dp_t;
		cvs_k[tid].ve_tp1 = 0;
		cvs_k[tid].qe_tp1 = 0;
		cvs_k[tid].qw_tp1 = 0;
		cvs_k[tid].qn_tp1 = 0;
		cvs_k[tid].qs_tp1 = 0;
		cvs_k[tid].hp_tp1 = cvs_k[tid].dp_tp1 + cvs_k[tid].elez;
		cvsadd_k[tid].fdmax = 0;// N = 1, E = 4, S = 16, W = 64, NONE = 0
		cvsadd_k[tid].bcData_curOrder = 0;
		cvsadd_k[tid].sourceRFapp_dt_meter = 0;
		cvsadd_k[tid].rfReadintensity_mPsec = 0;
		cvs_k[tid].isSimulatingCell = 0;
	}
}


void runSolver_GPU(cvatt* cvs_k, int arraySize){
	int nCells = gvi[0].nCellsInnerDomain;
	int thdWet = gvi[0].dMinLimitforWet;
	// 여기서는 배열, critical 속도 같다...
	psi.iGSmax = 0;
//	for (int igs = 0; igs < gvi[0].iGSmaxLimit; igs++) {
//		psi.bAllConvergedInThisGSiteration = 1;
//		psi.iNRmax = 0;
//#pragma omp parallel 
//		{
//			int nrMax = 0;
//#pragma omp for schedule(guided) 
//			for (int i = 0; i < nCells; ++i) {
//				if (cvs[i].isSimulatingCell == 1) {
//					nrMax = calCEqUsingNRforCPU(i);
//					if (cvs[i].dp_tp1 > thdWet) {
//						setEffCells(i);
//					}
//				}
//			}
//#pragma omp critical(getMaxNR) 
//			{
//				if (nrMax > psi.iNRmax) {
//					psi.iNRmax = nrMax;
//				}
//			}
//		}
//		psi.iGSmax += 1;
//		if (psi.bAllConvergedInThisGSiteration == 1) {
//			break;
//		}
//	}
}
