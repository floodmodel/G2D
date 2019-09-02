using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Threading;
using System.Collections.Concurrent;
using Alea;
using Alea.Fody;
using Alea.FSharp;
using Alea.CSharp; // gpu.Launch는 이게 참조되어야 한다.
using System.Linq;




namespace G2DCore
{
    public class cSolver
    {
        private cDomain mDM = new cDomain();
        private int nRows;
        private int nCols;
        private double dx = 0;
        private ParallelOptions mPoptions = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };

        public cSolver(cDomain dm)
        {
            mDM = dm;
            dx = dm.dx;
            nRows = dm.nRows;
            nCols = dm.nCols;
        }

        public cSolver()
        {
        }
               

        public void RunSolverUsing1DArray(stCVAtt[] cvs, stCVAtt_add[] cvsadd, double dt_sec, stGlobalValues[] gv, stBCinfo[] bcinfos)
        {
            int igs = 0;
            var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            cThisProcess.maxNR_inME = 0;

            for (igs = 0; igs < cGenEnv.iGSmax; igs++) 
            {
                gv[0].bAllConvergedInThisGSiteration = 1;
                gv[0].iNR = 0;

                if (cGenEnv.isparallel == 1)
                {
                    #region "parallel for each"
                    //이렇게 하면, dem 값이 있는 셀만 접근하므로, 속도 향상 된다. 또한 유역 형상에 상관없이 같은 성능 나온다.
                    //0은 포함, dm.cvCount는 제외됨 
                    Parallel.ForEach(Partitioner.Create(0, cvs.Length), options,
                        (range) =>
                        {
                            for (int i = range.Item1; i < range.Item2; i++)
                            {
                                if (cvs[i].isSimulatingCell == 1)
                                {
                                    int bcCellid = cConditionData.getbcArrayIndex(bcinfos, i);
                                    int isBCcell = -1;
                                    double bcDepth = 0;
                                    int bctype = 0;
                                    if (bcCellid >= 0) { isBCcell = 1; bcDepth = bcinfos[bcCellid].bcDepth_dt_m_tp1; bctype = bcinfos[bcCellid].bctype; }
                                    calculateContinuityEqUsingNRUsing1DArrayForCPU(cvs, i, gv, isBCcell, bcDepth, bctype);
                                    if (cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet) { cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i); }
                                }
                            }
                        });
                    #endregion

                }
                else
                {
                    for (int i = 0; i < cvs.Length; i++)
                    {
                        if (cvs[i].isSimulatingCell == 1)
                        {
                            int bcCellid = cConditionData.getbcArrayIndex(bcinfos, i);
                            int isBCcell = -1;
                            double bcDepth = 0;
                            int bctype = 0;
                            if (bcCellid >= 0) { isBCcell = 1; bcDepth = bcinfos[bcCellid].bcDepth_dt_m_tp1; bctype = bcinfos[bcCellid].bctype; }
                            calculateContinuityEqUsingNRUsing1DArrayForCPU(cvs, i, gv, isBCcell, bcDepth, bctype);
                            if (cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet) { cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i); }
                        }
                    }
                }
                gv[0].iGS = igs+1;

                if (gv[0].bAllConvergedInThisGSiteration == 1)
                {
                    break;
                }
            }
            //여기까지 gs iteration

            UpdateValuesInThisStepResultsUsing1DArray(cvs, cvsadd);
            if (gv[0].bAllConvergedInThisGSiteration == 1)
            {
                if (cGenEnv.bwritelog_process == true)
                {
                    cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
                                          cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
                }
            }
            else
            {
                if (cGenEnv.bwritelog_process == true)
                {
                    cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was not converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
                                        cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
                }
            }
        }



        [GpuManaged]
        public void DummyKernelLaunch()
        {
            var gpu = Gpu.Default;
            var lp = new LaunchParam(16, 256);
            double[] a = new double[10];
            gpu.Launch(DummyKernel, lp, a);
            gpu.Synchronize();
            //Gpu.FreeAllImplicitMemory(); // 이거하는데 10ms 정도 소요된다.
        }

        private void DummyKernel(double[] a)
        {

        }

      

        [GpuManaged]
        public static void calculateContinuityEqUsingNRUsing1DArrayForCPU(stCVAtt[] cvs, int idx, stGlobalValues[] gv,
            int isBCCell, double dcdtpth, int bctype)
        {
            //double sourceTerm = (cell.sourceAlltoRoute_tp1_dt_m + cell.sourceAlltoRoute_t_dt_m) / 2; //이건 Crank-Nicolson 방법.  dt는 이미 곱해서 있다..
            //double c1_CR = gv[0].dt_sec / gv[0].dx / 2; //이건 CR
            //double sourceTerm = cvs[idx].sourceAll_tp1_dt_m; //dt는 이미 곱해서 있다... 이건 음해법
            //double c1_IM = gv[0].dt_sec / gv[0].dx;//이건 음해법
            double dp_old = cvs[idx].dp_tp1;
            int inr = 0;
            for (inr = 0; inr < gv[0].iNRmax_forCE; inr++) //cGenEnv.iNRmax_forCE 값 참조
            {
                if (gv[0].iNR < (inr+1)) { gv[0].iNR = inr+1; }  // gv[0].iNR은 현재 셀의 inr 이 아니라, 현재 gs 에서의 max 값이다.
                if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1) { break; }
            }
            cvs[idx].resd = Math.Abs(cvs[idx].dp_tp1 - dp_old);
            if (cvs[idx].resd > gv[0].ConvgC_h) { gv[0].bAllConvergedInThisGSiteration = -1; }
        }

        [GpuManaged]
        public static int NRinner(stCVAtt[] cvs, int idx, stGlobalValues[] gv, 
            int isBCCell, double dbdtpth, int bctype)
        {
            double c1_IM = gv[0].dt_sec / gv[0].dx;//이건 음해법
            double dn = cvs[idx].dp_tp1;
            calculateWFluxUsingArray(cvs, idx, gv[0], isBCCell);
            calculateEFluxUsingArray(cvs, idx, gv[0], isBCCell);
            calculateNFluxUsingArray(cvs, idx, gv[0], isBCCell);
            calculateSFluxUsingArray(cvs, idx, gv[0], isBCCell);
            // 현재 셀의 수위가 올라가려면  -> qe-, qw+, qs-, qn+
            double dnp1 = 0;
            //NR
            double fn = dn - cvs[idx].dp_t + (cvs[idx].qe_tp1 - cvs[idx].qw_tp1 + cvs[idx].qs_tp1 - cvs[idx].qn_tp1) * c1_IM;//- sourceTerm; //이건 음해법
            double eElem = DeviceFunction.Pow(cvs[idx].dfe, (double)2 / 3) * DeviceFunction.Sqrt(DeviceFunction.Abs(cvs[idx].slpe)) / cvs[idx].rc;
            double sElem = DeviceFunction.Pow(cvs[idx].dfs, (double)2 / 3) * DeviceFunction.Sqrt(DeviceFunction.Abs(cvs[idx].slps)) / cvs[idx].rc;
            double dfn = 1 + (eElem + sElem) * (5.0 / 3) * c1_IM;// 이건 음해법
            if (dfn == 0) { return 1; }
            dnp1 = dn - fn / dfn;
            if (isBCCell == 1 && bctype == 2)// 1:Discharge, 2:Depth, 3:Height, 4:None
            {
                dnp1 = dbdtpth;
            }
            if (dnp1 < 0) { dnp1 = 0; }
            double resd = dnp1 - dn;
            cvs[idx].dp_tp1 = dnp1;
            cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
            if (DeviceFunction.Abs(resd) < gv[0].ConvgC_h)
            {
              return 1 ;
            }
            return -1;
        }





        [GpuManaged]
        public void RunSolverUsingGPU(stCVAtt[] cvs, stCVAtt_add[] cvsadd, ref stGlobalValues[] gv, stBCinfo[] bcinfos)
        {
            var gpu = Gpu.Default;
            var lp = new LaunchParam(16, 256);
            gpu.Launch(RunSolverUsingGPU_kernel, lp, cvs, gv, bcinfos);
            //var test_uni = gpu.AllocatePinned<int>(cvs.Length);
            //var cvs_pinned = gpu.AllocatePinned<stCVAtt>(cvs.Length);
            //var gv_pinned = gpu.AllocatePinned<stGlobalValues>(gv.Length);
            //var bcinfos_pinned = gpu.AllocatePinned<stBCinfo>(bcinfos.Length);
            //Gpu.Copy(cvs, cvs_pinned);
            //Gpu.Copy(gv, gv_pinned);
            //Gpu.Copy(bcinfos, bcinfos_pinned);
            ////var cvs_pinned_ar = cvs_pinned.ToArray();
            ////var gv_pinned_ar = gv_pinned.ToArray();
            ////var bcinfos_pinned_ar = bcinfos_pinned.ToArray();

            //for (int n = 0; n < cvs.Length; n++)
            //{
            //    //Gpu.Copy(cvs, cvs_pinned[n]);
            //    test_uni[n] = cvs_pinned[n].colxary;
            //    stGlobalValues stg;
            //    stg.iGS = n;
            //    int[] a = new int[1];
            //    a[0] = 1;
            //    gv_pinned[0].iGS = a[0];
            //    gv[0].iGS = n;
            //}
            //gpu.Launch(RunSolverUsingGPU_kernel, lp, cvs_pinned, gv_pinned, bcinfos_pinned);
            ////gpu.Launch(RunSolverUsingGPU_kernel, lp, cvs_pinned_ar, gv_pinned_ar, bcinfos_pinned_ar);


            //var test_uni = gpu.AllocateUnified<int>(cvs.Length);

            //var cvs_uni = gpu.AllocateUnified<stCVAtt>(cvs.Length);
            //var gv_uni = gpu.AllocateUnified<stGlobalValues>(gv.Length);
            //var bcinfos_uni = gpu.AllocateUnified<stBCinfo>(bcinfos.Length);

            //for (int n = 0; n < cvs.Length; n++)
            //{
            //    test_uni[n] = cvs[n].colxary;
            //    cvs_uni[n].colxary = cvs[n].colxary; // 구조체 배열  값 수정 안된다.
            //}
            //Gpu.Copy(cvs, cvs_uni);
            //Gpu.Copy(gv, gv_uni);
            //Gpu.Copy(bcinfos, bcinfos_uni);

            //var cvs_ptr = cvs_uni.Ptr;
            //var gv_ptr = gv_uni.Ptr;
            //var bcinfos_ptr = bcinfos_uni.Ptr;

            //gpu.Launch(RunSolverUsingGPU_kernel, lp, cvs_uni, gv_uni, bcinfos_uni);
            //gpu.Launch(RunSolverUsingGPU_kernel, lp, cvs_ptr, gv_ptr, bcinfos_ptr);

            //gpu.Synchronize();
            UpdateValuesInThisStepResultsUsing1DArray(cvs, cvsadd);
            //Gpu.FreeAllImplicitMemory(); // 이거하는데 10ms 정도 소요된다.
        }


        [GpuManaged]
        //public static void RunSolverUsingGPU_kernel<T>(PinnedMemory<stCVAtt> cvs,
        //    PinnedMemory<stGlobalValues> gv, PinnedMemory<stBCinfo> bcinfos)
        //public static void RunSolverUsingGPU_kernel(UnifiedMemory <stCVAtt> cvs,
        //    UnifiedMemory<stGlobalValues> gv, UnifiedMemory<stBCinfo> bcinfos)
        //        public static void RunSolverUsingGPU_kernel(UnifiedMemory<stCVAtt> cvs,
        //UnifiedMemory<stGlobalValues> gv, UnifiedMemory<stBCinfo> bcinfos)
        public static void RunSolverUsingGPU_kernel(stCVAtt[] cvs, stGlobalValues[] gv, stBCinfo[] bcinfos)
        {
            //var start = blockIdx.x * blockDim.x + threadIdx.x;
            //var stride = gridDim.x * blockDim.x;
            //var startGS = blockIdx.y * blockDim.y + threadIdx.y;
            //var strideGS = gridDim.y * blockDim.y;
            //cudaHostAlloc()
            //CUDAInterop.cuMemAllocHost

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 1;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax==1) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 2;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 2) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 3;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 3) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 4;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 4) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 5;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 5) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 6;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 6) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 7;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 7) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 8;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 8) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 9;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 9) { return; }

            GSinnerForGPU(cvs, gv, bcinfos);
            gv[0].iGS = 10;
            if (gv[0].bAllConvergedInThisGSiteration == 1 || gv[0].iGSmax == 10) { return; }
        }


        [GpuManaged]
        //public static void GSinnerForGPU(UnifiedMemory< stCVAtt> cvs, 
        //    UnifiedMemory<stGlobalValues> gv, 
        //    UnifiedMemory <stBCinfo> bcinfos)
        // DeviceFunction은 NVIDIA 그래픽 카드가 없어도 사용할 수 있다. 2019.02.20. 원이사님 라데온 카드에서 테스트 
        // NVIDIA 그래픽 카드가 없을 경우, 반드시 CPU 사용으로 설정(UsingGPU=false)하고, 사용해도 DeviceFunction에서 애러 안난다. 2019.02.20. 원이사님 라데온 카드에서 테스트 
        public static void GSinnerForGPU(stCVAtt[] cvs,
            stGlobalValues[] gv,
            stBCinfo[] bcinfos)
        {
            var start = blockIdx.x * blockDim.x + threadIdx.x;
            var stride = gridDim.x * blockDim.x;
            gv[0].iNR = 0;
            for (int i = start; i < cvs.Length; i += stride)
            {
                if (cvs[i].isSimulatingCell == 1)
                {
                    int bcCellid = cConditionData.getbcArrayIndex(bcinfos, i);
                    int isBCcell = -1;
                    double bcDepth = 0;
                    int bctype = 0;
                    double dp_old = cvs[i].dp_tp1;
                    if (bcCellid >= 0) { isBCcell = 1; bcDepth = bcinfos[bcCellid].bcDepth_dt_m_tp1; bctype = bcinfos[bcCellid].bctype; }
                    calculateContinuityEqUsingNRUsing1DArrayForGPU(cvs, i, gv, isBCcell, bcDepth, bctype);
                    cvs[i].resd = DeviceFunction.Abs(cvs[i].dp_tp1 - dp_old);
                    if (cvs[i].resd > gv[0].ConvgC_h) { gv[0].bAllConvergedInThisGSiteration = -1; } // 현재 셀의 nr은 수렴했지만, 여기서는 -1 이 설정될 수 있다.
                    if (cvs[i].dp_tp1 > gv[0].dMinLimitforWet) { cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i); }
                }
            }
        }


        [GpuManaged]
        public static void calculateContinuityEqUsingNRUsing1DArrayForGPU(stCVAtt[] cvs, int idx, stGlobalValues[] gv,
          int isBCCell, double dcdtpth, int bctype)
        {

            if (gv[0].iNR < 1) { gv[0].iNR = 1; }  // gv[0].iNR은 현재 셀의 inr 이 아니라, 현재 gs 에서의 max 값이다. gpu에서 max 값 잡기 어렵다
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 1) { return; }

            if (gv[0].iNR < 2) { gv[0].iNR = 2; }
            //gv[0].iNR = 2;
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 2) { return; }

            if (gv[0].iNR < 3) { gv[0].iNR = 3; }
            //gv[0].iNR = 3;
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 3) { return; }

            if (gv[0].iNR < 4) { gv[0].iNR = 4; }
            //gv[0].iNR = 4;
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 4) { return; }

            if (gv[0].iNR < 5) { gv[0].iNR = 5; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 5) { return; }

            if (gv[0].iNR < 6) { gv[0].iNR = 6; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 6) { return; }

            if (gv[0].iNR < 7) { gv[0].iNR = 7; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 7) { return; }

            if (gv[0].iNR < 8) { gv[0].iNR = 8; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 8) { return; }

            if (gv[0].iNR < 9) { gv[0].iNR = 9; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 9) { return; }

            if (gv[0].iNR < 10) { gv[0].iNR = 10; }
            if (NRinner(cvs, idx, gv, isBCCell, dcdtpth, bctype) == 1 || gv[0].iNRmax_forCE == 10) { return; }

        }

        public void UpdateValuesInThisStepResultsUsing1DArray(stCVAtt[] cvs, stCVAtt_add[] cvsadd)
        {
            var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            cGenEnv.dflowmaxInThisStep = -9999;
            cGenEnv.vmaxInThisStep = -9999;
            cGenEnv.VNConMinInThisStep = 9999;
            cThisProcess.maxResd = 0;
            cThisProcess.subregionDflowmax = new double[nRows];
            cThisProcess.subregionVmax = new double[nRows];
            cThisProcess.subregionVNCmin = new double[nRows];
            cThisProcess.subregionMaxResd = new double[nRows];
            cThisProcess.subregionMaxResdCell = new string[nRows];

            Parallel.For(0, nRows, options, delegate (int ry)
            //for (int ry = 0; ry < nRows; ry++)
            {
                double maxdflow = 0;
                double maxv = 0;
                double minvnc = 9999;
                double maxR = 0;
                string maxRCell = "";
                for (int cx = 0; cx < nCols; cx++)
                {
                    int idx = mDM.dminfo[cx, ry].cvid;
                    if (idx > -1 && cvs[idx].isSimulatingCell == 1)
                    {
                        stFluxData flxmax;
                        if (cvs[idx].cvaryNum_atW >= 0 && cvs[idx].cvaryNum_atN >= 0)//  이경우는 4개 방향 성분에서 max 값 얻고
                        {
                            flxmax = cHydro.getFD4MaxUsingGPU(cvs[idx], cvs[cvs[idx].cvaryNum_atW], cvs[cvs[idx].cvaryNum_atN]);

                        }
                        else// 이경우는 s, e에서 max 값 얻는다
                        {
                            flxmax = cHydro.getFD4MaxUsingGPU(cvs[idx], cvs[idx], cvs[idx]);
                        }

                        cvsadd[idx].fdmax = flxmax.fd;
                        cvsadd[idx].vmax = flxmax.v;
                        cvsadd[idx].Qmax_cms = flxmax.q * dx;
                        if (flxmax.dflow > maxdflow) { maxdflow = flxmax.dflow; }
                        if (cvsadd[idx].vmax > maxv) { maxv = cvsadd[idx].vmax; }
                        double vnCon = 0;
                        if (cHydro.applyVNC == 1) { vnCon = getVonNeumanConditionValueUsingGPUFunction(cvs[idx]); }
                        if (vnCon < minvnc) { minvnc = vnCon; }
                        if (cvs[idx].resd > maxR) { maxR = cvs[idx].resd; maxRCell = "(" + cvs[idx].colxary.ToString() + "," + cvs[idx].rowyary.ToString() + ")"; }
                    }
                }
                cThisProcess.subregionDflowmax[ry] = maxdflow;
                cThisProcess.subregionVmax[ry] = maxv;
                cThisProcess.subregionVNCmin[ry] = minvnc;
                cThisProcess.subregionMaxResd[ry] = maxR;
                cThisProcess.subregionMaxResdCell[ry] = maxRCell;
                //}
            });

            for (int nary = 0; nary < nRows; nary++)
            {
                if (cGenEnv.dflowmaxInThisStep < cThisProcess.subregionDflowmax[nary]) { cGenEnv.dflowmaxInThisStep = cThisProcess.subregionDflowmax[nary]; }
                if (cGenEnv.vmaxInThisStep < cThisProcess.subregionVmax[nary]) { cGenEnv.vmaxInThisStep = cThisProcess.subregionVmax[nary]; }
                if (cGenEnv.VNConMinInThisStep > cThisProcess.subregionVNCmin[nary]) { cGenEnv.VNConMinInThisStep = cThisProcess.subregionVNCmin[nary]; }
                if (cThisProcess.maxResd < cThisProcess.subregionMaxResd[nary])
                {
                    cThisProcess.maxResd = cThisProcess.subregionMaxResd[nary];
                    cThisProcess.maxResdCell = cThisProcess.subregionMaxResdCell[nary];
                }
            }
        }

        [GpuManaged]
        public static stFluxData noFlx ()
        {
            stFluxData noFlx = new stFluxData();
            noFlx.dflow = 0;
            noFlx.fd = 0;
            noFlx.q = 0;
            noFlx.slp = 0;
            noFlx.v = 0;
            return noFlx;
        }

        [GpuManaged]
        private static void calculateWFluxUsingArray(stCVAtt[] cvs, int idx, stGlobalValues gv, int isBCcell)
        {
            if (gv.nCols== 1) { return; }
            stFluxData flxw = new stFluxData(); //W, x-
            if (cvs[idx].colxary == 0 || cvs[idx].cvaryNum_atW == -1)//w 측 경계셀
            {
                if (isBCcell == 1)
                {
                    flxw = noFlx(); // w측 최 경계에서는 w 방향으로 flx 없다.
                }
                else
                {// w측 최 경계에서는 w 방향으로 자유수면 flx 있다.
                    double slp_tm1 = 0;
                    if (cvs[idx].cvaryNum_atE >=0)
                    {
                        double he = cvs[cvs[idx].cvaryNum_atE].dp_t + cvs[cvs[idx].cvaryNum_atE].elez;
                        double hcur = cvs[idx].dp_t + cvs[idx].elez;
                        slp_tm1 = (he - hcur) / gv.dx; //i+1 셀과의 e 수면경사를 w 방향에 적용한다.
                    }
                    //double slp_tm1 = (cvs[cvs[idx].cvaryNum_atE].hp_t - cvs[idx].hp_t) / gv.dx; //i+1 셀과의 수면경사를 w 방향에 적용한다.
                    slp_tm1 = slp_tm1 + gv.domainOutBedSlope;
                    if (slp_tm1 >= gv.slpMinLimitforFlow && cvs[idx].dp_tp1 > gv.dMinLimitforWet)
                    {
                        flxw = calculateMomentumEQ_DWEm_DeterministricUsingGPU(cvs[idx], cvs[idx].qw_t, gv.gravity,gv.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1,0);
                    }
                    else { flxw = noFlx(); }
                }
            }
            else
            {
                if (cvs[idx].isSimulatingCell == -1)
                {
                    flxw = noFlx();
                }
                else
                {
                    flxw.v = cvs[cvs[idx].cvaryNum_atW].ve_tp1;
                    flxw.slp = cvs[cvs[idx].cvaryNum_atW].slpe;
                    flxw.q = cvs[cvs[idx].cvaryNum_atW].qe_tp1;
                    flxw.dflow = cvs[cvs[idx].cvaryNum_atW].dfe;
                }
            }
            //cvs[idx].slpw = flxw.slp;
            //cvs[idx].dfw = flxw.dflow;
            //cvs[idx].vw_tp1 = flxw.v;
            cvs[idx].qw_tp1 = flxw.q;
        }

        private static void calculateEFluxUsingArray(stCVAtt[] cvs, int idx, stGlobalValues gv, int isBCcell)
        {
            if (gv.nCols == 1) { return; }
            stFluxData flxe = new stFluxData();    //E,  x+
            if (cvs[idx].colxary == (gv.nCols - 1) || cvs[idx].cvaryNum_atE == -1)
            {
                if (isBCcell == 1) { flxe = noFlx(); }
                else
                {
                    double slp_tm1 = 0;
                    if (cvs[idx].cvaryNum_atW >= 0)
                    {
                        //double slp = (cell.hp_tp1 - dm.cells[cx - 1, ry].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                        double hw = cvs[cvs[idx].cvaryNum_atW].dp_t + cvs[cvs[idx].cvaryNum_atW].elez;
                        double hcur = cvs[idx].dp_t + cvs[idx].elez;
                        slp_tm1 = (hcur - hw) / gv.dx;
                    }
                    //double slp_tm1 = (cvs[idx].hp_t - cvs[cvs[idx].cvaryNum_atW].hp_t) / gv.dx;
                    slp_tm1 = slp_tm1 - gv.domainOutBedSlope;
                    if (slp_tm1 <= (-1 * gv.slpMinLimitforFlow) && cvs[idx].dp_tp1 > gv.dMinLimitforWet)
                    {
                        flxe = calculateMomentumEQ_DWEm_DeterministricUsingGPU(cvs[idx], cvs[idx].qe_t, gv.gravity, gv.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1,0);
                    }
                    else { flxe = noFlx(); }
                }
            }
            else
            {
                if (cvs[idx].isSimulatingCell == -1)
                {
                    flxe = noFlx();
                }
                else
                {
                    //stCVAtt pcell = new stCVAtt();
                    //if (cvs[idx].colxary > 0 && cvs[idx].cvaryNum_atW>-1) { pcell = cvs[cvs[idx].cvaryNum_atW]; }
                    //else { pcell = cvs[idx]; }
                    //E = 1, S = 3, W = 5, N = 7, NONE = 0
                    //flxe = getFluxToEastOrSouth(dx, cell, pcell, mDM.cells[cx + 1, ry], cVars.FlowDirection4.E, dt_sec);
                    //flxe = getFluxToEastOrSouthUsing1DArray(cvs[idx], pcell, cvs[cvs[idx].cvaryNum_atE], 1, gv);
                    flxe = getFluxToEastOrSouthUsing1DArray(cvs[idx], cvs[cvs[idx].cvaryNum_atE], 1, gv);
                }
            }
            cvs[idx].ve_tp1 = flxe.v;
            cvs[idx].dfe = flxe.dflow;
            cvs[idx].slpe = flxe.slp;
            cvs[idx].qe_tp1 = flxe.q;
        }

        private static void calculateNFluxUsingArray(stCVAtt[] cvs, int idx, stGlobalValues gv, int isBCcell)
        {
            if (gv.nRows == 1) { return; }
            stFluxData flxn = new stFluxData();  //N, y-
            if (cvs[idx].rowyary == 0 || cvs[idx].cvaryNum_atN == -1)
            {
                if (isBCcell == 1) { flxn = noFlx(); }
                else
                {// n측 최 경계에서는 n 방향으로 자유수면 flx 있다.
                    double slp_tm1 = 0;
                    if (cvs[idx].cvaryNum_atS >= 0)
                    {
                        //double slp = (dm.cells[cx, ry + 1].hp_tp1 - cell.hp_tp1) / dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
                        //double slp_tm1 = (cvs[cvs[idx].cvaryNum_atS].hp_t - cvs[idx].hp_t) / gv.dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
                        double hs = cvs[cvs[idx].cvaryNum_atS].dp_t + cvs[cvs[idx].cvaryNum_atS].elez;
                        double hcur = cvs[idx].dp_t + cvs[idx].elez;
                        slp_tm1 = (hs - hcur) / gv.dx;
                    }
                    slp_tm1 = slp_tm1 + gv.domainOutBedSlope;
                    if (slp_tm1 >= gv.slpMinLimitforFlow && cvs[idx].dp_tp1 > gv.dMinLimitforWet)
                    {
                        //flxn = getFluxToDomainOut(cell, slp_tm1, cell.qn_t, cell.vn_t, gv.gravity, dt_sec);
                        //flxn = calculateMomentumEQ_DWEm_DeterministricUsingGPU(gv);
                        flxn = calculateMomentumEQ_DWEm_DeterministricUsingGPU(cvs[idx],cvs[idx].qn_t, gv.gravity, gv.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1,0);
                    }
                    else { flxn = noFlx(); }
                }
            }
            else
            {
                if (cvs[idx].isSimulatingCell == -1)
                {
                    flxn = noFlx();
                }
                else
                {
                    flxn.v = cvs[cvs[idx].cvaryNum_atN].vs_tp1;
                    flxn.slp = cvs[cvs[idx].cvaryNum_atN].slps;
                    flxn.dflow = cvs[cvs[idx].cvaryNum_atN].dfs;
                    flxn.q = cvs[cvs[idx].cvaryNum_atN].qs_tp1;
                }
            }
            //cvs[idx].vn_tp1 = flxn.v;
            //cvs[idx].slpn = flxn.slp;
            //cvs[idx].dfn = flxn.dflow;
            cvs[idx].qn_tp1 = flxn.q;
        }

        private static void calculateSFluxUsingArray(stCVAtt[] cvs, int idx, stGlobalValues gv, int isBCcell)
        {
            if (gv.nRows == 1) { return; }
            stFluxData flxs = new stFluxData();//S, y+
            if (cvs[idx].rowyary == (gv.nRows - 1) || cvs[idx].cvaryNum_atS == -1)
            {
                if (isBCcell == 1) { flxs = noFlx(); }
                else
                {
                    double slp_tm1 = 0;
                    if (cvs[idx].cvaryNum_atN >= 0)
                    {
                        //double slp = (cell.hp_tp1 - dm.cells[cx, ry - 1].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                        //double slp_tm1 = (cvs[idx].hp_t - cvs[cvs[idx].cvaryNum_atN].hp_t) / gv.dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                        double hn = cvs[cvs[idx].cvaryNum_atN].dp_t + cvs[cvs[idx].cvaryNum_atN].elez;
                        double hcur = cvs[idx].dp_t + cvs[idx].elez;
                        slp_tm1 = (hcur - hn) / gv.dx;
                    }
                    slp_tm1 = slp_tm1 - gv.domainOutBedSlope;
                    if (slp_tm1 <= (-1 * gv.slpMinLimitforFlow) && cvs[idx].dp_tp1 > gv.dMinLimitforWet)
                    {
                        //flxs = getFluxToDomainOut(cell, slp_tm1, cell.qs_t, cell.vs_t, gv.gravity, dt_sec);
                        //flxs = calculateMomentumEQ_DWEm_DeterministricUsingGPU(gv);
                        flxs = calculateMomentumEQ_DWEm_DeterministricUsingGPU(cvs[idx],cvs[idx].qs_t, gv.gravity, gv.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1,0);
                        //flxs.v = flxs.q / cell.dp_tp1;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
                        //flxs.dflow = cell.dp_tp1;
                    }
                    else { flxs = noFlx(); }
                }
            }
            else
            {
                if (cvs[idx].isSimulatingCell == -1)
                {
                    flxs = noFlx();
                }
                else
                {
                    //stCVAtt pcell = new stCVAtt();
                    //if (cvs[idx].rowyary > 0&& cvs[idx].cvaryNum_atN>-1) { pcell = cvs[cvs[idx].cvaryNum_atN]; }
                    //else { pcell = cvs[idx]; }
                    //flxs = getFluxToEastOrSouth(dx, cell, pcell, mDM.cells[cx, ry + 1], cVars.FlowDirection4.S, dt_sec);
                    //E = 1, S = 3, W = 5, N = 7, NONE = 0
                    //flxs = getFluxToEastOrSouthUsing1DArray(cvs[idx], pcell, cvs[cvs[idx].cvaryNum_atS], 3, gv);
                    flxs = getFluxToEastOrSouthUsing1DArray(cvs[idx], cvs[cvs[idx].cvaryNum_atS], 3, gv);
                }
            }
            cvs[idx].vs_tp1 = flxs.v;
            cvs[idx].dfs = flxs.dflow;
            cvs[idx].slps = flxs.slp;
            cvs[idx].qs_tp1 = flxs.q;
        }


        /// <summary>
        /// tp1 값을 이용해서 두셀간의 flux 계산
        /// </summary>
        /// <param name="dx"></param>
        /// <param name="curCell"></param>
        /// <param name="tarCell"></param>
        /// <returns></returns>

       // private static stFluxData getFluxToEastOrSouthUsing1DArray(stCVAtt curCell,
       //stCVAtt preCell, stCVAtt tarCell, int targetCellDir, stGlobalValues gv)
        [GpuManaged]
        private static stFluxData getFluxToEastOrSouthUsing1DArray(stCVAtt curCell,
                   stCVAtt tarCell, int targetCellDir, stGlobalValues gv)
        {
            double slp = 0;
            //double dht = (tarCell.elez+tarCell .dp_t)-(curCell.elez+curCell.dp_t); //+면 자신의 셀이, 대상 셀보다 낮다, q는 -, slp는 +.   -면 자신의 셀이, 대상 셀보다 높다, q는 +, slp는 - 
            double dhtp1 = tarCell.hp_tp1 - curCell.hp_tp1;
            //double dh_inACV = tarCell.hp_tp1 - preCell.hp_tp1;
            if (dhtp1 == 0) { return noFlx(); }
            if (dhtp1 > 0 && tarCell.dp_tp1 <= gv.dMinLimitforWet) { return noFlx(); }
            if (dhtp1 < 0 && curCell.dp_tp1 <= gv.dMinLimitforWet) { return noFlx(); }
            slp = dhtp1 / gv.dx;
            //slp = dht / gv.dx;
            if (DeviceFunction.Abs(slp) < gv.slpMinLimitforFlow || DeviceFunction.Abs(slp) == 0) { return noFlx(); }
            //double slp_tm1 = dht / gv.dx;
            double dflow = DeviceFunction.Max(curCell.hp_tp1, tarCell.hp_tp1) - DeviceFunction.Max(curCell.elez, tarCell.elez);
            // 최대 수심법
            //dflow = DeviceFunction.Max(curCell.hp_tp1, tarCell.hp_tp1); 
            //// 수심평균 법
            //double maxBedElev = DeviceFunction.Max(curCell.elez, tarCell.elez);
            //double d1 = curCell.hp_tp1 - maxBedElev;
            //if (d1 < 0) { d1 = 0; }
            //double d2 = tarCell.hp_tp1 - maxBedElev;
            //if (d2 < 0) { d2 = 0; }
            //dflow = (d1 + d2) / 2;
            //// 수심평균 법
            if (dflow <= 0) { return noFlx(); }
            double qt = 0; double qtp1 = 0;

            double q_ip1 = 0;
            double u_ip1 = 0;
            if (targetCellDir == 1)
            {
                qt = curCell.qe_t;
                qtp1 = curCell.qe_tp1; // qtp1
                u_ip1 = tarCell.ve_tp1; q_ip1 = tarCell.qe_tp1;
                //v_ip1 = tarCell.vw_t; q_ip1 = tarCell.qw_tp1;
                //vw_tp1 = currentCell.vw_tp1;
                //qAddedToCurCell = curCell.qw_tp1;
                //if (slp < 0) { qAddedToHigherCell = currentCell.qw_tp1; } 
                // 이조건은 물리적으로는 맞지만, 계산순서상 currentCell에서의 유량이 미리 더해져서 계산되므로, target셀의 tp1 시간에서의 유입량을 먼저 고려할 필요는 없다.
                //else { qAddedToHigherCell = targetCell.qe_tp1; }
            }
            else if (targetCellDir == 3)
            {
                qt = curCell.qs_t;
                qtp1 = curCell.qs_tp1;
                u_ip1 = tarCell.vs_tp1; q_ip1 = tarCell.qs_tp1;
                //v_ip1 = tarCell.vn_t; q_ip1 = tarCell.qn_tp1;
                //vw_tp1 = currentCell.vn_tp1;
                //qAddedToCurCell = curCell.qn_tp1;
                //if (slp < 0) { qAddedToHigherCell = currentCell.qn_tp1; }
                //else { qAddedToHigherCell = targetCell.qs_tp1; }
            }

            stFluxData flx = new stFluxData();
            if (gv.isDWE == 1)
            {
                //flx = calFluxUsingME_DWE_Implicit_UsingGPU(dhtp1, qt, qtp1, dflow, currentCell.rc, dx, dt_sec);
                //flx = calFluxUsingME_DWE_Implicit_UsingGPU(gv);
                flx = calculateMomentumEQ_DWE_DeterministricUsingGPU(qt, dflow, slp, gv.gravity, curCell.rc, gv.dx, gv.dt_sec, q_ip1, u_ip1);
            }
            else
            {
                //flx = calFluxUsingME_mDWE_Implicit(dhtp1, dht,
                //       qt, qtp1, dflow, currentCell.lc.roughnessCoeff, dx, dt_sec, currentCell.colxary, currentCell.rowyary);
                //flx.q = calculateMomentumEQ_DWEm_Deterministric(qt, utp1, dflow, slp, currentCell.rc, gv.gravity, dt_sec);
                //flx = calculateMomentumEQ_DWEm_DeterministricUsingGPU(gv);
                flx = calculateMomentumEQ_DWEm_DeterministricUsingGPU(curCell, qt, gv.gravity, gv.dt_sec, slp, curCell.rc, dflow, q_ip1);
                //flx.q = calculateMomentumEQ_DWEm_DeterministricUsingGPU(gv);
                //flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
                //flx.dflow = dflow;
            }

            if (gv.isAnalyticSolution == -1)
            {
                if (DeviceFunction.Abs(flx.q) > 0)
                {
                    flx = getFluxUsingSubCriticalConUsingGPU(flx, gv.gravity, gv.froudNCriteria);
                    flx = getFluxLimitBetweenTwoCellUsingDflowUsingGPU(flx, dflow, gv.dx, gv.dt_sec);
                    //flx = getFluxqUsingFourDirLimitUsingDepthCondition(currentCell, flx, dflow, dx, dt_sec); //이건 수렴이 잘 안된다.
                    //flx = getFluxUsingFourDirLimitUsingCellDepth(currentCell, targetCell, flx, dx, dt_sec);
                    //flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
                }
            }
            flx.slp = slp;
            return flx;
        }

        [GpuManaged]
        public static stFluxData calculateMomentumEQ_DWEm_DeterministricUsingGPU
            (stCVAtt curCell,double qt, double gravity, double dt_sec, double slp, double rc, double dflow, double qt_ip1)
        {
            stFluxData flx = new stFluxData();
            double qapp = qt ; //Math.Abs(qt);
                               //double slpapp = slp;// Math.Abs(slp);
                               //2019.1.2
                               // 관성이 없을 경우, slp가 + 면 q는 -, slp가 - 이면 q는 + 가 되어야 함.
                               // 이전 t에서 q 가  0 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 무조건 +.
                               // 이전 t에서 q 가  - 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 - 일수도 있고, + 일수도 있음. => 조건 처리 필요
                               // 이전 t에서 q 가 + 이면, slp가 + 일때 q는 - 일수도 있고, + 일수도 있음, slp가 - 일때는 q는 무조건 +. => 조건 처리 필요
                               // 아래 조건 넣으면니까.. 해가 좀 더 안정적이다.. 그러나 관성이 반영되지 않는다.
                               //if (slp > 0 && qapp > 0 || slp < 0 && qapp < 0) { qapp = 0; }
                               //if (slp > 0 && qapp > 0 || slp < 0 && qapp < 0) { qapp = 0.75 * qapp; }

            //double q = (qapp - (gravity * dflow * dt_sec * slp)) /
            //                           (1 + gravity * dt_sec * (rc * rc) * DeviceFunction.Sqrt((qapp * qapp + qt_ip1 * qt_ip1) / 2) / DeviceFunction.Pow(dflow, (double)7 / 3));
            double q = (qapp - (gravity * dflow * dt_sec * slp)) /
                           (1 + gravity * dt_sec * (rc * rc) * DeviceFunction.Abs(qapp) / DeviceFunction.Pow(dflow, (double)7 / 3));

            flx.q = q;
            flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            flx.dflow = dflow;
            flx.slp = slp;
            return flx; ;
        }

        [GpuManaged]
        private static stFluxData calculateMomentumEQ_DWE_DeterministricUsingGPU(double qt, double dflow,
 double slp, double gravity, double rc, double dx, double dt_sec, double q_ip1, double u_ip1 )
        {
            // 이거 잘 안된다. 반복법이 필요.. 2018.12.26.
            stFluxData flx = new stFluxData();
            double qapp = qt; //Math.Abs(qt);
            //2019.1.2 관성이 없을 경우에는 
            // slp가 + 면 q는 -, slp가 - 이면 q는 + 가 되어야 함.
            // 이전 t에서 q 가  0 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 무조건 +.
            // 이전 t에서 q 가  - 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 - 일수도 있고, + 일수도 있음. => 조건 처리 필요
            // 이전 t에서 q 가 + 이면, slp가 + 일때 q는 - 일수도 있고, + 일수도 있음, slp가 - 일때는 q는 무조건 +. => 조건 처리 필요
            // 아래 조건 넣으니까.. 좀 더 안정적이다.. 그러나, 관성이 반영되지 않는다.
            //if (slp > 0 && qapp > 0 || slp < 0 && qapp < 0) { qapp = 0; }
            //if (slp > 0 && qapp > 0 || slp < 0 && qapp < 0) { qapp = 0.95 * qapp; }
            double ut = qapp / dflow;
            double q = (qapp - (gravity * dflow * dt_sec * slp)) /
                                       (1 + ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * DeviceFunction.Abs(qapp) / DeviceFunction.Pow(dflow, (double)7 / 3));
            //double q = ((qapp - q_ip1 * u_ip1 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
            //                           (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * DeviceFunction.Abs(qapp) / DeviceFunction.Pow(dflow, (double)7 / 3)));
            //double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2) * (u_ip1+ut)/2 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
            //                           (1 - (u_ip1 + ut) / 2 * dt_sec / dx + gravity * dt_sec * (rc * rc) * Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2) / DeviceFunction.Pow(dflow, (double)7 / 3)));
            //double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2) * (u_ip1 + ut) / 2 * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
            //               (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2) / DeviceFunction.Pow(dflow, (double)7 / 3)));
            //double q = ((qapp - Math.Sqrt((q_ip1 * q_ip1 + qapp * qapp) / 2) * ut * dt_sec / dx - (gravity * dflow * dt_sec * slp)) /
            //   (1 - ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * qapp / DeviceFunction.Pow(dflow, (double)7 / 3)));

            flx.q = q;
            flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            flx.dflow = dflow;
            flx.slp = slp;
            return flx; ;
        }



        private static stFluxData getFluxUsingSubCriticalConUsingGPU(stFluxData inflx, double gravity, double froudNCriteria)
        {
            double v_wave = DeviceFunction.Sqrt(gravity * inflx.dflow);
            double fn = DeviceFunction.Abs(inflx.v) / v_wave;
            double qbak = inflx.q;
            if (fn > froudNCriteria)
            {
                double v = froudNCriteria * v_wave;
                inflx.v = v;
                if (qbak < 0) { inflx.v = -1 * v; }
                inflx.q = inflx.v * inflx.dflow;
            }
            return inflx;
        }


        private stFluxData getFluxUsingSubCriticalCon(stFluxData inflx)
        {
            double v_wave = Math.Sqrt(cGenEnv.gravity * inflx.dflow);
            double fn = Math.Abs(inflx.v) / v_wave;
            double qbak = inflx.q;
            if (fn > cHydro.froudeNCriteria)
            {
                double v = cHydro.froudeNCriteria * v_wave;
                inflx.v = v;
                if (qbak < 0) { inflx.v = -1 * v; }
                inflx.q = inflx.v * inflx.dflow;
            }
            return inflx;
        }

        private static stFluxData getFluxLimitBetweenTwoCellUsingDflowUsingGPU(stFluxData inflx, double dflow, double dx, double dt_sec)
        {
            double qmax = DeviceFunction.Abs(dflow) * dx / 2 / dt_sec; // 수위차의 1/2 이 아니라, 흐름 수심의 1/2이므로, 수위 역전 될 수 있다.
            double qbak = inflx.q;
            if (DeviceFunction.Abs(inflx.q) > qmax)
            {
                inflx.q = qmax;
                if (qbak < 0) { inflx.q = -1 * qmax; }
                inflx.v = inflx.q / inflx.dflow;
            }
            return inflx;
        }




        [GpuManaged]
        public static double getVonNeumanConditionValueUsingGPUFunction(stCVAtt cell)
        {
            double searchMIN = double.MaxValue;
            double curValue = 0;
            double rc = cell.rc;
            // e 값과 중복되므로, w는 계산하지 않는다.
            if (cell.dfe > 0)
            {
                curValue = 2 * rc * DeviceFunction.Sqrt(DeviceFunction.Abs(cell.slpe))
                                          / DeviceFunction.Pow(cell.dfe, (double)5 / 3);
                if (curValue < searchMIN) { searchMIN = curValue; }
            }
            // s 값과 중복되므로, n는 계산하지 않는다.
            if (cell.dfs > 0)
            {
                curValue = 2 * rc * DeviceFunction.Sqrt(DeviceFunction.Abs(cell.slps))
                                        / DeviceFunction.Pow(cell.dfs, (double)5 / 3);
                if (curValue < searchMIN) { searchMIN = curValue; }
            }
            return searchMIN;
        }

        [GpuManaged]
        public static void innerLoopForGPUonly_trial(stCVAtt[] cvs, stGlobalValues[] gv) // 순서가 있는 루프는 안된다.
        {

            ////for (int iter = 0; iter < 100000; iter++)
            //while ((gv[0].tnow_sec) < gv[0].tsecToExitInnerLoop)
            //{
            //    var start = blockIdx.x * blockDim.x + threadIdx.x;//
            //    var stride = gridDim.x * blockDim.x;
            //    for (int igs = 0; igs < gv[0].iGSmax; igs++) // 반복계산은 우선 100번 정도..
            //    {
            //        for (int i = start; i < cvs.Length; i += stride)
            //            //for (int i = 0; i < cvs.Length; i++)
            //        {
            //            if (igs == 0) { cSimulationSetting.initializeAcellUsingArray_static_trial(cvs, i, gv[0]); }
            //            if (cvs[i].isSimulatingCell == 1)
            //            {
            //                cSolver.calculateContinuityEqUsingNRUsing1DArray(cvs, i, gv[0]);
            //                if (cvs[i].dp_tp1 > gv[0].dMinLimitforWet)
            //                { cSimulationSetting.setSimulatingCellUsingArray(cvs, i); }
            //            }
            //        }
            //    }
            //    double dflowmaxInThisStep = -1;
            //    //double dmaxInThisStep = -1;
            //    double vmaxInThisStep = -1;
            //    double VNConMinInThisStep = -1;
            //    double maxResd = -1;
            //    int maxResdAryNum = -1;
            //    for (int i = 0; i < cvs.Length; i++)
            //    {
            //        if (cvs[i].isSimulatingCell == 1)
            //        {
            //            //cSolver.getMaxValuesFrom4DirectionUsingArray(cvs, i, gv[0].dx);
            //            if (dflowmaxInThisStep < cvs[i].dp_tp1) { dflowmaxInThisStep = cvs[i].dp_tp1; }
            //            //if (dmaxInThisStep < cvs[i].dp_tp1) { dmaxInThisStep = cvs[i].dp_tp1; }
            //            //if (vmaxInThisStep < cvs[i].vmax) { vmaxInThisStep = cvs[i].vmax; }
            //            if (gv[0].isApplyVNC == 1)
            //            { VNConMinInThisStep = DeviceFunction.Max(VNConMinInThisStep, cSolver.getVonNeumanConditionValueUsingGPU(cvs[i])); }
            //            if (maxResd < cvs[i].resd)
            //            {
            //                maxResd = cvs[i].resd;
            //                maxResdAryNum = i;
            //            }

            //            //cSolver.getMaxValuesFrom4DirectionUsingArray(cvs, i, gv[0].dx);
            //            //if (dflowmaxInThisStep < cvs[i].dflowmax) { dflowmaxInThisStep = cvs[i].dflowmax; }
            //            //if (dmaxInThisStep < cvs[i].dp_tp1) { dmaxInThisStep = cvs[i].dp_tp1; }
            //            //if (vmaxInThisStep < cvs[i].vmax) { vmaxInThisStep = cvs[i].vmax; }
            //            //if (gv[0].isApplyVNC == 1)
            //            //{ VNConMinInThisStep = DeviceFunction.Max(VNConMinInThisStep, cSolver.getVonNeumanConditionValueUsingGPU(cvs[i])); }
            //            //if (maxResd < cvs[i].resd)
            //            //{
            //            //    maxResd = cvs[i].resd;
            //            //    maxResdAryNum = i;
            //            //}
            //        }
            //    }
            //    gv[0].dflowmaxInThisStep = dflowmaxInThisStep;
            //    gv[0].vmaxInThisStep = vmaxInThisStep;
            //    gv[0].VNConMinInThisStep = VNConMinInThisStep;
            //    gv[0].maxResd = maxResd;
            //    gv[0].maxResdAryNum = maxResdAryNum;

            //    gv[0].tnow_sec = gv[0].tnow_sec + gv[0].dt_sec; // 여기서 전진
            //    gv[0].dt_sec_bak = gv[0].dt_sec;
            //    if (gv[0].isFixedDT == -1)
            //    {
            //        cHydro.getDTsecUsingConstraintsInGPU(gv);
            //    }
            //    //if (gv[0].tnow_sec > gv[0].tsecToExitInnerLoop) { break; }
            //}
        }
    }
}





