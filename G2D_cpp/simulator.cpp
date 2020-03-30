
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
//globalVinner * gvip;
globalVinner gvi[1];


int simulationControlUsingCPUnGPU()
{
	int	nRows = di.nRows;
	int nCols = di.nCols;
	double dx = di.dx;
	//	mSolver = new G2DCore.cSolver(prj.domain);
	//	msimulSetting = new cSimulationSetting(prj);
	double simDur_min = prj.simDuration_min + 1.0;
	int bcDataOrder = 0;
	int rfDataOrder = 0;
	int rfEnded = 1;
	int demToChangeEnded = 1;
	ps.tsec_targetToprint = 0;
	ps.dt_printout_sec = (int)prj.printOutInterval_min * 60;
	ps.tnow_sec = 0;
	ps.effCellCount = 0;
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
	psi.dt_sec = prj.calculationTimeStep_sec;
	//*gvip = initGlobalVinner();
	if (setStartingConditionUsingCPU() == -1) { return -1; }
	do //���� ������ �� t �� �ʱ� ����, t+dt�� �ҽ� �ϳ��� ����� ���
	{
		ps.tnow_min = ps.tnow_sec / 60.0;
		//�̰� ������� ��
		if (prj.isbcApplied == 1) {
			//if (fmod(ps.tnow_min, dtbc_min) == 0.0 ||
			//	((tnow_min_bak < (bcDataOrder * dtbc_min)) &
			//	(ps.tnow_min >= bcDataOrder * dtbc_min)))
			int bc_min = bcDataOrder * dtbc_min;
			if (((tnow_min_bak < bc_min) & (ps.tnow_min >= bc_min))|| bc_min==0)
			{
				bcDataOrder++;
				getCellConditionData(bcDataOrder, dtbc_min);
			}
		}
		//�̰� ����
		if (prj.isRainfallApplied == 1 && rfEnded == -1) {
			//if (fmod(ps.tnow_min, prj.rainfallDataInterval_min == 0) ||
			//	((tnow_min_bak < (rfDataOrder * prj.rainfallDataInterval_min)) &
			//	(ps.tnow_min >= rfDataOrder * prj.rainfallDataInterval_min)))
			int rf_min = rfDataOrder * prj.rainfallDataInterval_min;
			if (((tnow_min_bak < rf_min) & (ps.tnow_min >= rf_min))|| rf_min==0)
			{
				psi.rfisGreaterThanZero = -1;
				rfDataOrder++; //1���� ����. �迭�� rainfallDataOrder-1
				rfEnded = readRainfallAndGetIntensity(rfDataOrder);
				// 0���� ū ���찡 �ϳ��� ������...
				if (psi.rfisGreaterThanZero == 1) {
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori;
				}
				else if (psi.rfisGreaterThanZero == -1 || rfEnded == 1) {
					//���찡 �������� �ּҼ����� �� ũ�� ��Ƶ� �ȴ�.
					gvi[0].dMinLimitforWet = ge.dMinLimitforWet_ori * 5.0;
				}
			}
		}
		//�̰�  dem file ��ü
		if (prj.isDEMtoChangeApplied == 1 && demToChangeEnded == -1) {// && ps.tnow_min > 0) {
			demToChangeEnded = changeDomainElevWithDEMFile(ps.tnow_min, tnow_min_bak);
		}
		initilizeThisStep(psi.dt_sec, ps.tnow_sec, dtbc_sec, rfEnded);

		// gpu �ɼ��� true �� ��쿡�� ������ �̻��� ������ ���� gpu�� ����Ѵ�.
		// ���� ��� �� ������ ���� ���� cpu �� �� ������.
		if (prj.usingGPU == 1 && ps.effCellCount > prj.effCellThresholdForGPU)
		{
			if (onCPU == 1) {
				writeLog(fpn_log, "Calculation was converted into GPU.\n", 1, 1);
				gvi[0].iNRmaxLimit = prj.maxIterationACellOnGPU;
				gvi[0].iNRmaxLimit = prj.maxIterationAllCellsOnGPU;
				onCPU = -1;
			}
			runSolverUsingGPU();
			//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingGPU, elaplsed time [ms] : " +
			//                       sw.Elapsed.TotalMilliseconds.ToString()  + "\r\n");
		}
		else {
			if (onCPU == -1) {
				writeLog(fpn_log, "Calculation was converted into CPU.\n", 1, 1);
				gvi[0].iNRmaxLimit = prj.maxIterationACellOnCPU;
				gvi[0].iGSmaxLimit = prj.maxIterationAllCellsOnCPU;
				onCPU = 1;
			}
			runSolverUsingCPU();
			//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingCPU, elaplsed time [ms] : " +
			//    sw.Elapsed.TotalMilliseconds.ToString() + " simCell :  " + cGenEnv.effCellCount.ToString() + "\r\n");
		}
		// �������..
		updateValuesInThisStepResults();
		if (ps.tnow_sec >= ps.tsec_targetToprint)
		{
			checkEffetiveCellNumberAndSetAllFlase();// �Ź� ������Ʈ ���� �ʰ�, ����Ҷ� ���� �� ���� ������Ʈ �Ѵ�.
			makeOutputFiles(ps.tnow_sec);
			int progressRatio = (int)(ps.tnow_min / prj.simDuration_min * 100);
			printf("\rCurrent progress[min]: %d/%d[%d%%]..", (int)ps.tnow_min, (int)prj.simDuration_min, progressRatio);
			//�ѹ� ����Ҷ� ���� ���Ǻ��� ������Ʈ
			if (updateProjectParameters() == -1) {
				return -1;
			}
			ps.tsec_targetToprint = ps.tsec_targetToprint + ps.dt_printout_sec;
			ps.thisPrintStepStartTime = COleDateTime::GetCurrentTime();
		}
		tnow_min_bak = ps.tnow_min;
		ps.tnow_sec = ps.tnow_sec + psi.dt_sec;
		if (prj.isFixedDT == -1)
		{
			psi.dt_sec = getDTsecWithConstraints(psi.dflowmaxInThisStep, psi.vmaxInThisStep,
				psi.VNConMinInThisStep);
		}

	} while (ps.tnow_min < simDur_min);
	return 1;
}