
#include <stdio.h>

#include "gentle.h"
#include "g2d.h"

using namespace std;

fs::path fpn_prj;
fs::path fpn_log;
fs::path fp_prj;

extern projectFile prj;
extern generalEnv genEnv;
extern domaininfo di;
extern domainCell** dmcells;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern vector<rainfallinfo> rf;
extern bcinfo* bcs;

thisProcess ps;
thisProcessInner psi;

int simulationControlUsingCPUnGPU()
{
int	nRows = di.nRows;
int nCols = di.nCols;
float dx = di.dx;

//	mSolver = new G2DCore.cSolver(prj.domain);
//	msimulSetting = new cSimulationSetting(prj);

	double tnow_hr = 0;
	int bcDataOrder = 0;
	int rainfallDataOrder = 0;
	ps.tsec_targetToprint = 0;
	ps.tnow_sec = 0;
//	cThisProcess.effCellCount = 0;
//	double tnow_min_bak = 0;
//	int dtbc_sec = prj.BConditionInterval_min * 60;
//	cGenEnv.thisPrintStepStartTime = DateTime.Now;
//	int bcdt_min = prj.BConditionInterval_min;
//	if (prj.isRainfallApplied == 1)
//	{
//		rainfallisEnded = -1;
//	}
//	else
//	{
//		rainfallisEnded = 1;
//	}
//	string logFPN = Path.Combine(prj.prjFilePath, "G2DTimeLog" + "_" + prj.prjFileNameWithoutExt + ".txt");
//	if (File.Exists(logFPN)) { File.Delete(logFPN); }
//	msimulSetting.setStartingConditionUsingCPU(cvs, cvsadd);
//	stGlobalValues[] gv = new stGlobalValues[1];
//	bcCellinfo = new stBCinfo[prj.bcCellCountAll];
//	gv[0] = cSimulationSetting.initGlobalValues(prj, mDM);
//	cSimulationSetting.initBCInfo(prj, bcCellinfo);
//	gv[0].dt_sec = cGenEnv.dt_sec;
//	double exitInterval_sec = getTintervalToExitInnerLoop(prj);
//	int onCPU = 1;
//	Stopwatch sw = new Stopwatch();
//	do //���� ������ �� t �� �ʱ� ����, t+dt�� �ҽ� �ϳ��� ����� ���
//	{
//		cGenEnv.tnow_min = (double)cGenEnv.tnow_sec / 60;
//		tnow_hr = (double)cGenEnv.tnow_sec / 3600;
//		//�̰� ������� ��
//		if (prj.isBCApplied == 1)
//		{
//			if ((cGenEnv.tnow_min % bcdt_min == 0) ||
//				((tnow_min_bak < (bcDataOrder * bcdt_min)) &
//				(cGenEnv.tnow_min >= bcDataOrder * bcdt_min)))
//			{
//				bcDataOrder++;
//				cConditionData.getConditionDataUsingAry(mDM, cvs, cvsadd, prj.bcData,
//					prj.mBCCount, bcDataOrder, bcdt_min);
//			}
//		}
//		//�̰� ����
//		cThisProcess.rfisGreaterThanZero = -1;
//		if (prj.isRainfallApplied == 1 && rainfallisEnded == -1)
//		{
//			if ((cGenEnv.tnow_min % prj.rainfallInterval_min == 0) ||
//				(tnow_min_bak < (rainfallDataOrder * prj.rainfallInterval_min)) &
//				(cGenEnv.tnow_min >= rainfallDataOrder * prj.rainfallInterval_min))
//			{
//				rainfallDataOrder++; //1���� ����. �迭�� rainfallDataOrder-1
//				rainfallisEnded = G2DCore.cRainfall.ReadRainfallAndGetIntensityUsingArray(prj, rainfallDataOrder, cvs, cvsadd);
//			}
//		}
//		//�̰�  dem file ��ü
//		if (mDM.demfileLstToChange.Count > 0 && cGenEnv.tnow_min > 0)
//		{
//			for (int nr = 0; nr < prj.prjds.DEMFileToChange.Rows.Count; nr++)
//			{
//				Dataset.projectds.DEMFileToChangeRow row =
//					(Dataset.projectds.DEMFileToChangeRow)prj.prjds.DEMFileToChange.Rows[nr];
//				if (cSimulationSetting.changeDEMFileUsingArray(mDM, row, tnow_min_bak, cvs, cGenEnv.tnow_min) == false)
//				{
//					cGenEnv.writelogNconsole(string.Format("An error was occurred while changing dem file."), cGenEnv.bwritelog_error);
//					return false;
//				}
//			}
//		}
//
//		// �������
//		if (cThisProcess.rfisGreaterThanZero == 1)
//		{
//			cGenEnv.dMinLimitforWet = cGenEnv.dMinLimitforWet_ori;
//		}
//		else
//		{
//			cGenEnv.dMinLimitforWet = cGenEnv.dMinLimitforWet_ori * 5;
//		}
//		gv[0].dMinLimitforWet = cGenEnv.dMinLimitforWet;
//		msimulSetting.initilizeThisStepUsingArray(cvs, cvsadd, bcCellinfo, cGenEnv.dt_sec, cGenEnv.tnow_sec, dtbc_sec, rainfallisEnded);//, mAllCellFalse);
//
//		// gpu �ɼ��� true �� ��쿡�� ������(����ڿ����� 40,000 �� �� ����) �� �̻��� ������ ���� gpu�� ����Ѵ�.
//		// ���� ��� �� ������ ���� ���� cpu �� �� ������.
//		if (cGenEnv.usingGPU == 1 && cThisProcess.effCellCount > cGenEnv.EffCellThresholdForGPU)
//		{
//			//sw.Start();
//			if (onCPU == 1)
//			{
//				cGenEnv.writelog("Calculation was converted into GPU. ", true);
//				onCPU = -1;
//			}
//			mSolver.RunSolverUsingGPU(cvs, cvsadd, ref gv, bcCellinfo);
//			//sw.Stop();
//			//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingGPU, elaplsed time [ms] : " +
//			//                       sw.Elapsed.TotalMilliseconds.ToString()  + "\r\n");
//			//sw.Reset();
//		}
//		else
//		{
//			//sw.Start();
//			if (onCPU == -1)
//			{
//				cGenEnv.writelog("Calculation was converted into CPU. ", true);
//				onCPU = 1;
//			}
//			mSolver.RunSolverUsing1DArray(cvs, cvsadd, gv[0].dt_sec, gv, bcCellinfo);
//			//sw.Stop();
//			//File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingCPU, elaplsed time [ms] : " +
//			//    sw.Elapsed.TotalMilliseconds.ToString() + " simCell :  " + cGenEnv.effCellCount.ToString() + "\r\n");
//			//sw.Reset();
//		}
//		// �������..
//
//		cGenEnv.iGS = gv[0].iGS;
//		cGenEnv.iNR = gv[0].iNR;
//		if (cGenEnv.tnow_sec >= cGenEnv.tsec_targetToprint)
//		{
//			checkEffetiveCellNumberAndSetAllFlase(cvs);// �Ź� ������Ʈ ���� �ʰ�, ����Ҷ� ���� �� ���� ������Ʈ �Ѵ�.
//			prj.output.makeOutputFilesUsing1DArray(cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_printout_min, prj.domain.nodata_value);
//			SimulationStep(cGenEnv.tnow_min);
//			if (onCPU == 1) { Gpu.FreeAllImplicitMemory(true); }
//			if (UpdateSimulaltionParameters(prj, Path.Combine(prj.prjFilePath, prj.prjFileName)) == false) { return false; } //�ѹ� ����Ҷ� ���� ���Ǻ��� ������Ʈ
//			cGenEnv.tsec_targetToprint = cGenEnv.tsec_targetToprint + cGenEnv.dt_printout_sec;
//			cGenEnv.thisPrintStepStartTime = DateTime.Now;
//		}
//		tnow_min_bak = cGenEnv.tnow_min;
//		cGenEnv.tnow_sec = cGenEnv.tnow_sec + gv[0].dt_sec;
//		if (cGenEnv.isfixeddt == -1)
//		{
//			cGenEnv.dt_sec = cHydro.getDTsecUsingConstraints(bcCellinfo, cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_sec, cHydro.cflNumber, dx,
//				cGenEnv.gravity, cGenEnv.dflowmaxInThisStep, cGenEnv.vmaxInThisStep, cGenEnv.VNConMinInThisStep,
//				cGenEnv.dt_printout_min * 30, bcdt_min * 30, prj.rainfallInterval_min * 30, cHydro.applyVNC);
//			gv[0].dt_sec = cGenEnv.dt_sec;
//		}
//
//	} while ((tnow_hr) < simduration_hr);
//	SimulationComplete();
	return 1;
}