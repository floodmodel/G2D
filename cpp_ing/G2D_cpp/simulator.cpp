
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
	// gpu���� ����ü ���� ������Ʈ �ȵ� (c#). �׷��� ����ü �迭�� ���. 
	// C++���� ������ Ȥ�� ����ü �� ��ȯ�� gpu �ڵ����� ���� ������Ʈ �Ǵ��� Ȯ�� �ʿ�
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

	do //���� ������ �� t �� �ʱ� ����, t+dt�� �ҽ� �ϳ��� ����� ���
	{
		ps.tnow_min = ps.tnow_sec / 60.0;
		//�̰� ������� ��
		if (prj.isbcApplied == 1) {
			if (fmod(ps.tnow_min, dtbc_min) == 0.0 ||
				((tnow_min_bak < (bcDataOrder * dtbc_min)) &
				(ps.tnow_min >= bcDataOrder * dtbc_min)))
			{
				bcDataOrder++;
				getCellConditionData(bcDataOrder, dtbc_min);
			}
		}
		//�̰� ����
		if (prj.isRainfallApplied == 1 && rfEnded == -1) {
			if (fmod(ps.tnow_min, prj.rainfallDataInterval_min == 0) ||
				((tnow_min_bak < (rfDataOrder * prj.rainfallDataInterval_min)) &
				(ps.tnow_min >= rfDataOrder * prj.rainfallDataInterval_min)))
			{
				psi.rfisGreaterThanZero = -1;
				rfDataOrder++; //1���� ����. �迭�� rainfallDataOrder-1
				rfEnded = readRainfallAndGetIntensity(rfDataOrder);
				// 0���� ū ���찡 �ϳ��� ������...
				if (psi.rfisGreaterThanZero == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori;
				}
				//���찡 �������� �ּҼ����� �� ũ�� ��Ƶ� �ȴ�.
				if (psi.rfisGreaterThanZero == -1 || rfEnded == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0f;
				}
			}
		}
		//�̰�  dem file ��ü
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == -1 && ps.tnow_min > 0) {
			demToChangeEnded = changeDomainElevWithDEMFile(ps.tnow_min, tnow_min_bak);
		}
		initilizeThisStep(psi.dt_sec, ps.tnow_sec, dtbc_sec, rfEnded);

		//	// gpu �ɼ��� true �� ��쿡�� ������(����ڿ����� 40,000 �� �� ����) �� �̻��� ������ ���� gpu�� ����Ѵ�.
		//	// ���� ��� �� ������ ���� ���� cpu �� �� ������.
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
		//	// �������..

		//	cGenEnv.iGS = gv[0].iGS;
		//	cGenEnv.iNR = gv[0].iNR;
		//	if (cGenEnv.tnow_sec >= cGenEnv.tsec_targetToprint)
		//	{
		//		checkEffetiveCellNumberAndSetAllFlase(cvs);// �Ź� ������Ʈ ���� �ʰ�, ����Ҷ� ���� �� ���� ������Ʈ �Ѵ�.
		//		prj.output.makeOutputFilesUsing1DArray(cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_printout_min, prj.domain.nodata_value);
		//		SimulationStep(cGenEnv.tnow_min);
		//		if (onCPU == 1) { Gpu.FreeAllImplicitMemory(true); }
		//		if (UpdateSimulaltionParameters(prj, Path.Combine(prj.prjFilePath, prj.prjFileName)) == false) { return false; } //�ѹ� ����Ҷ� ���� ���Ǻ��� ������Ʈ
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