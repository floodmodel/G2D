using System.Text;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Alea;
using Alea.CSharp; // gpu.Launch는 이게 참조되어야 한다.
using Alea.FSharp;


namespace G2DCore
{
    public class cSimulatorCPUnGPU
    {
        public event SimulationStepEventHandler SimulationStep;
        public delegate void SimulationStepEventHandler(double elapsedMinutes);
        public event SimulationStopEventHandler SimulationStop;
        public delegate void SimulationStopEventHandler();
        public event SimulationCompleteEventHandler SimulationComplete;
        public delegate void SimulationCompleteEventHandler();
        public event SimulationRaiseErrorEventHandler SimulationRaiseError;
        public delegate void SimulationRaiseErrorEventHandler
                                 (cSimulatorCPUnGPU sender, string simulError, object erroData);

        private int rainfallisEnded = 1;
        private cDomain mDM;
        private int nRows;
        private int nCols;
        private double dx;
        private cSolver mSolver;
        private cSimulationSetting msimulSetting;
        public stCVAtt[] cvs;
        public stCVAtt_add[] cvsadd;
        public stBCinfo[] bcCellinfo;

        [GpuManaged]
        public bool simulationControlUsingCPUnGPU(cProject prj)
        {
            mDM = prj.domain;
            nRows = mDM.nRows;
            nCols = mDM.nCols;
            dx = mDM.dx;
            cvs = mDM.mCVsAry;
            cvsadd = mDM.mCVsAddAary;
            mSolver = new G2DCore.cSolver(prj.domain);
            msimulSetting = new cSimulationSetting(prj);
            double simduration_hr = prj.simDur_hr;
            double tnow_hr = 0;
            int bcDataOrder = 0;
            int rainfallDataOrder = 0;
            cGenEnv.tsec_targetToprint = 0;
            cGenEnv.tnow_sec = 0;
            cThisProcess.effCellCount = 0;
            double tnow_min_bak = 0;
            int dtbc_sec = prj.BConditionInterval_min * 60;
            cGenEnv.thisPrintStepStartTime = DateTime.Now;
            int bcdt_min = prj.BConditionInterval_min;
            if (prj.isRainfallApplied == 1)
            { rainfallisEnded = -1; }
            else
            { rainfallisEnded = 1; }
            string logFPN = Path.Combine(prj.prjFilePath, "G2DTimeLog" + "_" + prj.prjFileNameWithoutExt + ".txt");
            if (File.Exists(logFPN)) { File.Delete(logFPN); }
            msimulSetting.setStartingConditionUsingCPU(cvs, cvsadd);
            stGlobalValues[] gv = new stGlobalValues[1];
            bcCellinfo = new stBCinfo[prj.bcCellCountAll];
            gv[0] = cSimulationSetting.initGlobalValues(prj, mDM);
            cSimulationSetting.initBCInfo(prj, bcCellinfo);
            gv[0].dt_sec = cGenEnv.dt_sec;
            double exitInterval_sec = getTintervalToExitInnerLoop(prj);
            int onCPU = 1;
            Stopwatch sw = new Stopwatch();
            do //모의 시작할 때 t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
            {
                cGenEnv.tnow_min = (double)cGenEnv.tnow_sec / 60;
                tnow_hr = (double)cGenEnv.tnow_sec / 3600;
                //이건 경계조건 등
                if (prj.isBCApplied == 1)
                {
                    if ((cGenEnv.tnow_min % bcdt_min == 0) ||
                     ((tnow_min_bak < (bcDataOrder * bcdt_min)) &
                       (cGenEnv.tnow_min >= bcDataOrder * bcdt_min)))
                    {
                        bcDataOrder++;
                        cConditionData.getConditionDataUsingAry(mDM, cvs, cvsadd, prj.bcData,
                            prj.mBCCount, bcDataOrder, bcdt_min);
                    }
                }
                //이건 강우
                cThisProcess.rfisGreaterThanZero = -1;
                if (prj.isRainfallApplied == 1 && rainfallisEnded == -1)
                {
                    if ((cGenEnv.tnow_min % prj.rainfallInterval_min == 0) ||
                    (tnow_min_bak < (rainfallDataOrder * prj.rainfallInterval_min)) &
                    (cGenEnv.tnow_min >= rainfallDataOrder * prj.rainfallInterval_min))
                    {
                        rainfallDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
                        rainfallisEnded = G2DCore.cRainfall.ReadRainfallAndGetIntensityUsingArray(prj, rainfallDataOrder, cvs, cvsadd);
                    }
                }
                //이건  dem file 교체
                if (mDM .demfileLstToChange .Count >0 && cGenEnv.tnow_min > 0)
                {
                    for (int nr = 0; nr < prj.prjds.DEMFileToChange.Rows.Count; nr++)
                    {
                        Dataset.projectds.DEMFileToChangeRow row =
                            (Dataset.projectds.DEMFileToChangeRow)prj.prjds.DEMFileToChange.Rows[nr];
                        if (cSimulationSetting.changeDEMFileUsingArray(mDM, row, tnow_min_bak, cvs, cGenEnv.tnow_min) == false)
                        {
                            cGenEnv.writelogNconsole(string.Format("An error was occurred while changing dem file."), cGenEnv.bwritelog_error);
                            return false;
                        }
                    }
                }

                // 여기부터
                if (cThisProcess .rfisGreaterThanZero ==1)
                { cGenEnv.dMinLimitforWet = cGenEnv.dMinLimitforWet_ori;                    }
                else
                { cGenEnv.dMinLimitforWet = cGenEnv.dMinLimitforWet_ori * 5; }
                gv[0].dMinLimitforWet = cGenEnv.dMinLimitforWet;
                msimulSetting.initilizeThisStepUsingArray(cvs, cvsadd, bcCellinfo, cGenEnv.dt_sec, cGenEnv.tnow_sec, dtbc_sec, rainfallisEnded);//, mAllCellFalse);

                // gpu 옵션이 true 인 경우에도 지정셀(모로코에서는 40,000 개 가 적당) 셀 이상을 모의할 때만 gpu를 사용한다.
                // 모의 대상 셀 개수가 작을 때는 cpu 가 더 빠르다.
                if (cGenEnv.usingGPU == 1 && cThisProcess.effCellCount > cGenEnv.EffCellThresholdForGPU)
                {
                    //sw.Start();
                    if (onCPU == 1)
                    {
                        cGenEnv.writelog("Calculation was converted into GPU. ", true);
                        onCPU = -1;
                    }
                    mSolver.RunSolverUsingGPU(cvs, cvsadd, ref gv, bcCellinfo);
                    //sw.Stop();
                    //File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingGPU, elaplsed time [ms] : " +
                    //                       sw.Elapsed.TotalMilliseconds.ToString()  + "\r\n");
                    //sw.Reset();
                }
                else
                {
                    //sw.Start();
                    if (onCPU == -1)
                    {
                        cGenEnv.writelog("Calculation was converted into CPU. ", true);
                        onCPU = 1;
                    }
                    mSolver.RunSolverUsing1DArray(cvs, cvsadd, gv[0].dt_sec, gv, bcCellinfo);
                    //sw.Stop();
                    //File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingCPU, elaplsed time [ms] : " +
                    //    sw.Elapsed.TotalMilliseconds.ToString() + " simCell :  " + cGenEnv.effCellCount.ToString() + "\r\n");
                    //sw.Reset();
                }
                // 여기까지..

                cGenEnv.iGS = gv[0].iGS;
                cGenEnv.iNR = gv[0].iNR;
                if (cGenEnv.tnow_sec >= cGenEnv.tsec_targetToprint)
                {
                    checkEffetiveCellNumberAndSetAllFlase(cvs);// 매번 업데이트 하지 않고, 출력할때 마다 이 정보 업데이트 한다.
                    prj.output.makeOutputFilesUsing1DArray(cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_printout_min, prj.domain.nodata_value);
                    SimulationStep(cGenEnv.tnow_min);
                    if (onCPU == 1) { Gpu.FreeAllImplicitMemory(true); }
                    if (UpdateSimulaltionParameters(prj, Path.Combine(prj.prjFilePath, prj.prjFileName)) == false) { return false; } //한번 출력할때 마다 모의변수 업데이트
                    cGenEnv.tsec_targetToprint = cGenEnv.tsec_targetToprint + cGenEnv.dt_printout_sec;
                    cGenEnv.thisPrintStepStartTime = DateTime.Now;
                }
                tnow_min_bak = cGenEnv.tnow_min;
                cGenEnv.tnow_sec = cGenEnv.tnow_sec + gv[0].dt_sec;
                if (cGenEnv.isfixeddt == -1)
                {
                    cGenEnv.dt_sec = cHydro.getDTsecUsingConstraints(bcCellinfo, cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_sec, cHydro.cflNumber, dx,
                                             cGenEnv.gravity, cGenEnv.dflowmaxInThisStep, cGenEnv.vmaxInThisStep, cGenEnv.VNConMinInThisStep,
                                             cGenEnv.dt_printout_min * 30, bcdt_min * 30, prj.rainfallInterval_min * 30, cHydro.applyVNC);
                    gv[0].dt_sec = cGenEnv.dt_sec;
                }

            } while ((tnow_hr) < simduration_hr);
            SimulationComplete();
            return true;
        }

        private void checkEffetiveCellNumberAndSetAllFlase(stCVAtt[] cvs)
        {
            cThisProcess.effCellCount = 0;
            cThisProcess.FloodingCellCounts = new List<int>();
            //cThisProcess.FloodingCellMaxDepth = new List<double>();
            cThisProcess.FloodingCellMeanDepth = new List<double>();
            List<double> FloodingCellSumDepth = new List<double>();
            cThisProcess.FloodingCellMaxDepth = 0;
            for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
            {
                cThisProcess.FloodingCellCounts.Add(0);//0개씩으로 초기화 한다.
                //cThisProcess.FloodingCellMaxDepth.Add(0);
                cThisProcess.FloodingCellMeanDepth.Add(0);
                FloodingCellSumDepth.Add(0);
            }
            for (int i = 0; i < cvs.Length; i++)
            {
                if (cvs[i].isSimulatingCell == 1)
                {
                    cThisProcess.effCellCount += 1;
                    if (cvs[i].dp_tp1 > cThisProcess.FloodingCellMaxDepth)
                    {
                        cThisProcess.FloodingCellMaxDepth = cvs[i].dp_tp1;
                    }
                }
                cvs[i].isSimulatingCell = -1;
                for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
                {
                    if (cvs[i].dp_tp1 >= cGenEnv.floodingCellDepthThresholds_m[n])
                    {
                        cThisProcess.FloodingCellCounts[n] += 1;
                        FloodingCellSumDepth[n] += cvs[i].dp_tp1;
                        //if (cvs[i].dp_tp1 > cThisProcess.FloodingCellMaxDepth)
                        //{
                        //    cThisProcess.FloodingCellMaxDepth = cvs[i].dp_tp1;
                        //}
                    }
                }
            }
            for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
            {
                if (cThisProcess.FloodingCellCounts[n] > 0)
                {
                    cThisProcess.FloodingCellMeanDepth[n] = FloodingCellSumDepth[n] / cThisProcess.FloodingCellCounts[n];
                }
            }
        }

        private bool UpdateSimulaltionParameters(cProject prj, string prjFPN)
        {
            Dataset.projectds prjds = new Dataset.projectds();
            if (File.Exists(prjFPN) == false)
            {
                cGenEnv.writelogNconsole(string.Format("{0} is not exist. ", prjFPN), true);
                return false;
            }
            prjds.ReadXml(prjFPN);
            bool parChanged = false;
            // 여기부터 cGenEnv에 있는 변수들 확인
            Dataset.projectds.ProjectSettingsRow row =
                   (G2DCore.Dataset.projectds.ProjectSettingsRow)prjds.ProjectSettings.Rows[0];
            int bak_usingGPU = cGenEnv.usingGPU;
            int bak_isparallel = cGenEnv.isparallel;
            int bak_MDP = cGenEnv.maxDegreeParallelism;
            int bak_EffCellThresholdForGPU = cGenEnv.EffCellThresholdForGPU;
            int bak_iGSmax = cGenEnv.iGSmax;
            int bak_iNRmax = cGenEnv.iNRmax_forCE;
            double bak_dt_printout_min = cGenEnv.dt_printout_min ;
            int bak_dt_printout_sec = cGenEnv .dt_printout_sec ;
            List<double> bak_FloodingCellThresholds = cGenEnv.floodingCellDepthThresholds_m;
            cGenEnv.setValues(row);
            if (bak_isparallel != cGenEnv.isparallel || bak_MDP != cGenEnv.maxDegreeParallelism || bak_usingGPU != cGenEnv.usingGPU)
            {
                if (cGenEnv.isparallel == 1)
                {
                    string usingGPU = "false";
                    if (cGenEnv.usingGPU == 1) { usingGPU = "true"; }
                    Console.WriteLine();
                    cGenEnv.writelogNconsole(string.Format("Parallel : true. Max. degree of parallelism : {0}. Using GPU : {1}",
                        cGenEnv.maxDegreeParallelism.ToString(), usingGPU), true);
                }
                else
                {
                    Console.WriteLine();
                    cGenEnv.writelogNconsole(string.Format("Parallel : false. Using GPU : false"), true);
                }
                parChanged = true;
            }

            if (bak_isparallel == -1 && cGenEnv.isparallel == 1)
            {
                cGenEnv.getCPUInfo();
            }
            if (bak_usingGPU == -1 && cGenEnv.usingGPU == 1 || bak_EffCellThresholdForGPU != cGenEnv.EffCellThresholdForGPU)
            {
                if (parChanged == false && bak_EffCellThresholdForGPU != cGenEnv.EffCellThresholdForGPU) { Console.WriteLine(); }
                cGenEnv.getGPUDeviceInfo();
                cGenEnv.writelogNconsole(string.Format("Effective cells number threshold to convert to GPU calculation : {0}", cGenEnv.EffCellThresholdForGPU.ToString()), true);
                parChanged = true;
            }
            if (bak_iGSmax != cGenEnv.iGSmax || bak_iNRmax != cGenEnv.iNRmax_forCE)
            {
                if (parChanged == false) { Console.WriteLine(); }
                cGenEnv.writelogNconsole(string.Format("iGS(all cells) max : {0}, iNR(a cell) max : {1}, tolerance : {2}",
                        cGenEnv.iGSmax.ToString(), cGenEnv.iNRmax_forCE.ToString(), cGenEnv.convergenceConditionh.ToString()), true);
                parChanged = true;
            }
            if (bak_dt_printout_min != cGenEnv.dt_printout_min)
            {
                if (parChanged == false) { Console.WriteLine(); }
                cGenEnv.writelogNconsole(string.Format("Print out time step : {0} min.", cGenEnv.dt_printout_min), true);
                parChanged = true;
            }
            string thresholds = "".ToString();
            for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
            {
                if (n == 0)
                {
                    thresholds = (cGenEnv.floodingCellDepthThresholds_m[n] * 100).ToString();
                }
                else
                {
                    thresholds = thresholds + ", " + (cGenEnv.floodingCellDepthThresholds_m[n] * 100).ToString();
                }
            }
            if (bak_FloodingCellThresholds.Count != cGenEnv.floodingCellDepthThresholds_m.Count)
            {
                if (parChanged == false) { Console.WriteLine(); }
                cGenEnv.writelogNconsole(string.Format("Flooding cell threshold (cm) : {0}.", thresholds), true);
                parChanged = true;
            }
            else
            {
                bool changed = false;
                for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
                {
                    if (bak_FloodingCellThresholds[n] != cGenEnv.floodingCellDepthThresholds_m[n])
                    {
                        changed = true;
                    }
                }
                if (changed==true )
                {
                    if (parChanged == false) { Console.WriteLine(); }
                    cGenEnv.writelogNconsole(string.Format("Flooding cell threshold (cm) : {0}.", thresholds), true);
                    parChanged = true;
                }
            }

            // 여기부터 DEM file to change 관련 변수들 확인
            int bak_dfc_count = prj.domain.demfileLstToChange.Count;
            List<cVars.DEMFileInfo> new_DEMFileListToChange = new List<cVars.DEMFileInfo>() ;
            if (prjds.DEMFileToChange.Rows.Count > 0)
            {
                for (int nr = 0; nr < prjds.DEMFileToChange.Rows.Count; nr++)
                {
                    Dataset.projectds.DEMFileToChangeRow rowDEMtoChange =
                        (Dataset.projectds.DEMFileToChangeRow)prjds.DEMFileToChange.Rows[nr];
                    double minuteout = 0;
                    if (double.TryParse(rowDEMtoChange.TimeMinute, out minuteout) == true)
                    {
                        if (System.IO.File.Exists(rowDEMtoChange.DEMFile) == true && minuteout > 0)
                        {
                            cVars.DEMFileInfo nf;
                            nf.timeMinutes = minuteout;
                            nf.demFPN = rowDEMtoChange.DEMFile;
                            new_DEMFileListToChange.Add(nf);
                        }
                    }
                }
            }
            if (new_DEMFileListToChange.Count != prj.domain.demfileLstToChange.Count)
            {
                if (parChanged == false) { Console.WriteLine(); }
                if (new_DEMFileListToChange.Count == 0)
                {
                    cGenEnv.writelogNconsole(string.Format("All DEM files to change were removed."), true);
                }
                else
                {
                    for (int n = 0; n < new_DEMFileListToChange.Count; n++)
                    {
                        cGenEnv.writelogNconsole(string.Format("DEM file to change. Time : {0}, File : {1}.",
                            new_DEMFileListToChange[n].timeMinutes.ToString(), new_DEMFileListToChange[n].demFPN), true);
                    }
                }
                prj.domain.demfileLstToChange.Clear();
                prj.domain.demfileLstToChange = new_DEMFileListToChange;
                parChanged = true;
            }
            else if (new_DEMFileListToChange.Count > 0)
            {
                bool changed = false;
                for (int n = 0; n < new_DEMFileListToChange.Count; n++)
                {
                    if (new_DEMFileListToChange[n].timeMinutes != prj.domain.demfileLstToChange[n].timeMinutes ||
                        new_DEMFileListToChange[n].demFPN != prj.domain.demfileLstToChange[n].demFPN)
                    {
                        if (parChanged == false) { Console.WriteLine(); }
                        cGenEnv.writelogNconsole(string.Format("DEM file to change. Time : {0}, File : {1}.",
                            new_DEMFileListToChange[n].timeMinutes.ToString(), new_DEMFileListToChange[n].demFPN), true);
                        parChanged = true;
                        changed = true;
                    }
                }
                if (changed == true)
                {
                    prj.domain.demfileLstToChange.Clear();
                    prj.domain.demfileLstToChange = new_DEMFileListToChange;
                }
            }
            if (parChanged == true) { cGenEnv.writelogNconsole("Simulation parameters were changed.", true); }
            return true;
        }

        [GpuManaged]
        public bool simulationControlUsingGPUwithInnerLoop_trial(cProject prj)
        {
            mDM = prj.domain;
            nRows = mDM.nRows;
            nCols = mDM.nCols;
            dx = mDM.dx;
            cvs = mDM.mCVsAry;
            cvsadd = mDM.mCVsAddAary;
            mSolver = new G2DCore.cSolver(prj.domain);
            msimulSetting = new cSimulationSetting(prj);
            double simduration_hr = prj.simDur_hr;
            int bcDataOrder = 0;
            int rainfallDataOrder = 0;
            double tnow_hr = 0;
            cGenEnv.tsec_targetToprint = 0;
            cGenEnv.tnow_sec = 0;
            double tnow_min_bak = 0;
            double dt_sec = cGenEnv.dtStart_sec;
            cGenEnv.dt_sec = cGenEnv.dtStart_sec;
            cGenEnv.thisPrintStepStartTime = DateTime.Now;
            int bcdt_min = prj.BConditionInterval_min;
            if (prj.isRainfallApplied == 1)
            { rainfallisEnded = -1; }
            else
            { rainfallisEnded = 1; }
            string logFPN = Path.Combine(prj.prjFilePath, "G2DTimeLog" + "_" + prj.prjFileNameWithoutExt + ".txt");
            if (File.Exists(logFPN)) { File.Delete(logFPN); }
            Stopwatch sw = new Stopwatch();
            msimulSetting.setStartingConditionUsingCPU(cvs, cvsadd);
            stGlobalValues[] gv = new stGlobalValues[1];
            gv[0] = cSimulationSetting. initGlobalValues(prj, mDM);
            double exitInterval_sec = getTintervalToExitInnerLoop(prj);
            if (exitInterval_sec > 600) { exitInterval_sec = 600; }
            var gpu = Gpu.Default;
            var lp = new LaunchParam(16, 256);
            do //모의 시작할 때, t 는 초기 조건, t+dt는 소스 하나가 적용된 결과
            {

                dt_sec = cGenEnv.dt_sec;// 전진된 tnow_sec에서의 dt_sec을 설정한다.
                cGenEnv.tnow_min = (double)cGenEnv.tnow_sec / 60;
                tnow_hr = (double)cGenEnv.tnow_sec / 3600;
                //이건 경계조건 등
                if (prj.isBCApplied == 1)
                {
                    if ((cGenEnv.tnow_min % bcdt_min == 0) ||
                     ((tnow_min_bak < (bcDataOrder * bcdt_min)) &
                       (cGenEnv.tnow_min >= bcDataOrder * bcdt_min)))
                    {
                        bcDataOrder++;
                        cConditionData.getConditionDataUsingAry(mDM, cvs, cvsadd, prj.bcData,
                            prj.mBCCount, bcDataOrder, bcdt_min);
                    }
                }
                //이건 강우
                cThisProcess.rfisGreaterThanZero = -1;
                if (prj.isRainfallApplied == 1 && rainfallisEnded == -1)
                {
                    if ((cGenEnv.tnow_min % prj.rainfallInterval_min == 0) ||
                    (tnow_min_bak < (rainfallDataOrder * prj.rainfallInterval_min)) &
                    (cGenEnv.tnow_min >= rainfallDataOrder * prj.rainfallInterval_min))
                    {
                        rainfallDataOrder++; //1부터 시작. 배열은 rainfallDataOrder-1
                        rainfallisEnded = G2DCore.cRainfall.ReadRainfallAndGetIntensityUsingArray(prj, rainfallDataOrder, cvs, cvsadd);
                    }
                }
                //이건  dem file 교체
                if (mDM.demfileLstToChange.Count > 0 && cGenEnv.tnow_min > 0)
                {
                    for (int nr = 0; nr < prj.prjds.DEMFileToChange.Rows.Count; nr++)
                    {
                        Dataset.projectds.DEMFileToChangeRow row =
                            (Dataset.projectds.DEMFileToChangeRow)prj.prjds.DEMFileToChange.Rows[nr];
                        if (cSimulationSetting.changeDEMFileUsingArray(mDM, row, tnow_min_bak, cvs, cGenEnv.tnow_min) == false)
                        {
                            cGenEnv.writelogNconsole(string.Format("An error was occurred while changing dem file."), cGenEnv.bwritelog_error);
                            return false;
                        }
                    }
                }

                gv[0].dt_sec = cGenEnv.dt_sec;
                //gv[0].tnow_sec = cGenEnv.tnow_sec;
                //gv[0].rfReadintensityForMAP_mPsec = cThisProcess.rfReadintensityForMAP_mPsec;
                //if (rainfallisEnded == true) { gv[0].rainfallisEnded = 1; } else { gv[0].rainfallisEnded = -1; }
                if (cGenEnv.tnow_sec == 0)
                {
                    //gv[0].tsecToExitInnerLoop = gv[0].tnow_sec + cGenEnv.dt_sec;
                }
                else
                {
                    //gv[0].tsecToExitInnerLoop = gv[0].tnow_sec + exitInterval_sec;
                    //double res = gv[0].tsecToExitInnerLoop % exitInterval_sec;
                    //gv[0].tsecToExitInnerLoop = gv[0].tsecToExitInnerLoop - res;
                }
                //여기부터2
                tnow_min_bak = cGenEnv.tnow_min; // dt 만큼 전진하기 전의 값.
                sw.Start();
                //cSolver.innerLoop2(cvs, gv);
                gpu.Launch(cSolver.innerLoopForGPUonly_trial, lp, cvs, gv);
                gpu.Synchronize();
                File.AppendAllText(logFPN, cGenEnv.tnow_min.ToString("F2") + "min. RunSolverUsingGPU_innerLoop, elaplsed time [ms] : "
                        + sw.Elapsed.TotalMilliseconds.ToString() + "\r\n");
                sw.Stop();
                sw.Reset();
                cGenEnv.dt_sec = gv[0].dt_sec;
                cGenEnv.tnow_min = (double)cGenEnv.tnow_sec / 60;
                tnow_hr = (double)cGenEnv.tnow_sec / 3600;
                //여기까지2

                if (cGenEnv.tnow_sec >= cGenEnv.tsec_targetToprint)// 이건 이미 dt 만큼 전진된 시간이다.
                {
                    checkEffetiveCellNumberAndSetAllFlase(cvs);
                    prj.output.makeOutputFilesUsing1DArray(cvs, cvsadd, cGenEnv.tnow_sec, cGenEnv.dt_printout_min, prj.domain.nodata_value);
                    SimulationStep(cGenEnv.tnow_min);
                    cGenEnv.tsec_targetToprint = cGenEnv.tsec_targetToprint + cGenEnv.dt_printout_sec;
                    cGenEnv.thisPrintStepStartTime = DateTime.Now;
                    Gpu.FreeAllImplicitMemory(true);
                }
            } while ((tnow_hr) < simduration_hr);
            SimulationComplete();
            return true;
        }

        private double getTintervalToExitInnerLoop(cProject prj)
        {
            double tsec = double.MaxValue;
            if (prj.BConditionInterval_min > 0) { tsec = Math.Min(prj.BConditionInterval_min * 60 / 2, tsec); }
            if (prj.rainfallInterval_min > 0) { tsec = Math.Min(prj.rainfallInterval_min * 60 / 2, tsec); }
            if (cGenEnv.dt_printout_sec > 0) { tsec = Math.Min(cGenEnv.dt_printout_sec / 2, tsec); }
            return tsec;
        }
    }
}

