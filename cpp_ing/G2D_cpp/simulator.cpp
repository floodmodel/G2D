
#include <stdio.h>
#include <ATLComTime.h>
#include <math.h>
#include "gentle.h"
#include "g2d.h"

using namespace std;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;

extern projectFile prj;
extern generalEnv ge;
extern domaininfo di;
extern domainCell **dmcells;
extern cvatt *cvs;
extern cvattAdd *cvsAA;
extern vector<rainfallinfo> rf;
extern bcCellinfo* bci;

thisProcess ps;
thisProcessInner psi;
globalVinner gvi[1];


int simulationControlUsingCPUnGPU()
{
	int	nRows = di.nRows;
	int nCols = di.nCols;
	float dx = di.dx;
	//	mSolver = new G2DCore.cSolver(prj.domain);
	//	msimulSetting = new cSimulationSetting(prj);
	double simDur_min = prj.simDuration_min + 1.0;
	int bcDataOrder = 0;
	int rfDataOrder = 0;
	int rfEnded = 1;
	int demToChangeEnded = 1;
	ps.tsec_targetToprint = 0;
	ps.tnow_sec = 0;
	psi.effCellCount = 0;
	double tnow_min_bak = 0;
	int dtbc_sec = prj.bcDataInterval_min * 60;
	int dtbc_min = prj.bcDataInterval_min;
	// gpu에서 구조체 변수 업데이트 안됨 (c#). 그래서 구조체 배열로 사용. 
	// C++에서 포인터 혹은 구조체 로 전환시 gpu 코딩에서 변수 업데이트 되는지 확인 필요
	int onCPU = 1;
	ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
	ps.simulationStartTime = COleDateTime::GetCurrentTime();
	if (prj.isRainfallApplied == 1) {
		rfEnded = -1;
	}
	if (prj.isDEMtoChangeApplied == 1) {
		demToChangeEnded = -1;
	}
	gvi[0] = initGlobalVinner();
	if (setStartingConditionUsingCPU() == -1) { return -1; }

	do //모의 시작할 때 t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
	{
		ps.tnow_min = ps.tnow_sec / 60.0;
		//이건 경계조건 등
		if (prj.isbcApplied == 1) {
			if (fmod(ps.tnow_min, dtbc_min) == 0.0 ||
				((tnow_min_bak < (bcDataOrder * dtbc_min)) &
				(ps.tnow_min >= bcDataOrder * dtbc_min)))
			{
				bcDataOrder++;
				getCellConditionData(bcDataOrder, dtbc_min);
			}
		}
		//이건 강우
		if (prj.isRainfallApplied == 1 && rfEnded == -1) {
			if (fmod(ps.tnow_min, prj.rainfallDataInterval_min == 0) ||
				((tnow_min_bak < (rfDataOrder * prj.rainfallDataInterval_min)) &
				(ps.tnow_min >= rfDataOrder * prj.rainfallDataInterval_min)))
			{
				psi.rfisGreaterThanZero = -1;
				rfDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
				rfEnded = readRainfallAndGetIntensity(rfDataOrder);
				// 0보다 큰 강우가 하나라도 있으면...
				if (psi.rfisGreaterThanZero == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori;
				}
				//강우가 없을때는 최소수심을 좀 크게 잡아도 된다.
				if (psi.rfisGreaterThanZero == -1 || rfEnded == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0f;
				}
			}
		}
		//이건  dem file 교체
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == -1 && ps.tnow_min > 0) {
			demToChangeEnded = changeDomainElevWithDEMFile(ps.tnow_min, tnow_min_bak);
		}
		initilizeThisStep(psi.dt_sec, ps.tnow_sec, dtbc_sec, rfEnded);

		//	// gpu 옵션이 true 인 경우에도 지정셀(모로코에서는 40,000 개 가 적당) 셀 이상을 모의할 때만 gpu를 사용한다.
		//	// 모의 대상 셀 개수가 작을 때는 cpu 가 더 빠르다.
		//	if (cGenEnv.usingGPU == 1 && cThisProcess.effCellCount > cGenEnv.EffCellThresholdForGPU)
		//	{
		//		//sw.Start();
		//		if (onCPU == 1)
		//		{
		//			cGenEnv.writelog("Calculation was converted into GPU. ", true);
		//			onCPU = -1;
		//		}
		//		mSolver.RunSolverUsingGPU(cvs, cvsadd, ref gv, bcCellinfo);
		//		//sw.Stop();
		//		//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingGPU, elaplsed time [ms] : " +
		//		//                       sw.Elapsed.TotalMilliseconds.ToString()  + "\r\n");
		//		//sw.Reset();
		//	}
		//	else
		//	{
		//		//sw.Start();
		//		if (onCPU == -1)
		//		{
		//			cGenEnv.writelog("Calculation was converted into CPU. ", true);
		//			onCPU = 1;
		//		}
		//		mSolver.RunSolverUsing1DArray(cvs, cvsadd, gv[0].dt_sec, gv, bcCellinfo);
		//		//sw.Stop();
		//		//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingCPU, elaplsed time [ms] : " +
		//		//    sw.Elapsed.TotalMilliseconds.ToString() + " simCell :  " + cGenEnv.effCellCount.ToString() + "\r\n");
		//		//sw.Reset();
		//	}
		//	// 여기까지..

		//	cGenEnv.iGS = gv[0].iGS;
		//	cGenEnv.iNR = gv[0].iNR;
		//	if (cGenEnv.tnow_sec >= cGenEnv.tsec_targetToprint)
		//	{
		//		checkEffetiveCellNumberAndSetAllFlase(cvs);// 매번 업데이트 하지 않고, 출력할때 마다 이 정보 업데이트 한다.
		//		prj.output.makeOutputFilesUsing1DArray(cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_printout_min, prj.domain.nodata_value);
		//		SimulationStep(cGenEnv.tnow_min);
		//		if (onCPU == 1) { Gpu.FreeAllImplicitMemory(true); }
		//		if (UpdateSimulaltionParameters(prj, Path.Combine(prj.prjFilePath, prj.prjFileName)) == false) { return false; } //한번 출력할때 마다 모의변수 업데이트
		//		cGenEnv.tsec_targetToprint = cGenEnv.tsec_targetToprint + cGenEnv.dt_printout_sec;
		//		cGenEnv.thisPrintStepStartTime = DateTime.Now;
		//	}
		tnow_min_bak = ps.tnow_min;
		ps.tnow_sec = ps.tnow_sec + psi.dt_sec;
		//	if (cGenEnv.isfixeddt == -1)
		//	{
		//		cGenEnv.dt_sec = cHydro.getDTsecUsingConstraints(bcCellinfo, cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_sec, cHydro.cflNumber, dx,
		//			cGenEnv.gravity, cGenEnv.dflowmaxInThisStep, cGenEnv.vmaxInThisStep, cGenEnv.VNConMinInThisStep,
		//			cGenEnv.dt_printout_min * 30, bcdt_min * 30, prj.rainfallInterval_min * 30, cHydro.applyVNC);
		//		gv[0].dt_sec = cGenEnv.dt_sec;
		//	}

	} while (ps.tnow_min < simDur_min);
	//SimulationComplete();
	return 1;
}