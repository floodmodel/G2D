#pragma once

#include <cuda.h>
#include <cuda_runtime.h>
#include "g2d.h"
#include "gentle.h"



inline void error_check(cudaError_t err, const char* file, int line) {
	if (err != cudaSuccess) {
		printf( "CUDA ERROR at %s[%d] : %s\n", file, line, cudaGetErrorString(err));
		abort();
	}
}
#define CUDA_CHECK(err) error_check(err, __FILE__, __LINE__); 

//inline void error_check(cudaError_t err, const char* file, int line) {
//	if (err != cudaSuccess) {
//		::fprintf(stderr, "CUDA ERROR at %s[%d] : %s\n", file, line, cudaGetErrorString(err));
//		abort();
//	}
//}
//#define CUDA_CHECK(err) do { error_check(err, __FILE__, __LINE__); } while(0)

__host__ __device__ fluxData calMEq_DWEm_Deterministric(double qt,
	double dt_sec, double slp, double rc, double dflow, double qt_ip1);
__host__ __device__ fluxData calMEq_DWE_Deterministric(double qt,
	double dflow, double slp, double rc,
	float dx, double dt_sec, double q_ip1, double u_ip1);
__host__ __device__ void calEFlux(cvatt* cvs_L, double* cvsele_L,
	globalVinner gvi_L, int idx);
__host__ __device__ void calWFlux(cvatt* cvs_L, double* cvsele_L,
	globalVinner gvi_L, int idx);
__host__ __device__ void calNFlux(cvatt* cvs_L, double* cvsele_L,
	globalVinner gvi_L, int idx);
__host__ __device__ void calSFlux(cvatt* cvs_L, double* cvsele_L,
	globalVinner gvi_L, int idx);
__host__ __device__ int getBcAppinfoidx(bcAppinfo* bcAppinfos, 
	int bcCellCountAll, int cvidToGet);;
__host__ __device__ double getCDasDepthWithLinear(int bctype,
	double vcurOrder, double vnextOrder,
	int t_curOrderStarted_sec, double elev_m,
	double tnow_sec, globalVinner gvi);
__host__ __device__ fluxData getFD4MaxValues_inner(cvatt* cvs_L,
	int ip, int iw, int in);
__host__ __device__ fluxData getFD4MaxValues(cvatt* cvs_L, int i);
__host__ __device__ fluxData getFluxToEorS(cvatt* cvs_L,
	double* cvsele_L, globalVinner gvi_L, int idxc,
	int idxt, int targetCellDir);
__host__ __device__ fluxData getFluxUsingFluxLimit(fluxData inflx,
	float dx, double dt_sec);
__host__ __device__ fluxData getFluxUsingSubCriticalCon(fluxData inflx,
	float froudNCriteria);
__global__ void getMinMaxFromCV(cvatt* cvs_k, cvattAddAtt* cvsAA_k,
	globalVinner gvi_k, minMaxCVidx* ominMaxCVidx);
__global__ void getMinMaxFromArray(minMaxCVidx* minMaxCVidx_k, int arraySize,
	int applyVNC, minMaxCVidx* ominMaxCVidx);
__host__ __device__ double getVelocity(double q, double dflow, 
	double slp, double rc);
__host__ __device__ double getVNConditionValue(cvatt* cvs_L, int i);
__host__ __device__ void initializeThisStepAcell(cvatt* cvs_L,
	cvattAddAtt* cvsAA_L, bcAppinfo* bcApp_L, double elevz, 
	double rfi_read_mPs_L, int idx, 
	thisProcessInner psi, globalVinner gvi_L);
__global__ void initilizeThisStep_GPU(cvatt* d_cvs, cvattAddAtt* d_cvsAA, 
	double* d_cvsele,
	bcAppinfo* d_bcApp, double* d_rfi_read_mPs,
	thisProcessInner psi_k, globalVinner gvi_k);
__global__ void runSolver_GPU(cvatt* cvs_k, bcAppinfo* bcAppinfos_k, 
	double* cvsele_k, globalVinner gvi_k);
__host__ __device__ void setEffCells(cvatt* cvs_L, int i);
__global__ void setStartingConditionCVs_GPU(cvatt* d_cvs, 
	cvattAddAtt* d_cvsAA, double* d_cvsele, int arraySize);
__global__ void setAllCVFalse(cvatt* d_cvs, int arraySize);
__host__ __device__ void setStartingConditionCVs_inner(cvatt* cvs_L,
	cvattAddAtt* cvsAA_L, double* cvselez_k, int idx);

__host__ __device__ inline fluxData noFlx() {
	fluxData flx; // 여기서 0으로 초기화 된다.
	return flx;
}
