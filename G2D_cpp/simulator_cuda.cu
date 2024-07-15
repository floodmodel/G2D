#ifdef __INTELLISENSE___
void __syncthreads();
#else
void __syncthreads();
#endif

#include "stdafx.h"
#include <ATLComTime.h>

#include "g2d.h"
#include "g2d_cuda.cuh"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_log;

extern projectFile prj;
extern domaininfo di;
extern domainCell** dmcells;
extern generalEnv ge;

extern cvatt* cvs;
extern cvattAddAtt* cvsAA;
extern double* cvsele;
extern double* rfi_read_mPs;
extern bcAppinfo * bcAppinfos;

extern thisProcess ps;
extern thisProcessInner psi;
extern globalVinner gvi;
extern dataForCalDT dataForDT;
extern minMaxFlux mnMxFluxFromAllcells;


//#ifdef OnGPU
int simulationControl_GPU()
{
	cudaDeviceProp m_deviceProp;
	cudaGetDeviceProperties(&m_deviceProp, 0);
	if (!m_deviceProp.canMapHostMemory) {
		fprintf(stderr, "Device does not support mapping CPU host memory!\n");
		return 0;
	}
	double simDur_min = 0.0;
	if (prj.printOutInterval_min > 1) {
		simDur_min = prj.simDuration_min + 1.0;
	}
	else {
		simDur_min = prj.simDuration_min;
	}
	int bcDataOrder = 0;
	int rfDataOrder = 0;
	int demToChangeEnded = 1;
	double tnow_min_bak = 0.0;
	int dtbc_min = prj.bcDataInterval_min;
	if (prj.isDEMtoChangeApplied == 1) {
		demToChangeEnded = 0;
	}
	initThisProcess();
	initGlobalVinner();
	initFloodingThresholds();
	initMinMax();
	cvatt* d_cvs;
	cvattAddAtt* d_cvsAA;
	double* d_cvsele;
	double* d_rfi_read_mPs;
	double* d_dtsec;
	bcAppinfo* d_bcAppinfos;
	minMaxFlux* d_minMaxCVidx;

	size_t  ms_cvs_ncvs = di.cellNnotNull * sizeof(cvatt);
	size_t  ms_cvsAA_ncvs = di.cellNnotNull * sizeof(cvattAddAtt);
	size_t ms_double_ncvs = di.cellNnotNull * sizeof(double);
	size_t ms_bcAppinfo_allBCcells = prj.bcCellCountAll * sizeof(bcAppinfo);
	size_t ms_minMaxCVidx = sizeof(minMaxFlux);
	size_t ms_minMaxCVidx_TPB = ps.threadsPerBlock * sizeof(minMaxFlux);

	dim3 thPblock(ps.threadsPerBlock, 1, 1); //blockDim    
	dim3 bPgrid(di.cellNnotNull / (thPblock.x * thPblock.y) + 1, 1); //gridDim 

	//clock_t  ts, tf;
	//long tc_memcopy = 0;
	//long tc_setStartingConditionCVs_GPU = 0;
	//long tc_set_rf_bc_dem = 0;
	//long tc_initilizeThisStep_GPU = 0;
	//long tc_GSiteration = 0;
	//long tc_getMinMaxFromCV = 0;
	//long tc_getMinMax_reduction = 0;
	//long tc_memcpy_mnMxCVidx = 0;
	//long tc_setAllCVFalse = 0;

	//ts = clock();
	cudaMalloc((void**)& d_cvs, ms_cvs_ncvs);
	cudaMalloc((void**)& d_cvsAA, ms_cvsAA_ncvs);
	cudaMalloc((void**)& d_cvsele, ms_double_ncvs);
	cudaMalloc((void**)& d_bcAppinfos, ms_bcAppinfo_allBCcells);
	cudaMalloc((void**)& d_rfi_read_mPs, ms_double_ncvs);
	cudaMalloc((void**)& d_minMaxCVidx, bPgrid.x * sizeof(minMaxFlux));
	cudaMalloc((void**)& d_dtsec, sizeof(double));
	cudaMemcpy(d_cvs, cvs, ms_cvs_ncvs, cudaMemcpyHostToDevice);
	cudaMemcpy(d_cvsAA, cvsAA, ms_cvsAA_ncvs, cudaMemcpyHostToDevice);
	cudaMemcpy(d_cvsele, cvsele, ms_double_ncvs, cudaMemcpyHostToDevice);
	cudaMemcpy(d_bcAppinfos, bcAppinfos, ms_bcAppinfo_allBCcells, cudaMemcpyHostToDevice);
	cudaMemcpy(d_rfi_read_mPs, rfi_read_mPs, ms_double_ncvs, cudaMemcpyHostToDevice);
	//tf = clock();
	//tc_memcopy = long(tf - ts);

	//ts = clock();
	//setStartingConditionCVs_GPU << < bPgrid, thPblock >> > (d_cvs, d_cvsAA, d_cvsele, gvi.nCellsInnerDomain);
	setStartingConditionCVs_GPU << < bPgrid, thPblock >> > (d_cvs, d_cvsAA, d_cvsele, gvi.nCellsInnerDomain);
	CUDA_CHECK(cudaGetLastError());
	CUDA_CHECK(cudaDeviceSynchronize());
	//tf = clock();
	//tc_setStartingConditionCVs_GPU = long(tf - ts);

	do { //모의 시작할 때 t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
		psi.tnow_min = psi.tnow_sec / 60.0;
		//ts = clock();
		if (prj.isbcApplied == 1) {//경계조건 등
			int bc_min = bcDataOrder * dtbc_min;
			if (((tnow_min_bak < bc_min) & (psi.tnow_min >= bc_min))
				|| bc_min == 0) {
				bcDataOrder++;
				getCellCD(bcDataOrder, dtbc_min); // 경계조건 값은 포인터로 넘긴다..
				cudaMemcpy(d_bcAppinfos, bcAppinfos, ms_bcAppinfo_allBCcells, cudaMemcpyHostToDevice);
			}
		}
		if (prj.isRainfallApplied == 1 && psi.rfEnded == 0) {//강우
			int rf_min = rfDataOrder * prj.rainfallDataInterval_min;
			if (((tnow_min_bak < rf_min) & (psi.tnow_min >= rf_min))
				|| rf_min == 0) {
				rfDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
				psi.rfEnded = readRainfallAndGetIntensity(rfDataOrder);
				cudaMemcpy(d_rfi_read_mPs, rfi_read_mPs, ms_double_ncvs, cudaMemcpyHostToDevice);
			}
		}
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == 0) {//dem file 교체
			demToChangeEnded = changeDomainElevWithDEMFile(psi.tnow_min, tnow_min_bak);
			cudaMemcpy(d_cvsele, cvsele, ms_double_ncvs, cudaMemcpyHostToDevice);
		}
		//CUDA_CHECK(cudaDeviceSynchronize());
		//tf = clock();
		//tc_set_rf_bc_dem = long(tf - ts);

		//ts = clock();
		initilizeThisStep_GPU << < bPgrid, thPblock >> > (d_cvs, d_cvsAA, d_cvsele, d_bcAppinfos, d_rfi_read_mPs, psi, gvi);
		CUDA_CHECK(cudaGetLastError());
		CUDA_CHECK(cudaDeviceSynchronize());
		//tf = clock();
		//tc_initilizeThisStep_GPU = long(tf - ts);

		//ts = clock();
		runSolver_GPU << < bPgrid, thPblock >> > (d_cvs, d_bcAppinfos, d_cvsele, gvi);
		CUDA_CHECK(cudaGetLastError());
		CUDA_CHECK(cudaDeviceSynchronize());
		//tf = clock();
		//tc_GSiteration = long(tf - ts);

		//ts = clock();
		//getMinMaxFromCV << < bPgrid, thPblock, ms_minMaxCVidx_TPB >> > (d_cvs,	d_cvsAA, gvi, d_minMaxCVidx);
		updateGlobalMinMaxFromCV << < bPgrid, thPblock, ms_minMaxCVidx_TPB >> > (d_cvs, gvi, d_minMaxCVidx);
		CUDA_CHECK(cudaGetLastError());
		CUDA_CHECK(cudaDeviceSynchronize());
		//tf = clock();
		//tc_getMinMaxFromCV = long(tf - ts);

		// reduction=================================
		//ts = clock();
		if (bPgrid.x > 1) {
			int array_size = bPgrid.x;
			int numBlock = (array_size + thPblock.x - 1) / thPblock.x + 1;;
			while (numBlock != 1) {
				if (array_size < ps.threadsPerBlock) {
					numBlock = 1;
					updateGlobalMinMaxFromArray << < numBlock, thPblock, ms_minMaxCVidx_TPB >> > (d_minMaxCVidx,
						array_size, gvi.isApplyVNC, d_minMaxCVidx);
					//CUDA_CHECK(cudaGetLastError());
					CUDA_CHECK(cudaDeviceSynchronize()); // 커널함수 들이 완료될때 까지 대기, block 동기화
					break;
				}
				else {
					updateGlobalMinMaxFromArray << < numBlock, thPblock, ms_minMaxCVidx_TPB >> > (d_minMaxCVidx,
						array_size, gvi.isApplyVNC, d_minMaxCVidx);
					//CUDA_CHECK(cudaGetLastError());
					CUDA_CHECK(cudaDeviceSynchronize());// 커널함수 들이 완료될때 까지 대기, block 동기화 
					array_size = numBlock;
					numBlock = (numBlock + thPblock.x - 1) / thPblock.x + 1;
					
				}
			}
		}
		//tf = clock();
		//tc_getMinMax_reduction = long(tf - ts);
		//==========================================
		if (psi.tnow_sec >= psi.tsec_targetToprint) {
			cudaMemcpy(cvs, d_cvs, ms_cvs_ncvs, cudaMemcpyDeviceToHost);
			updateSummaryAndSetAllFalse();// 출력할때 마다 이 정보 업데이트
			//updateSummaryAndSetAllFalse_serial(); // openMP 병렬계산과 결과 같음
			//ts = clock();
			setAllCVFalse << <bPgrid, thPblock >> > (d_cvs, gvi.nCellsInnerDomain); // cvs.isSimulatingCell 을 복사하지 않고, 여기서 d_cvs에서 설정해 준다.
			CUDA_CHECK(cudaGetLastError());
			CUDA_CHECK(cudaDeviceSynchronize());
			//tf = clock();
			//tc_setAllCVFalse = long(tf - ts);
			//cout << "\n\nCurrent min           : " << psi.tnow_min << "min" << endl;
			//cout << "Time consumed malloc, memcopy          : " << tc_memcopy << "ms" << endl;
			//cout << "Time consumed setStartingCondition_GPU : " << tc_setStartingConditionCVs_GPU << "ms" << endl;
			//cout << "Time consumed tc_set_rf_bc_dem         : " << tc_set_rf_bc_dem << "ms" << endl;
			//cout << "Time consumed tc_initilizeThisStep_GPU : " << tc_initilizeThisStep_GPU << "ms" << endl;
			//cout << "Time consumed tc_GSiteration           : " << tc_GSiteration << "ms" << endl;
			//cout << "Time consumed tc_getMinMaxFromCV       : " << tc_getMinMaxFromCV << "ms" << endl;
			//cout << "Time consumed tc_getMinMax_reduction   : " << tc_getMinMax_reduction << "ms" << endl;
			//cout << "Time consumed tc_memcpy_mnMxCVidx      : " << tc_memcpy_mnMxCVidx << "ms" << endl;
			//cout << "Time consumed tc_setAllCVFalse         : " << tc_setAllCVFalse << "ms" << endl;
			makeOutputFiles(psi.tnow_sec, gvi.iGSmaxLimit);
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
			//ts = clock();
			cudaMemcpy(&mnMxFluxFromAllcells, d_minMaxCVidx, ms_minMaxCVidx, cudaMemcpyDeviceToHost); // dt를 계산하려면, 매번 받아와야 한다. 
			// bcAppinfos_L[bci].bcDepth_dt_m_tp1를 initializeThisStepAcell()에서 매번 계산하므로
			// dt를 계산하려면, 매번 받아와야 한다.
			cudaMemcpy(bcAppinfos, d_bcAppinfos, ms_bcAppinfo_allBCcells, cudaMemcpyDeviceToHost); 
			//tf = clock();
			//tc_memcpy_mnMxCVidx = long(tf - ts);
			gvi.dt_sec = getDTsecWithConstraints(dataForDT, gvi, psi.tnow_sec, bcAppinfos,
				mnMxFluxFromAllcells);
		}
	} while (psi.tnow_min < simDur_min);

	CUDA_CHECK(cudaFree(d_cvs));
	CUDA_CHECK(cudaFree(d_cvsAA));
	CUDA_CHECK(cudaFree(d_cvsele));
	CUDA_CHECK(cudaFree(d_bcAppinfos));
	CUDA_CHECK(cudaFree(d_rfi_read_mPs));
	return 1;
}

__global__ void initilizeThisStep_GPU(cvatt* d_cvs, cvattAddAtt* d_cvsAA, double * d_cvsele,
	bcAppinfo* d_bcApp, double* d_rfi_read_mPs, 
	thisProcessInner psi_k, globalVinner gvi_k){
	int idx = blockDim.x * blockIdx.x + threadIdx.x;
	if (idx < gvi_k.nCellsInnerDomain) {
		initializeThisStepAcell(d_cvs, d_cvsAA, d_bcApp, d_cvsele[idx], d_rfi_read_mPs[idx], idx, psi_k, gvi_k);
		if (d_cvs[idx].dp_tp1 >dMinLimit) {
			setEffCells(d_cvs, idx);
		}
	}
}

__global__ void setStartingConditionCVs_GPU(cvatt* d_cvs, cvattAddAtt * d_cvsAA, 
	double* d_cvsele, int arraySize) {
	int tid = blockDim.x * blockIdx.x + threadIdx.x;
	if (tid < arraySize) {
		setStartingConditionCVs_inner(d_cvs, d_cvsAA, d_cvsele, tid);
	}
}

__global__ void updateGlobalMinMaxFromCV(cvatt* cvs_k, //cvattAddAtt* cvsAA_k,
	globalVinner gvi_k, minMaxFlux* odata) {
	/*__shared__ minMaxCVidx sdata[THPB]; */
	extern __shared__ minMaxFlux sdata[];
	unsigned int tid = threadIdx.x;
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	sdata[tid].dflowmaxInThisStep = -9999.0; // 여기에서 초기화 해준다. 실제 배열 길이가 < tid + s 인경우에도 값을 입력..
	sdata[tid].vmaxInThisStep = -9999.0;
	sdata[tid].VNConMinInThisStep = 9999.0;
	fluxNfd flxmax;
	if (idx < gvi_k.nCellsInnerDomain) {
		flxmax = get_maxFlux_FD(cvs_k, idx); // 셀별 max 값을 찾아서 global max 찾는데 이용
		sdata[tid].dflowmaxInThisStep = flxmax.dflow;
		sdata[tid].vmaxInThisStep = flxmax.v;
		if (gvi_k.isApplyVNC == 1) {
			sdata[tid].VNConMinInThisStep = getVNConditionValue(cvs_k, idx);
		}
	}
	__syncthreads(); // 초기화 부분에서 최대한 sync 시킨다. // 필수. 중요
	//for (int s = blockDim.x / 2; s > 0; s /= 2) {
	for (unsigned int s = blockDim.x / 2; s > 0; s >>=1) {
		if (tid < s && idx < gvi_k.nCellsInnerDomain) {
			if (sdata[tid].dflowmaxInThisStep < sdata[tid + s].dflowmaxInThisStep) {
				sdata[tid].dflowmaxInThisStep = sdata[tid + s].dflowmaxInThisStep;
			}
			if (sdata[tid].vmaxInThisStep < sdata[tid + s].vmaxInThisStep) {
				sdata[tid].vmaxInThisStep = sdata[tid + s].vmaxInThisStep;
			}
			if (gvi_k.isApplyVNC == 1) {
				if (sdata[tid].VNConMinInThisStep > sdata[tid + s].VNConMinInThisStep) {
					sdata[tid].VNConMinInThisStep = sdata[tid + s].VNConMinInThisStep;
				}
			}
		}
		__syncthreads();
	}
	if (tid == 0) {
		odata[blockIdx.x].dflowmaxInThisStep = sdata[0].dflowmaxInThisStep;
		odata[blockIdx.x].vmaxInThisStep = sdata[0].vmaxInThisStep;
		odata[blockIdx.x].VNConMinInThisStep = sdata[0].VNConMinInThisStep;
	}
	//}
}
__global__ void updateGlobalMinMaxFromArray(minMaxFlux* minMaxCVidx_k, int arraySize,
	int applyVNC, minMaxFlux* odata) {
	extern  __shared__ minMaxFlux sdata[];
	unsigned int tid = threadIdx.x;
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	sdata[tid].dflowmaxInThisStep = -9999.0; // 여기에서 초기화 해준다. 실제 배열 길이가 < tid + s 인경우에도 값을 입력..
	sdata[tid].vmaxInThisStep = -9999.0;
	sdata[tid].VNConMinInThisStep = 9999.0;
	if (idx < arraySize) {
		sdata[tid].dflowmaxInThisStep = minMaxCVidx_k[idx].dflowmaxInThisStep;
		sdata[tid].vmaxInThisStep = minMaxCVidx_k[idx].vmaxInThisStep;
		sdata[tid].VNConMinInThisStep = minMaxCVidx_k[idx].VNConMinInThisStep;
	}
	__syncthreads(); // 초기화 부분에서 최대한 sync 시킨다. // 필수. 중요
	//for (int s = blockDim.x / 2; s > 0; s /= 2) {
	for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
		if (tid < s && idx < arraySize) {
			if (sdata[tid].dflowmaxInThisStep < sdata[tid + s].dflowmaxInThisStep) {
				sdata[tid].dflowmaxInThisStep = sdata[tid + s].dflowmaxInThisStep;
			}
			if (sdata[tid].vmaxInThisStep < sdata[tid + s].vmaxInThisStep) {
				sdata[tid].vmaxInThisStep = sdata[tid + s].vmaxInThisStep;
			}
			if (applyVNC == 1) {
				if (sdata[tid].VNConMinInThisStep > sdata[tid + s].VNConMinInThisStep) {
					sdata[tid].VNConMinInThisStep = sdata[tid + s].VNConMinInThisStep;
				}
			}
		}
		__syncthreads();
	}
	if (tid == 0) {
		odata[blockIdx.x].dflowmaxInThisStep = sdata[0].dflowmaxInThisStep;
		odata[blockIdx.x].vmaxInThisStep = sdata[0].vmaxInThisStep;
		odata[blockIdx.x].VNConMinInThisStep = sdata[0].VNConMinInThisStep;
	}
}


__global__ void setAllCVFalse(cvatt* d_cvs, int arraySize) {
	int tid = blockDim.x * blockIdx.x + threadIdx.x;
	if (tid < arraySize) {
		d_cvs[tid].isSimulatingCell = 0;
	}
}


// 2021.05.28. 안정적인 __syncthreads() 를 위해서 전용으로 만듬.
__global__ void runSolver_GPU(cvatt* cvs_k, bcAppinfo* bcAppinfos_k,	double* cvsele_k, globalVinner gvi_k) {
	int nCells = gvi_k.nCellsInnerDomain;
	int igsLimit = gvi_k.iGSmaxLimit;
	int iNRLimit = gvi_k.iNRmaxLimit;
	int idx = blockDim.x * blockIdx.x + threadIdx.x;
	double bcdepth = 0.0;
	int applyBCdepth = 0; // 1 : true, 0 : false
	double c1_IM = gvi_k.dt_sec / gvi_k.dx;


	if (idx < nCells && cvs_k[idx].isBCcell == 1) {
		int bcidx = getBcAppinfoidx(bcAppinfos_k, gvi_k.bcCellCountAll, idx);
		if (bcAppinfos_k[bcidx].bctype == 2 || bcAppinfos_k[bcidx].bctype == 3) {// 1:Discharge, 2:Depth, 3:WaterLevel, 4:None
			bcdepth = bcAppinfos_k[bcidx].bcDepth_dt_m_tp1;
			applyBCdepth = 1;
		}
	}
	for (int igs = 0; igs < igsLimit; ++igs) {
		int continueNR_aCell = 1;
		double dp_old = 0.0;
		double dn = 0.0;
		if (idx < nCells && cvs_k[idx].isSimulatingCell == 1) {
			dp_old = cvs_k[idx].dp_tp1;
		}
		for (int inr = 0; inr < iNRLimit; ++inr) {
			int calNR = -1;
			if (idx < nCells && cvs_k[idx].isSimulatingCell == 1 && continueNR_aCell == 1) {
				calNR = 1;
				dn = cvs_k[idx].dp_tp1;
			}
			if (calNR == 1) { calWFlux(cvs_k, cvsele_k, gvi_k, idx); }
			__syncthreads(); //sync_01 // if 문 밖에서 써야 한다. if 문 안에서 사용하면, 무한루프 된다.
			if (calNR == 1) { calEFlux(cvs_k, cvsele_k, gvi_k, idx); }
			if (calNR == 1) { calNFlux(cvs_k, cvsele_k, gvi_k, idx);	}
			__syncthreads(); //sync_02
			if (calNR == 1) { calSFlux(cvs_k, cvsele_k, gvi_k, idx); }
			__syncthreads(); //sync_03 
			if (calNR == 1) { 
				// 현재 셀의 수위가 올라가려면  -> qe-, qw+, qs-, qn+
				double fn = dn - cvs_k[idx].dp_t + (cvs_k[idx].qe_tp1 - cvs_k[idx].qw_tp1
					+ cvs_k[idx].qs_tp1 - cvs_k[idx].qn_tp1) * c1_IM;//- sourceTerm; 
				double eElem = pow(cvs_k[idx].dfe, 2 / 3.0) * sqrt(abs(cvs_k[idx].slpe)) / cvs_k[idx].rc;
				double sElem = pow(cvs_k[idx].dfs, 2 / 3.0) * sqrt(abs(cvs_k[idx].slps)) / cvs_k[idx].rc;
				double dfn = 1 + (eElem + sElem) * (5.0 / 3.0) * c1_IM;
				if (dfn == 0.0) {
					continueNR_aCell = -1;
				}
				else {
					double dnp1 = 0.0;
					if (applyBCdepth == 1) {
						dnp1 = bcdepth;
					}
					else {
						dnp1 = dn - fn / dfn;
					}
					if (dnp1 < 0.0) { dnp1 = 0.0; }
					//double resd =;
					cvs_k[idx].dp_tp1 = dnp1;
					cvs_k[idx].hp_tp1 = cvs_k[idx].dp_tp1 + cvsele_k[idx];
					if (abs(dnp1 - dn) <= CCh) {
						continueNR_aCell = -1;
					}
				}
			}
			__syncthreads(); //sync_04
		}
		//__syncthreads(); //sync_04_01
		if (idx < nCells) {
			cvs_k[idx].resd = abs(cvs_k[idx].dp_tp1 - dp_old);
			////2021.08.04 // 2021.08.05 NR 밖에서 이것을 이용하면, 각 함수에서 v 계산하는 것 보다 느려진다.
			//cvs_k[idx].ve_tp1 = getVelocity(cvs_k[idx].qe_tp1, cvs_k[idx].dfe, cvs_k[idx].slpe, cvs_k[idx].rc);
			//cvs_k[idx].vs_tp1 = getVelocity(cvs_k[idx].qs_tp1, cvs_k[idx].dfs, cvs_k[idx].slps, cvs_k[idx].rc);
			if (cvs_k[idx].dp_tp1 > dMinLimit) {
				setEffCells(cvs_k, idx);
			}
		}
		__syncthreads();  //sync_05
	}
}
//#endif

__host__ __device__ void calWFlux(cvatt* cvs_L, double* cvsele_L, globalVinner gvi_L, int idx) {
	if (gvi_L.nCols == 1) { return; }
	flux flxw; //W, x-
	if (cvs_L[idx].colx == 0 || cvs_L[idx].cvidx_atW == -1)//w 측 경계셀
	{
		if (cvs_L[idx].isBCcell == 1) {
			flxw = noFlx(); // w측 최 경계에서는 w 방향으로 flx 없다.
		}
		else {// w측 최 경계에서는 w 방향으로 자유수면 flx 있다.
			double slp_tm1 = 0; // 2021.08.06. 인접셀 경사 적용하지 않고, bedslope 매개변수 값만 적용
			//if (cvs_L[idx].cvdix_atE >= 0)
			//{
			//	double he = cvs_L[cvs_L[idx].cvdix_atE].dp_t + cvsele_L[cvs_L[idx].cvdix_atE];
			//	double hcur = cvs_L[idx].dp_t + cvsele_L[idx];
			//	slp_tm1 = (he - hcur) / gvi_L.dx; //i+1 셀과의 e 수면경사를 w 방향에 적용한다.
			//}
			slp_tm1 = slp_tm1 + gvi_L.domainOutBedSlope;
			if (slp_tm1 >= 0.0 && cvs_L[idx].dp_tp1 > dMinLimit) {
				// slp_tm1 > 0 인 경우가 아니면, w 방향으로 흐름 없다. e 방향으로 흐른다.
				flxw = calMEq_DWEm_Deterministric(cvs_L[idx].qw_t,
					gvi_L.dt_sec, slp_tm1, cvs_L[idx].rc, cvs_L[idx].dp_tp1, 0.0);
			}
			else { flxw = noFlx(); }
		}
	}
	else {
		flxw.v = cvs_L[cvs_L[idx].cvidx_atW].ve_tp1; //2021.08.04
		flxw.wsslp = cvs_L[cvs_L[idx].cvidx_atW].slpe;
		flxw.q = cvs_L[cvs_L[idx].cvidx_atW].qe_tp1;
		flxw.dflow = cvs_L[cvs_L[idx].cvidx_atW].dfe;
	}
	cvs_L[idx].qw_tp1 = flxw.q;
	cvs_L[idx].vw_tp1 = flxw.v;
	//cvs_L[idx].slpw = flxw.wsslp;
	cvs_L[idx].dfw = flxw.dflow;
}

__host__ __device__ void calEFlux(cvatt* cvs_L, double* cvsele_L, globalVinner gvi_L, int idx) {
	if (gvi_L.nCols == 1) { return; }
	flux flxe;    //E,  x+
	if (cvs_L[idx].colx == (gvi_L.nCols - 1) || cvs_L[idx].cvdix_atE == -1) {
		if (cvs_L[idx].isBCcell == 1) { flxe = noFlx(); }
		else {
			double slp_tm1 = 0.0;
			//if (cvs_L[idx].cvidx_atW >= 0) {
			//	double hw = cvs_L[cvs_L[idx].cvidx_atW].dp_t + cvsele_L[cvs_L[idx].cvidx_atW];
			//	double hcur = cvs_L[idx].dp_t + cvsele_L[idx];
			//	slp_tm1 = (hcur - hw) / gvi_L.dx;
			//}
			slp_tm1 = slp_tm1 - gvi_L.domainOutBedSlope;
			if (slp_tm1 <= 0.0 && cvs_L[idx].dp_tp1 > dMinLimit) {
				// slp_tm1 < 0 인 경우가 아니면, e 방향으로 흐름 없다. w 방향으로 흐른다.
				flxe = calMEq_DWEm_Deterministric(cvs_L[idx].qe_t,
					gvi_L.dt_sec, slp_tm1, cvs_L[idx].rc, cvs_L[idx].dp_tp1, 0.0);
			}
			else { flxe = noFlx(); }
		}
	}
	else {
		flxe = getFluxToEorS(cvs_L, cvsele_L, gvi_L, idx, cvs_L[idx].cvdix_atE, 1);
	}
	cvs_L[idx].ve_tp1 = flxe.v; //2021.08.04
	cvs_L[idx].dfe = flxe.dflow;
	cvs_L[idx].slpe = flxe.wsslp;
	cvs_L[idx].qe_tp1 = flxe.q;
}

__host__ __device__ void calNFlux(cvatt* cvs_L, double* cvsele_L, globalVinner gvi_L, int idx) {
	if (gvi_L.nRows == 1) { return; }
	flux flxn;  //N, y-
	if (cvs_L[idx].rowy == 0 || cvs_L[idx].cvidx_atN == -1) {
		if (cvs_L[idx].isBCcell == 1) { flxn = noFlx(); }
		else {// n측 최 경계에서는 n 방향으로 자유수면 flx 있다.
			double slp_tm1 = 0.0;
			//if (cvs_L[idx].cvidx_atS >= 0) {
			//	//double slp_tm1 = (cvs[cvs[idx].cvaryNum_atS].hp_t - cvs[idx].hp_t) / gv.dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
			//	double hs = cvs_L[cvs_L[idx].cvidx_atS].dp_t + cvsele_L[cvs_L[idx].cvidx_atS];
			//	double hcur = cvs_L[idx].dp_t + cvsele_L[idx];
			//	slp_tm1 = (hs - hcur) / gvi_L.dx;
			//}
			slp_tm1 = slp_tm1 + gvi_L.domainOutBedSlope;
			if (slp_tm1 >= 0.0 && cvs_L[idx].dp_tp1 > dMinLimit) {
				// slp_tm1 > 0 인 경우가 아니면, n 방향으로 흐름 없다. s 방향으로 흐른다.
				flxn = calMEq_DWEm_Deterministric(cvs_L[idx].qn_t,
					gvi_L.dt_sec, slp_tm1, cvs_L[idx].rc, cvs_L[idx].dp_tp1, 0.0);
			}
			else { flxn = noFlx(); }
		}
	}
	else {
		flxn.v = cvs_L[cvs_L[idx].cvidx_atN].vs_tp1; //2021.08.04
		flxn.wsslp = cvs_L[cvs_L[idx].cvidx_atN].slps;
		flxn.dflow = cvs_L[cvs_L[idx].cvidx_atN].dfs;
		flxn.q = cvs_L[cvs_L[idx].cvidx_atN].qs_tp1;
	}
	cvs_L[idx].qn_tp1 = flxn.q;
	cvs_L[idx].vn_tp1 = flxn.v;
	cvs_L[idx].dfn = flxn.dflow;
}

__host__ __device__ void calSFlux(cvatt* cvs_L, double* cvsele_L, globalVinner gvi_L, int idx) {
	if (gvi_L.nRows == 1) { return; }
	flux flxs;//S, y+
	if (cvs_L[idx].rowy == (gvi_L.nRows - 1)
		|| cvs_L[idx].cvidx_atS == -1) {
		if (cvs_L[idx].isBCcell == 1) { flxs = noFlx(); }
		else {
			double slp_tm1 = 0.0;
			//if (cvs_L[idx].cvidx_atN >= 0) {
			//	double hn = cvs_L[cvs_L[idx].cvidx_atN].dp_t + cvsele_L[cvs_L[idx].cvidx_atN];
			//	double hcur = cvs_L[idx].dp_t + cvsele_L[idx];
			//	slp_tm1 = (hcur - hn) / gvi_L.dx;
			//}
			slp_tm1 = slp_tm1 - gvi_L.domainOutBedSlope;
			if (slp_tm1 <= 0.0 && cvs_L[idx].dp_tp1 > dMinLimit) {
				// slp_tm1 < 0 인 경우가 아니면, s 방향으로 흐름 없다. n 방향으로 흐른다.
				flxs = calMEq_DWEm_Deterministric(cvs_L[idx].qs_t,
					gvi_L.dt_sec, slp_tm1, cvs_L[idx].rc, cvs_L[idx].dp_tp1, 0.0);
			}
			else { flxs = noFlx(); }
		}
	}
	else {
		flxs = getFluxToEorS(cvs_L, cvsele_L, gvi_L, idx, cvs_L[idx].cvidx_atS, 3);
	}
	cvs_L[idx].vs_tp1 = flxs.v; //2021.08.04
	cvs_L[idx].dfs = flxs.dflow;
	cvs_L[idx].slps = flxs.wsslp;
	cvs_L[idx].qs_tp1 = flxs.q;
}

__host__ __device__ flux calMEq_DWEm_Deterministric(double qt, 
	double dt_sec, double slp, double rc, double dflow, double qt_ip1){
	flux flx;
	double qapp = qt;
	//double q = (qapp - (gravity * dflow * dt_sec * slp)) /
	//         (1 + gravity * dt_sec * (rc * rc) * DeviceFunction.Sqrt((qapp * qapp + qt_ip1 * qt_ip1) / 2.0) 
	//         / pow(dflow, 7 / 3.0));

	double q = (qapp - (GRAVITY * dflow * dt_sec * slp)) /
		(1 + GRAVITY * dt_sec * (rc * rc) * abs(qapp) / pow(dflow, 7.0 / 3.0));

	flx.q = q;
	flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = pow(dflow, 2 / 3.0) * sqrt(abs(slp)) / mN; 
	flx.dflow = dflow;
	flx.wsslp = slp;
	return flx; ;
}

//targetCellDir : E = 1, S = 3, W = 5, N = 7, NONE = 0
__host__ __device__ flux getFluxToEorS(cvatt* cvs_L, double* cvsele_L,
	globalVinner gvi_L, int idxc, int idxt, int targetCellDir)
{
	cvatt curCell = cvs_L[idxc];
	cvatt tarCell = cvs_L[idxt];
	double slp = 0.0;
	double dhtp1 = tarCell.hp_tp1 - curCell.hp_tp1;
	if (dhtp1 == 0.0) { return noFlx(); }
	if (dhtp1 > 0.0
		&& tarCell.dp_tp1 <=dMinLimit) {
		return noFlx();
	}
	if (dhtp1 < 0.0
		&& curCell.dp_tp1 <= dMinLimit) {
		return noFlx();
	}
	slp = dhtp1 / gvi_L.dx;
	if (abs(slp) <= slpMIN) { return noFlx(); }
	double dflow = fmax(curCell.hp_tp1, tarCell.hp_tp1)
		- fmax(cvsele_L[idxc], cvsele_L[idxt]);
	// 최대 수심법
	//dflow = DeviceFunction.Max(curCell.hp_tp1, tarCell.hp_tp1); 
	//// 수심평균 법
	//double maxBedElev = DeviceFunction.Max(curCell.elez, tarCell.elez);
	//double d1 = curCell.hp_tp1 - maxBedElev;
	//if (d1 < 0) { d1 = 0; }
	//double d2 = tarCell.hp_tp1 - maxBedElev;
	//if (d2 < 0) { d2 = 0; }
	//dflow = (d1 + d2) / 2.0;
	//// 수심평균 법
	if (dflow <= 0.0) { return noFlx(); }
	double qt = 0.0;
	double qtp1 = 0.0;
	double q_ip1 = 0.0;
	double u_ip1 = 0.0;
	if (targetCellDir == 1) {
		qt = curCell.qe_t;
		qtp1 = curCell.qe_tp1; 
		u_ip1 = tarCell.ve_tp1; q_ip1 = tarCell.qe_tp1;
	}
	else { // 1 or 3 이 들어온다.
		qt = curCell.qs_t;
		qtp1 = curCell.qs_tp1;
		u_ip1 = tarCell.vs_tp1; q_ip1 = tarCell.qs_tp1;
	}
	flux flx;
#ifdef isDWE
		flx = calMEq_DWE_Deterministric(qt, dflow,
			slp, curCell.rc, gvi_L.dx, gvi_L.dt_sec, q_ip1, u_ip1);
#else
		flx = calMEq_DWEm_Deterministric(qt,
			gvi_L.dt_sec, slp, curCell.rc, dflow, q_ip1);
#endif

#ifdef isAS
#else
		if (abs(flx.q) > 0.0) {
			flx = getFluxUsingSubCriticalCon(flx, gvi_L.froudeNCriteria);
			flx = getFluxUsingFluxLimit(flx, gvi_L.dx, gvi_L.dt_sec);
			//flx = getFluxqUsingFourDirLimitUsingDepthCondition(currentCell, flx, dflow, dx, dt_sec); //이건 수렴이 잘 안된다.
			//flx = getFluxUsingFourDirLimitUsingCellDepth(currentCell, targetCell, flx, dx, dt_sec);
			//flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
		}
#endif
	//flx.slp = slp;
	return flx;
}

__host__ __device__ flux calMEq_DWE_Deterministric(double qt, double dflow,
	double slp, double rc, float dx, double dt_sec, double q_ip1, double u_ip1)
{
	flux flx;
	double qapp = qt; //Math.Abs(qt);
	//2019.1.2 관성이 없을 경우에는 
	// slp가 + 면 q는 -, slp가 - 이면 q는 + 가 되어야 함.
	// 이전 t에서 q 가  0 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 무조건 +.
	// 이전 t에서 q 가  - 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 - 일수도 있고, + 일수도 있음. => 조건 처리 필요
	// 이전 t에서 q 가 + 이면, slp가 + 일때 q는 - 일수도 있고, + 일수도 있음, slp가 - 일때는 q는 무조건 +. => 조건 처리 필요

	double ut = qapp / dflow;
	
	double q = (qapp - (GRAVITY * dflow * dt_sec * slp)) /
		(1 + ut * dt_sec / dx + GRAVITY * dt_sec * (rc * rc) * abs(qapp) / pow(dflow, 7.0 / 3.0));
	//double q = ((qapp - q_ip1 * u_ip1 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
	//                (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * abs(qapp) 
	//                / DeviceFunction.Pow(dflow, 7.0 / 3.0)));
	//double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2.0) * (u_ip1+ut)/2.0 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
	//              (1 - (u_ip1 + ut) / 2.0 * dt_sec / dx + gravity * dt_sec * (rc * rc) * Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2.0) 
	//             / DeviceFunction.Pow(dflow, 7.0 / 3.0)));
	//double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2.0) * (u_ip1 + ut) / 2.0 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
	//               (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2.0) 
	//              / DeviceFunction.Pow(dflow, 7.0 / 3.0)));
	//double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2.0) * ut * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
	//   (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * qapp / DeviceFunction.Pow(dflow, 7.0 / 3.0)));

	flx.q = q;
	flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = pow(dflow, 2 / 3) * sqrt(abs(slp)) / mN; 
	flx.dflow = dflow;
	flx.wsslp = slp;
	return flx; ;
}

__host__ __device__ flux getFluxUsingSubCriticalCon(flux inflx, float froudNCriteria){
	double v_wave = sqrt(GRAVITY * inflx.dflow);
	double fn = abs(inflx.v) / v_wave;
	if (fn > froudNCriteria) {
		double v = froudNCriteria * v_wave;
		if (inflx.q < 0.0) { v = -1.0 * v; }
		inflx.q = inflx.v * inflx.dflow;
		inflx.v = v;
	}
	return inflx;
}

__host__ __device__ flux getFluxUsingFluxLimit(flux inflx, 
	float dx, double dt_sec){
	double qmax = inflx.dflow * dx / 2.0 / dt_sec; // 수위차의 1/2 이 아니라, 흐름 수심의 1/2이므로, 수위 역전 될 수 있다.
	if (abs(inflx.q) > qmax) {
		if (inflx.q < 0.0) {
			inflx.q = -1.0 * qmax; 
		}
		else {
			inflx.q = qmax;
		}
		inflx.v = inflx.q / inflx.dflow;
	}
	return inflx;
}

__host__ __device__ void initializeThisStepAcell(cvatt* cvs_L, cvattAddAtt* cvsAA_L,
	bcAppinfo* bcAppinfos_L, double elev, double rfi_read_mPs_L,
	int idx, thisProcessInner psi_L, globalVinner gvi_L)
{
	double h = cvs_L[idx].dp_tp1 + elev;// cvs_L[idx].elez; //elev 가 변경되는 경우가 있으므로, 이렇게 수위설정
	if (cvs_L[idx].hp_tp1 <= h) { // 지면고가 높아진 경우
		// dem  고도 변경되면, 수심이 바뀐다. 수위는 유지.
		// cvs_L[idx].hp_t=cvs_L[idx].elez + cvs_L[idx].dp_t 이므로, cvs_L[idx].dp_t 이값과 cvs_L[idx].dp_tp1  모두 업데이트 해줘야 한다.
		cvs_L[idx].dp_tp1 = cvs_L[idx].hp_tp1 - elev;
		if (cvs_L[idx].dp_tp1 < 0.0) { cvs_L[idx].dp_tp1 = 0.0; }
		cvs_L[idx].dp_t = cvs_L[idx].dp_tp1;
	}
	else {
		cvs_L[idx].dp_t = cvs_L[idx].dp_tp1;
	}
	cvs_L[idx].qe_t = cvs_L[idx].qe_tp1;
	cvs_L[idx].qw_t = cvs_L[idx].qw_tp1;
	cvs_L[idx].qs_t = cvs_L[idx].qs_tp1;
	cvs_L[idx].qn_t = cvs_L[idx].qn_tp1;
	double sourceAlltoRoute_tp1_dt_m = 0.0;
	if (cvs_L[idx].isBCcell == 1) { // prj.isbcApplied == 1 조건은 보장됨
		int bci = getBcAppinfoidx(bcAppinfos_L, gvi_L.bcCellCountAll, idx);
		bcAppinfos_L[bci].bcDepth_dt_m_tp1 = getCDasDepthWithLinear(bcAppinfos_L[bci].bctype,
			bcAppinfos_L[bci].bcData_curOrder, bcAppinfos_L[bci].bcData_nextOrder,
			bcAppinfos_L[bci].bcData_curOrderStartedTime_sec, elev, psi_L.tnow_sec, gvi_L);
		if (bcAppinfos_L[bci].bctype == 1)//1:  Discharge,  2: Depth, 3: WaterLevel,  4: None
		{//경계조건이 유량일 경우, 소스항에 넣어서 홍수추적한다. 수심으로 환산된 유량..
			sourceAlltoRoute_tp1_dt_m = bcAppinfos_L[bci].bcDepth_dt_m_tp1;
		}
		else
		{//경계조건이 유량이 아닐경우, 홍수추적 하지 않고, 고정된 값 적용.
			cvs_L[idx].dp_tp1 = bcAppinfos_L[bci].bcDepth_dt_m_tp1;
			if (psi_L.tnow_sec == 0.0) {
				cvs_L[idx].dp_t = cvs_L[idx].dp_tp1;
			}
		}
	}
	cvsAA_L[idx].sourceRFapp_dt_meter = 0.0;
	//-1, 0 :false, 1: true
	if (psi_L.isRFApplied == 1 && psi_L.rfEnded == 0.0)
	{
		if (psi_L.rfType == weatherDataType::Raster_ASC) {
			cvsAA_L[idx].sourceRFapp_dt_meter = rfi_read_mPs_L * gvi_L.dt_sec;
		}
		else {
			cvsAA_L[idx].sourceRFapp_dt_meter = psi_L.rfReadintensityForMAP_mPs * gvi_L.dt_sec;
		}
	}
	sourceAlltoRoute_tp1_dt_m = sourceAlltoRoute_tp1_dt_m + cvsAA_L[idx].sourceRFapp_dt_meter;
	cvs_L[idx].dp_t = cvs_L[idx].dp_t + sourceAlltoRoute_tp1_dt_m;
	cvs_L[idx].dp_tp1 = cvs_L[idx].dp_tp1 + sourceAlltoRoute_tp1_dt_m;
	cvs_L[idx].hp_tp1 = cvs_L[idx].dp_tp1 + elev;
}

__host__ __device__ void setStartingConditionCVs_inner(cvatt* cvs_L, cvattAddAtt* cvsAA_L,
	double* cvselez_L, int idx) {
	cvs_L[idx].dp_t = cvsAA_L[idx].initialConditionDepth_m;
	cvs_L[idx].dp_tp1 = cvs_L[idx].dp_t;
	cvs_L[idx].ve_tp1 = 0.0;
	cvs_L[idx].qe_tp1 = 0.0;
	cvs_L[idx].qw_tp1 = 0.0;
	cvs_L[idx].qn_tp1 = 0.0;
	cvs_L[idx].qs_tp1 = 0.0;
	//cvs_L[idx].hp_tp1 = cvs_L[idx].dp_tp1 + cvs_L[idx].elez;
	cvs_L[idx].hp_tp1 = cvs_L[idx].dp_tp1 + cvselez_L[idx];
	//cvsAA_L[idx].fdmaxV = 0;//E = 1, S = 3, W = 5, N = 7, NONE = 0
	//cvsAA_L[idx].bcData_curOrder = 0;
	cvsAA_L[idx].sourceRFapp_dt_meter = 0.0;
	cvsAA_L[idx].rfAccCell = 0.0;
	cvsAA_L[idx].saturatedByCellRF = 0.0;
	//cvsAA_L[idx].rfReadintensity_mPsec = 0;
	cvs_L[idx].isSimulatingCell = 0;
}

__host__ __device__ double getCDasDepthWithLinear(int bctype, double vcurOrder, double vnextOrder,
	int t_curOrderStarted_sec, double elev_m, double tnow_sec, globalVinner gvi_L)
{
	double valueAsDepth_curOrder = 0.0;
	double valueAsDepth_nextOrder = 0.0;
	double dx = gvi_L.dx;
	double dt_s = gvi_L.dt_sec;
	//1:  Discharge,  2: Depth, 3: WaterLevel,  4: None
	switch (bctype)
	{
	case 1://conditionDataType::Discharge:
		valueAsDepth_curOrder = (vcurOrder / dx / dx) * dt_s;
		valueAsDepth_nextOrder = (vnextOrder / dx / dx) * dt_s;
		break;
	case 2://conditionDataType::Depth:
		valueAsDepth_curOrder = vcurOrder;
		valueAsDepth_nextOrder = vnextOrder;
		break;
	case 3://conditionDataType::WaterLevel:
		valueAsDepth_curOrder = vcurOrder - elev_m;
		valueAsDepth_nextOrder = vnextOrder - elev_m;
		break;
	}
	if (valueAsDepth_curOrder < 0.0) { valueAsDepth_curOrder = 0.0; }
	if (valueAsDepth_nextOrder < 0.0) { valueAsDepth_nextOrder = 0.0; }
	double bcDepth_dt_m_tp1 = 0.0;
#ifndef isAS // 해석해 테스트가 아닐때는 이 조건 사용
		bcDepth_dt_m_tp1 = (valueAsDepth_nextOrder - valueAsDepth_curOrder)
			* (tnow_sec - t_curOrderStarted_sec) / gvi_L.dtbc_sec
			+ valueAsDepth_curOrder;
#else
		bcDepth_dt_m_tp1 = valueAsDepth_curOrder; // 해석해 테스트는 이 조건
#endif
	return  bcDepth_dt_m_tp1;
}

__host__ __device__ int getBcAppinfoidx(bcAppinfo * bcAppinfos, int bcCellCountAll, int cvidxToGet) {

	for (int i = 0; i < bcCellCountAll; ++i) {
		if (bcAppinfos[i].cvidx == cvidxToGet) {
			return i;
		}
	}
	return -1;
}

__host__ __device__ void setEffCells(cvatt * cvs_L, int idx)
{
	cvs_L[idx].isSimulatingCell = 1;
	if (cvs_L[idx].cvdix_atE >= 0) {
		cvs_L[cvs_L[idx].cvdix_atE].isSimulatingCell = 1;
	}
	if (cvs_L[idx].cvidx_atW >= 0) {
		cvs_L[cvs_L[idx].cvidx_atW].isSimulatingCell = 1;
	}
	if (cvs_L[idx].cvidx_atN >= 0) {
		cvs_L[cvs_L[idx].cvidx_atN].isSimulatingCell = 1;
	}
	if (cvs_L[idx].cvidx_atS >= 0) {
		cvs_L[cvs_L[idx].cvidx_atS].isSimulatingCell = 1;
	}
}

// E에서 시작해서 시계방향으로 첫번째 최대값 방향 반환
__host__ __device__ fluxNfd get_maxFlux_FD(cvatt* cvs_L, int i) {
	fluxNfd flxmxfd;
	double ve = cvs_L[i].ve_tp1;
	double vs = cvs_L[i].vs_tp1;
	double avw = abs(cvs_L[i].vw_tp1);
	double avn = abs(cvs_L[i].vn_tp1);
	double vmax = 0.0;
	int fdMaxV = 0; //	flowDirection8G2D :: E1 = 1, SE2 = 2, S3 = 3, SW4 = 4, W5 = 5, NW6 = 6, N7 = 7, NE8 = 8, NONE = 0
	if (ve > 0) {
		vmax = ve;
		fdMaxV = 1;
	}
	if (cvs_L[i].vw_tp1 < 0 && avw > vmax) { // 이경우가 셀 밖으로 유속 있는 경우
		vmax = avw;
		fdMaxV = 5;
	}
	if (vs > vmax) { // 이러면 vs>0 보장
		vmax = vs;
		fdMaxV = 3;
	}
	if (cvs_L[i].vn_tp1 < 0 && avn > vmax) {// 이경우가 셀 밖으로 유속 있는 경우
		vmax = avn;
		fdMaxV = 7;
	}
	flxmxfd.fd_maxv = fdMaxV;
	flxmxfd.v = vmax;

	// 여기서 부터 최대 흐름 수심 찾고, 최대 유량 방향
	if (vmax == 0) {
		flxmxfd.dflow = 0;
		flxmxfd.q = 0;
		flxmxfd.fd_maxq = 0; //flowDirection8G2D::NONE ==0
	}
	else {

		double dflow_max = 0.0;
		dflow_max = cvs_L[i].dfe;
		if (cvs_L[i].dfw > dflow_max) {
			dflow_max = cvs_L[i].dfw;
		}
		if (cvs_L[i].dfn > dflow_max) {
			dflow_max = cvs_L[i].dfn;
		}
		if (cvs_L[i].dfs > dflow_max) {
			dflow_max = cvs_L[i].dfs;
		}
		flxmxfd.dflow = dflow_max; // 여기. 최대 흐름 수심

		double qe = cvs_L[i].qe_tp1;
		double qs = cvs_L[i].qs_tp1;
		double aqw = abs(cvs_L[i].qw_tp1);
		double aqn = abs(cvs_L[i].qn_tp1);
		double qmax = 0.0;
		int fd_maxq = 0; //	flowDirection8G2D :: E1 = 1, SE2 = 2, S3 = 3, SW4 = 4, W5 = 5, NW6 = 6, N7 = 7, NE8 = 8, NONE = 0

		if (qe > 0) {
			qmax = qe;
			fd_maxq = 1;
		}
		if (cvs_L[i].qw_tp1 < 0 && aqw>qmax) { // 이경우가 셀 밖으로 유량 있는 경우
			qmax = aqw;
			fd_maxq = 5;
		}
		if (qs > qmax) { // 이러면 vs>0 보장
			qmax = qs;
			fd_maxq = 3;
		}
		if (cvs_L[i].qn_tp1 < 0 && aqn>qmax) {// 이경우가 셀 밖으로 유량 있는 경우
			qmax = aqn;
			fd_maxq = 7;
		}
		flxmxfd.q = qmax;
		flxmxfd.fd_maxq = fd_maxq; //flowDirection8G2D::NONE ==0
	}
	return flxmxfd;
}


__host__ __device__ double getVNConditionValue(cvatt* cvs_L, int i) {
	double searchMIN = DBL_MAX;
	double curValue = 0.0;
	double rc = cvs_L[i].rc;
	// e 값과 중복되므로, w는 계산하지 않는다.
	if (cvs_L[i].dfe > 0.0) {
		searchMIN = 2.0 * rc * sqrt(abs(cvs_L[i].slpe))
			/ pow(cvs_L[i].dfe, 5.0 / 3.0);
	}
	// s 값과 중복되므로, n는 계산하지 않는다.
	if (cvs_L[i].dfs > 0.0) {
		curValue = 2.0 * rc * sqrt(abs(cvs_L[i].slps))
			/ pow(cvs_L[i].dfs, 5.0 / 3.0);
		if (curValue < searchMIN) {
			searchMIN = curValue;
		}
	}
	return searchMIN;
}

// 2021.08.05 NR 밖에서 이것을 이용하면, 각 함수에서 v 계산하는 것 보다 느려진다.
__host__ __device__ double getVelocity(double q, double dflow, double slp, double rc) {
	if (dflow <= 0.0) {
		return 0;
	}
	else {
		double v1 = q / dflow;  // dflow가 아주 작은 경우 발산한다.
		return v1;

	}
	//double v2 = pow(dflow, 2 / 3.0) * sqrt(abs(slp)) / rc;	 // 항상 +
	 //double v2 = sqrt(GRAVITY * dflow); // 항상 +
	 //double v = min(abs(v1), v2); // 항상 +
	 ////if (v > 9) {
	 ////	int a = 1;
	 ////}
	 //if (q < 0) {
	 //	v = -1.0 * v;
	 //}
	 //return v;
}