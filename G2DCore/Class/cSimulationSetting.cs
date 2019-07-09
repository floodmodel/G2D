using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Alea;
using Alea.Parallel;
using Alea.CSharp; // gpu.Launch는 이게 참조되어야 한다.

namespace G2DCore
{
    public class cSimulationSetting
    {
        double[] a;
        public int nRows;
        public int nCols;
        public cDomain dm;
        public double dx;
        public int isRainfallApplied;
        public int rainfallDataType; //1: MAP, 2: ASCII grid

        public cSimulationSetting(cProject prj)
        {
            dm = new cDomain();
            dm = prj.domain;
            nRows = dm.nRows;
            nCols = dm.nCols;
            dx = dm.dx;
            isRainfallApplied = prj.isRainfallApplied;
            rainfallDataType = prj.rainfall.rainfallDataType;
        }

        public void setStartingConditionUsingCPU(stCVAtt[] cvs, stCVAtt_add[] cvs_add)
        {
            if (cGenEnv.isparallel == 1)
            {
                var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
                Parallel.ForEach(Partitioner.Create(0, cvs.Length), options,
                            (range) =>
                            {
                                for (int i = range.Item1; i < range.Item2; i++)
                                {
                                    setStartingCondidtionInACell(cvs, i, cvs_add);
                                }
                            });
            }
            else
            {
                for (int i = 0; i < cvs.Length; i++)
                {

                    setStartingCondidtionInACell(cvs, i, cvs_add);
                }
            }
        }

        public static void setStartingCondidtionInACell(stCVAtt[] cvs, int idx, stCVAtt_add[] cvsadd)
        {
            cvs[idx].dp_t = cvsadd[idx].initialConditionDepth_meter;
            cvs[idx].dp_tp1 = cvs[idx].dp_t;
            cvs[idx].ve_tp1 = 0;
            cvs[idx].qe_tp1 = 0;
            cvs[idx].qw_tp1 = 0;
            cvs[idx].qn_tp1 = 0;
            cvs[idx].qs_tp1 = 0;
            cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
            cvsadd[idx].fdmax = 0;// N = 1, E = 4, S = 16, W = 64, NONE = 0
            cvsadd[idx].bcData_curOrder = 0;
            cvsadd[idx].sourceRFapp_dt_meter = 0;
            cvsadd[idx].rfReadintensity_mPsec = 0;
            cvs[idx].isSimulatingCell = -1;
        }


        public static stGlobalValues initGlobalValues(cProject prj, cDomain dm)
        {
            stGlobalValues gv = new stGlobalValues();
            gv.dx = dm.dx;
            gv.nCols = dm.nCols;
            gv.nRows = dm.nRows;
            gv.slpMinLimitforFlow = cGenEnv.slpMinLimitforFlow;
            gv.domainOutBedSlope = cHydro.domainOutBedSlope;
            gv.ConvgC_h = cGenEnv.convergenceConditionh;
            gv.froudNCriteria = cHydro.froudeNCriteria;
            gv.iNRmax_forCE = cGenEnv.iNRmax_forCE;
            gv.iGSmax = cGenEnv.iGSmax;
            gv.gravity = cGenEnv.gravity;
            if (cGenEnv.isDWE == true)
            {
                gv.isDWE = 1;
            }
            else
            {
                gv.isDWE = -1;
            }
            if (cGenEnv.isAnalyticSolution == true)
            {
                gv.isAnalyticSolution = 1;
            }
            else
            {
                gv.isAnalyticSolution = -1;
            }
            if (cHydro.applyVNC == 1)
            {
                gv.isApplyVNC = 1;
            }
            else
            {
                gv.isApplyVNC = -1;
            }
            return gv;
        }


        public static void initBCInfo(cProject prj,  stBCinfo[] bcinfo)
        {
            int idx = 0;
            for (int i=0; i< prj.mBCCount;i++)
            {
                for (int ci = 0; ci<prj.bcData[i].bcCells.Count; ci++)
                {
                    bcinfo[idx].cvid = prj.domain.dminfo[prj.bcData[i].bcCells[ci].x, prj.bcData[i].bcCells[ci].y].cvid;

                    switch (prj.bcData[i].conditionDataType)
                    {
                        case cVars.ConditionDataType.Discharge:
                            bcinfo[idx].bctype= 1;
                            break;
                        case cVars.ConditionDataType.Depth:
                            bcinfo[idx].bctype = 2;
                            break;
                        case cVars.ConditionDataType.Height:
                            bcinfo[idx].bctype = 3;
                            break;
                        case cVars.ConditionDataType.None:
                            bcinfo[idx].bctype = 0;
                            break;
                    }
                    idx++;
                }
            }
        }

        public static bool changeDEMFileUsingArray(cDomain dm, Dataset.projectds.DEMFileToChangeRow row, 
            double tnow_min_bak, stCVAtt[] cvs, double tnow_min)
        {
            double minuteout = 0;
            bool isnormal = true;
            if (double.TryParse(row.TimeMinute, out minuteout) == true)
            {
                if (tnow_min_bak < minuteout && tnow_min >= minuteout)
                {
                    isnormal = cDomain.changeDomainElevWithDEMFileUsingArray(row.DEMFile, dm, cvs);
                    if (isnormal == true) { cGenEnv.writelog(string.Format("DEM file was changed. "), true); }
                }
            }
            return isnormal;
        }


        public static bool changeDEMFile(cDomain dm, Dataset.projectds.DEMFileToChangeRow row, double tnow_min_bak)
        {
            double minuteout = 0;
            bool isnormal = true;
            if (double.TryParse(row.TimeMinute, out minuteout) == true)
            {
                if (tnow_min_bak < minuteout && cGenEnv.tnow_min >= minuteout)
                {
                    isnormal = cDomain.changeDomainElevWithDEMFile(row.DEMFile, dm);
                    if (isnormal == true) { cGenEnv.writelog(string.Format("DEM file was chaged. "), true); }
                }
            }
            return isnormal;
        }

        public void initilizeThisStepUsingArray(stCVAtt[] cvs, stCVAtt_add[] cvsadd, stBCinfo[] bcinfo, double dt_sec, double nowt_sec, 
            int bcdt_sec, int rainfallisEnded)
        {
            if (cGenEnv.isparallel == 1)
            {
                var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
                Parallel.ForEach(Partitioner.Create(0, cvs.Length), options,
                            (range) =>
                            {
                                for (int i = range.Item1; i < range.Item2; i++)
                                {
                                    initializeAcellUsingArray(cvs, i, cvsadd, bcinfo, dt_sec, bcdt_sec, nowt_sec, rainfallisEnded);
                                }
                            });
            }
            else
            {
                for (int i = 0; i < cvs.Length; i++)
                {
                    initializeAcellUsingArray(cvs, i, cvsadd, bcinfo, dt_sec, bcdt_sec, nowt_sec, rainfallisEnded);
                }
            }
        }


        public void initializeAcellUsingArray(stCVAtt[] cvs, int idx, stCVAtt_add[] cvsadd, stBCinfo[] bcinfos, double dt_sec,
        int dtbc_sec, double nowt_sec, int rainfallisEnded)
        {
            double h = cvs[idx].dp_tp1 + cvs[idx].elez; //elev 가 변경되는 경우가 있으므로, 이렇게 수위설정
            if (cvs[idx].hp_tp1 <= h)
            {
                // dem  고도 변경되면, 수심이 바뀐다. 수위는 유지.
                // cvs[idx].hp_t=cvs[idx].elez + cvs[idx].dp_t 이므로, cvs[idx].dp_t 이값과 cvs[idx].dp_tp1  모두 업데이트 해줘야 한다.
                cvs[idx].dp_tp1 = cvs[idx].hp_tp1 - cvs[idx].elez;
                if (cvs[idx].dp_tp1 < 0) { cvs[idx].dp_tp1 = 0; }
                cvs[idx].dp_t = cvs[idx].dp_tp1;
            }
            else
            {
                cvs[idx].dp_t = cvs[idx].dp_tp1;
            }
            cvs[idx].qe_t = cvs[idx].qe_tp1;
            cvs[idx].qw_t = cvs[idx].qw_tp1;
            cvs[idx].qs_t = cvs[idx].qs_tp1;
            cvs[idx].qn_t = cvs[idx].qn_tp1;
            double sourceAlltoRoute_tp1_dt_m = 0;
            int bid = cConditionData.getbcArrayIndex(bcinfos, idx);
            if (bid >= 0)
            {
                bcinfos[bid].bcDepth_dt_m_tp1 = getConditionDataAsDepthWithLinear(bcinfos[bid].bctype, cvs[idx].elez, cvsadd[idx], dx, dt_sec, dtbc_sec, nowt_sec);
                if (bcinfos[bid].bctype == 1)
                {//경계조건이 유량일 경우, 소스항에 넣어서 홍수추적한다. 수심으로 환산된 유량..
                    sourceAlltoRoute_tp1_dt_m = bcinfos[bid].bcDepth_dt_m_tp1;
                }
                else
                {//경계조건이 유량이 아닐경우, 홍수추적 하지 않고, 고정된 값 적용.
                    cvs[idx].dp_tp1 = bcinfos[bid].bcDepth_dt_m_tp1;
                    if (cGenEnv.tnow_sec == 0)
                    {
                        cvs[idx].dp_t = cvs[idx].dp_tp1;
                    }
                }
            }

            cvsadd[idx].sourceRFapp_dt_meter = 0;
            //-1:false, 1: true
            if (isRainfallApplied == 1 && rainfallisEnded == -1)
            {
                if (rainfallDataType == 2)
                {
                    cvsadd[idx].sourceRFapp_dt_meter = cvsadd[idx].rfReadintensity_mPsec * dt_sec;
                }
                else
                {
                    cvsadd[idx].rfReadintensity_mPsec = cThisProcess.rfReadintensityForMAP_mPsec;
                    cvsadd[idx].sourceRFapp_dt_meter = cThisProcess.rfReadintensityForMAP_mPsec * dt_sec;
                }
            }
            sourceAlltoRoute_tp1_dt_m = sourceAlltoRoute_tp1_dt_m + cvsadd[idx].sourceRFapp_dt_meter;
            cvs[idx].dp_t = cvs[idx].dp_t + sourceAlltoRoute_tp1_dt_m;
            cvs[idx].dp_tp1 = cvs[idx].dp_tp1 + sourceAlltoRoute_tp1_dt_m;
            cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
            if (cvs[idx].dp_tp1 > cGenEnv.dMinLimitforWet) { setEffectiveCellUsing1DArray(cvs, idx); }

        }

        public static void initializeAcellUsingArray_static_trial(stCVAtt[] cvs, int idx, stGlobalValues gv)
        {
            ////cell.resd = double.MaxValue;
            //cvs[idx].converged = 1;
            ////bool beginDry = true;

            ////if (cvs[idx].dp_tp1 > cGenEnv.dMinLimitforWet) { beginDry = false; }
            //double h = cvs[idx].dp_tp1 + cvs[idx].elez; //elev 가 변경되는 경우가 있으므로, 이렇게 수위설정
            //if (cvs[idx].hp_tp1 <= h)
            //{
            //    cvs[idx].dp_t = cvs[idx].hp_tp1 - cvs[idx].elez;
            //    if (cvs[idx].dp_t < 0) { cvs[idx].dp_t = 0; }
            //    cvs[idx].hp_t = cvs[idx].elez + cvs[idx].dp_t;
            //}
            //else
            //{
            //    cvs[idx].hp_t = h;
            //    cvs[idx].dp_t = cvs[idx].dp_tp1;
            //}
            //cvs[idx].qe_t = cvs[idx].qe_tp1;
            //cvs[idx].qw_t = cvs[idx].qw_tp1;
            //cvs[idx].qs_t = cvs[idx].qs_tp1;
            //cvs[idx].qn_t = cvs[idx].qn_tp1;
            //cvs[idx].ve_t = cvs[idx].ve_tp1;
            //cvs[idx].vw_t = cvs[idx].vw_tp1;
            //cvs[idx].vs_t = cvs[idx].vs_tp1;
            //cvs[idx].vn_t = cvs[idx].vn_tp1;
            //cvs[idx].bcDepth_dt_m_t = cvs[idx].bcDepth_dt_m_tp1;
            //cvs[idx].sourceAlltoRoute_t_dt_m = cvs[idx].sourceAlltoRoute_tp1_dt_m;
            //cvs[idx].sourceAlltoRoute_tp1_dt_m = 0;
            //if (cvs[idx].isBCcell == 1)
            //{
            //    cvs[idx].bcDepth_dt_m_tp1 = getConditionDataAsDepthWithLinear(cvs[idx], gv.dx, gv.dt_sec, gv.dtbc_sec, gv.tnow_sec);
            //    //if (cvs[idx].cvConditionDataType == cVars.ConditionDataType.Discharge)
            //    if (cvs[idx].cvConditionDataType == 1)
            //    {//경계조건이 유량일 경우, 소스항에 넣어서 홍수추적한다. 수심으로 환산된 유량..
            //        cvs[idx].sourceAlltoRoute_tp1_dt_m = cvs[idx].bcDepth_dt_m_tp1;
            //    }
            //    else
            //    {//경계조건이 유량이 아닐경우, 홍수추적 하지 않고, 고정된 값 적용.
            //        cvs[idx].dp_tp1 = cvs[idx].bcDepth_dt_m_tp1;
            //        if (gv.tnow_sec == 0)
            //        {
            //            cvs[idx].dp_t = cvs[idx].dp_tp1;
            //            cvs[idx].hp_t = cvs[idx].dp_tp1 + cvs[idx].elez;
            //        }
            //    }
            //}

            //cvs[idx].sourceRFapp_dt_meter = 0;
            //if (gv.isRainfallApplied == 1 && gv.rainfallisEnded == -1)
            //{
            //    if (gv.rainfallDataType == 2)
            //    {
            //        cvs[idx].sourceRFapp_dt_meter = cvs[idx].rfReadintensity_mPsec * gv.dt_sec;
            //    }
            //    else
            //    {
            //        cvs[idx].rfReadintensity_mPsec = gv.rfReadintensityForMAP_mPsec;
            //        cvs[idx].sourceRFapp_dt_meter = gv.rfReadintensityForMAP_mPsec * gv.dt_sec;
            //    }
            //}
            //cvs[idx].sourceAlltoRoute_tp1_dt_m = cvs[idx].sourceAlltoRoute_tp1_dt_m + cvs[idx].sourceRFapp_dt_meter
            //                                  - cvs[idx].sinkFlowDepth_dt_m;
            //cvs[idx].dp_t = cvs[idx].dp_t + cvs[idx].sourceAlltoRoute_tp1_dt_m;
            //cvs[idx].hp_t = cvs[idx].dp_t + cvs[idx].elez;
            //cvs[idx].dp_tp1 = cvs[idx].dp_tp1 + cvs[idx].sourceAlltoRoute_tp1_dt_m;
            //cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;

            ////if (gv.isMovingDomain == 1)
            ////{
            //if (cvs[idx].dp_tp1 > gv.dMinLimitforWet)
            //{
            //    setSimulatingCellUsingArray(cvs, idx);
            //}
            //}
        }

        public static double getConditionDataAsDepthWithLinear(int bctype, double elev_m, stCVAtt_add cvadd, double dx, double dtsec,
            int dtsec_cdata, double nowt_sec)
        {
            double vcurOrder = cvadd.bcData_curOrder;
            double vnextOrder = cvadd.bcData_nextOrder;
            double valueAsDepth_curOrder = 0;
            double valueAsDepth_nextOrder = 0;
            //1:  Discharge,  2: Depth, 3: Height,  4: None
            if (bctype == 1)
            {
                valueAsDepth_curOrder = (vcurOrder / dx / dx) * dtsec;
                valueAsDepth_nextOrder = (vnextOrder / dx / dx) * dtsec;
            }
            if (bctype == 2)
            {
                valueAsDepth_curOrder = vcurOrder;
                valueAsDepth_nextOrder = vnextOrder;
            }
            if(bctype==3)
            {
                valueAsDepth_curOrder = vcurOrder - elev_m;
                valueAsDepth_nextOrder = vnextOrder - elev_m;
            }
            if (valueAsDepth_curOrder < 0) { valueAsDepth_curOrder = 0; }
            if (valueAsDepth_nextOrder < 0) { valueAsDepth_nextOrder = 0; }
            double bcDepth_dt_m_tp1 = 0;
            if (cGenEnv.isAnalyticSolution == false)
            {
                bcDepth_dt_m_tp1 = (valueAsDepth_nextOrder - valueAsDepth_curOrder)
                       * (nowt_sec - cvadd.bcData_curOrderStartedTime_sec) / dtsec_cdata + valueAsDepth_curOrder;
            }
            else
            {
                bcDepth_dt_m_tp1 = valueAsDepth_curOrder;
            }
            return bcDepth_dt_m_tp1;
        }

        [GpuManaged]
        public static void setEffectiveCellUsing1DArray(stCVAtt[] inCVs, int idx)
        {
            inCVs[idx].isSimulatingCell = 1;
            if (inCVs[idx].cvaryNum_atE >= 0) { inCVs[inCVs[idx].cvaryNum_atE].isSimulatingCell = 1; }
            if (inCVs[idx].cvaryNum_atW >= 0) { inCVs[inCVs[idx].cvaryNum_atW].isSimulatingCell = 1; }
            if (inCVs[idx].cvaryNum_atN >= 0) { inCVs[inCVs[idx].cvaryNum_atN].isSimulatingCell = 1; }
            if (inCVs[idx].cvaryNum_atS >= 0) { inCVs[inCVs[idx].cvaryNum_atS].isSimulatingCell = 1; }
        }

        //[GpuManaged]
        //public void callTest()
        //{
        //    var gpu = Gpu.Default;
        //    var lp = new LaunchParam(256, 16);
        //    //double[,] c = new double[10, 10];
        //    //gpu.Launch(setStaringCondition, lp, cvs, nCols, nRows, iniDepth, cGenEnv.movingDomain);
        //    //cTest[] tt = new cTest[100];
        //    //gpu.Launch(called, lp, tt);
        //    //double dbltest= new double();
        //    //gpu.Launch(called, lp, dbltest);

        //}


        //public void called(cTest[] incls)
        //{
        //    for (int ncx = 0; ncx < 100; ncx++)
        //    {
        //        incls[ncx].nd = 1;
        //    }
        //}


        //public void called(double incls)
        //{
        //    for (int ncx = 0; ncx < 100; ncx++)
        //    {
        //        incls = 1;
        //    }
        //}    
    }
}
