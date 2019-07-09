using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Alea;

namespace G2DCore
{
    class cSolverBAK
    {
        public void RunSolver(double dt_sec)
        {
            ////mDM = dm;
            ////mdx = dm.dx;
            //mdt_sec = dt_sec;
            ////mCVCount1P4 = dm.cvCount / 4 ;
            ////mCVCount2P4 = dm.cvCount / 2;
            ////mCVCount3P4 = dm.cvCount / 4*3;
            ////mTotCountCV = dm.cvCount;

            //int igs = 0;
            ////int iNRmax = 0;
            ////// 삭제대상
            //////=======================
            ////string summary_h = "";
            ////string tmpFPNSum_h = System.IO.Path.Combine(cProject.Current.prjFilePath, "00_iteration_h.out");
            ////if (System.IO.File.Exists(tmpFPNSum_h) == true) { System.IO.File.Delete(tmpFPNSum_h); }

            ////cProject.Current.hvalues_Acell_willbeDeleted = String.Format(cGenEnv.tnow_sec.ToString("#.#0") + "_({0}, {1})", aCellcolX, aCellrowY) + "\t";
            //////=======================
            ////cThisProcess.bAllConvergedInThisTimeStep = true;
            //var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            //cThisProcess.maxNR_inME = 0;
            ////mNoflx = new stFluxData(); //setNoFlux();
            ////cThisProcess.maxResd = 0;
            //for (igs = 0; igs < cGenEnv.iGSmax; igs++) // 반복계산은 우선 100번 정도..
            //{
            //    // 삭제대상
            //    //=======================
            //    //summary_h = igs.ToString() + "\t";
            //    //=======================
            //    migs = igs;
            //    cThisProcess.bAllConvergedInThisGSiteration = true;
            //    cGenEnv.iNR = 0;
            //    cGenEnv.dflowmaxInThisStep = -9999;
            //    cGenEnv.dmaxInThisStep = -9999;
            //    cGenEnv.vmaxInThisStep = -9999;
            //    cGenEnv.VNConMinInThisStep = 9999;
            //    notconverged = new List<string>();
            //    if (cGenEnv.isparallel == true)
            //    {
            //        ////Threading test를 위해 주석처리. 2018.7.6
            //        //int mdp = cGenEnv.maxDegreeParallelism;
            //        ////if (mdp == -1 || mdp >= Environment.ProcessorCount)
            //        ////{
            //        ////    mdp = Environment.ProcessorCount / 2;
            //        ////}
            //        //mPoptions = new ParallelOptions { MaxDegreeOfParallelism = mdp };
            //        //int taskCount = 2;// mdp;
            //        //Task[] tasks = new Task[taskCount];
            //        //for (int iT = 0; iT < taskCount; iT++)
            //        //{
            //        //    int sid = mTotCountCV / taskCount * (iT);
            //        //    int eid = mTotCountCV / taskCount * (iT + 1);
            //        //    tasks[iT] = Task.Factory.StartNew((Object obj) =>
            //        //    {
            //        //        var data = (dynamic)obj;
            //        //        int sidr = data.sid;
            //        //        int eidr = data.eid;
            //        //        Parallel.ForEach(Partitioner.Create(0, dm.cvCount), mPoptions,
            //        //            (range) =>
            //        //            {
            //        //                for (int i = range.Item1; i < range.Item2; i++)
            //        //                {
            //        //                    if (dm.cvs[i].isSimulatingCell == true)
            //        //                    {
            //        //                        calculateContinuityEqUsingNR(dm.cvs[i], mdx, dt_sec, igs);
            //        //                        if (cGenEnv.movingDomain == true && dm.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
            //        //                        { cDomain.setSimulatingCell(dm, dm.cvs[i]); }

            //        //                    }
            //        //                }
            //        //            });
            //        //        //Parallel.ForEach(Partitioner.Create(sidr, eidr), mPoptions,
            //        //        //    (range) =>
            //        //        //    {
            //        //        //        for (int i = range.Item1; i < range.Item2; i++)
            //        //        //        {
            //        //        //            int ry = dm.cvs[i].rowyary;
            //        //        //            int cx = dm.cvs[i].colxary;
            //        //        //            if (dm.cvs[i].isSimulatingCell == true)
            //        //        //            {
            //        //        //                calculateContinuityEqUsingNR(ry, cx, mdx, dt_sec, igs);
            //        //        //                if (cGenEnv.movingDomain == true && dm.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
            //        //        //                { cDomain.setSimulatingCell(dm, dm.cvs[i]); }

            //        //        //            }
            //        //        //        }
            //        //        //    });
            //        //    }, new { iT = iT, sid = sid, eid = eid });
            //        //}
            //        //Task.WaitAll(tasks);

            //        //var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            //        #region "parallel for each"
            //        //이렇게 하면, dem 값이 있는 셀만 접근하므로, 속도 향상 된다. 또한 유역 형상에 상관없이 같은 성능 나온다.
            //        //0은 포함, dm.cvCount는 제외됨 
            //        Parallel.ForEach(Partitioner.Create(0, mDM.cvCount), options,
            //            (range) =>
            //            {
            //                for (int i = range.Item1; i < range.Item2; i++)
            //                {
            //                    if (mDM.mCVsAry[i].isSimulatingCell == 1)
            //                    {
            //                        //bool dryStart = true;
            //                        //if (mDM.mCVsAry[i].dp_tp1 > cGenEnv.dMinLimitforWet) { dryStart = false; }
            //                        calculateContinuityEqUsingNR(mDM.mCVsAry[i], mDM.dx, dt_sec, igs);
            //                        //if (dryStart == true && mDM.mCVsAry[i].dp_tp1 > cGenEnv.dMinLimitforWet)
            //                        if (mDM.mCVsAry[i].dp_tp1 > cGenEnv.dMinLimitforWet)
            //                        { cDomain.setSimulatingCell(mDM, mDM.mCVsAry[i]); }
            //                    }
            //                }
            //            });
            //        #endregion

            //        //#region "parallel for"
            //        ////// 이렇게 하면, 일단 접근해서 유효한 셀인지를 검토함. 또한 유역 형상에 따라서 분기를 해야 효율이 좀더 좋아진다.
            //        //////0은 포함, dm.nRows은 제외됨 
            //        ////Parallel.For(0, dm.nRows, options, delegate (int ry)
            //        ////{
            //        ////    for (int cx = 0; cx < dm.nCols; cx++)
            //        ////    {
            //        ////        if (dm.cells[cx, ry].isSimulated == true)
            //        ////        {
            //        ////            calculateContinuityEqUsingNR(dm, ry, cx, dx, dt_sec, igs);
            //        ////                //calculateContinuityEqUsingDeterministric(dm, ry, cx, dx, dt_sec, igs);
            //        ////            }
            //        ////    }
            //        ////});
            //        //#endregion
            //    }
            //    else
            //    {
            //        for (int ry = 0; ry < nRows; ry++)
            //        {
            //            for (int cx = 0; cx < nCols; cx++)
            //            {
            //                if (mDM.mCells[cx, ry].isSimulatingCell == 1)
            //                {
            //                    //bool dryStart = true;
            //                    //if (mDM.mCells[cx, ry].dp_tp1 > cGenEnv.dMinLimitforWet) { dryStart = false; }
            //                    //calculateContinuityEqUsingDeterministric(dm, ry, cx, dx, dt_sec, igs);
            //                    calculateContinuityEqUsingNR(mDM.mCells[cx, ry], mDM.dx, dt_sec, igs);
            //                    //if (cGenEnv.movingDomain == true && mDM.cells[cx, ry].dp_tp1 > cGenEnv.dMinLimitforWet)
            //                    if (mDM.mCells[cx, ry].dp_tp1 > cGenEnv.dMinLimitforWet)
            //                    { cDomain.setSimulatingCell(mDM, mDM.mCells[cx, ry]); }

            //                    ////======================= 이건 x 방향 한 줄만 있는 경우 변수 저장
            //                    //summary_h = summary_h + dm.cells[cx, ry].hp_tp1.ToString() + "\t";
            //                    ////=======================

            //                    ////======================= 이건 특정 한셀만 변수 저장
            //                    //if (cx == 1 && ry == 0 && cGenEnv.tnow_sec >= watchTime_sec)
            //                    //    //if (cx == aCellcolX && ry == aCellrowY && cGenEnv.tnow_sec >= 10)
            //                    //    //{
            //                    //        //summary_h = summary_h + dm.cells[cx, ry].hp_tp1.ToString() + "\t";
            //                    //        cProject.Current.hvalues_Acell_willbeDeleted = cProject.Current.hvalues_Acell_willbeDeleted + dm.cells[cx, ry].dp_tp1.ToString() + "\t\n";
            //                    //    //}
            //                    ////=======================
            //                }
            //                //// 삭제대상 이건 특정 열만 y 방향으로 출력할 경우의 변수 저장
            //                ////=======================
            //                //if (cx == 10)
            //                //{
            //                //    summary_h = summary_h + dm.cells[cx, ry].hp_tp1.ToString() + "\t";
            //                //}
            //                ////=======================
            //            }
            //            //// 삭제대상. 이건 y 방향 한줄만 있는 경우, 반복계산 출력
            //            ////=======================
            //            //summary_h = summary_h + "\r\n";
            //            //System.IO.File.AppendAllText(tmpSum_h, summary_h);
            //            ////=======================
            //        }
            //        //  삭제대상.이건 x 방향 한줄만 있는 경우, 혹은 특정셀, 특정시간에서의 gs 안에서의 반복계산 출력
            //        ////=======================
            //        //summary_h = summary_h + "\r\n";
            //        //cProject.Current.hvalues_Acell_willbeDeleted = cProject.Current.hvalues_Acell_willbeDeleted + "\r\n";
            //        //if (cGenEnv.tnow_sec > 0.4236 && cGenEnv.tnow_sec > 0.6)
            //        //{
            //        //    System.IO.File.AppendAllText(tmpFPNSum_h, summary_h);
            //        //}
            //        ////  =======================
            //    }
            //    cGenEnv.iGS = igs;
            //    if (cThisProcess.bAllConvergedInThisGSiteration == true)
            //    {
            //        //cGenEnv.writelog(string.Format("Time : {0}sec. GS iteration({1}) was converged. dtsec : {2}.", cGenEnv.tnow_sec, igs, dt_sec), true);
            //        //for (int i = 0; i < notconverged.Count; i++) { cGenEnv.writelog(notconverged[i], true); }
            //        break;
            //    }
            //    else
            //    {
            //        //cGenEnv.writelog(string.Format("Time : {0}sec. GS iteration({1}) was not converged. dtsec : {2}.", cGenEnv.tnow_sec, igs, dt_sec), true);
            //        //string notCovergedCell = "";
            //        //for (int i = 0; i < dm.nCols; i++)
            //        //{
            //        //    notCovergedCell = notCovergedCell + dm.cells[i, 0].converged.ToString() + "\t";
            //        //}
            //        //cGenEnv.writelog(notCovergedCell, true);

            //    }
            //여기까지 gs iteration

            ////여기서부터는 이번 step에서의 결과 정리

            //if (cThisProcess.bAllConvergedInThisGSiteration == false)
            //{
            //    cGenEnv.writelog(string.Format("Time : {0}sec. GS iteration({1}) was not converged. dtsec : {2}.", cGenEnv.tnow_sec, igs, dt_sec), true);
            //    if (cGenEnv.vdtest == true)
            //    {
            //        for (int i = 0; i < notconverged.Count; i++) { cGenEnv.writelog(notconverged[i], true); }
            //    }
            //}
            //else
            //{
            //    cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration({1}) was converged. Con. eq. NR max iteration: {2}.",
            //                             cGenEnv.tnow_sec, igs, iNRmax), true);
            //}
            ////  삭제대상.
            //////=======================
            //if (cGenEnv.tnow_sec >= watchTime_sec)
            //{
            //    if (cGenEnv.vdtest == true)
            //    {
            //        cProject.Current.hvalues_Acell_willbeDeleted = cProject.Current.hvalues_Acell_willbeDeleted + "\r\n";
            //        System.IO.File.AppendAllText(cProject.Current.fpnInterAcell_willbeDeleted, cProject.Current.hvalues_Acell_willbeDeleted);
            //    }
            //}
            ////  =======================
            //UpdateValuesInThisStepResults();

            //if (cGenEnv.isparallel == true)
            //{
            //    UpdateValuesInThisStep();
            //}
            //else
            //{
            //    for (int ry = 0; ry < nRows; ry++)
            //    {
            //        for (int cx = 0; cx < nCols; cx++)
            //        {
            //            if (mDM.cells[cx, ry].isSimulatingCell == true)
            //            {
            //                cCVAttribute cell = mDM.cells[cx, ry];
            //                getMaxValuesFrom4Direction(cell);
            //                double vnCon = getVonNeumanConditionValue(cell);
            //                if (cGenEnv.dflowmaxInThisStep < cell.dflowmax) { cGenEnv.dflowmaxInThisStep = cell.dflowmax; }
            //                if (cGenEnv.dmaxInThisStep < cell.dp_tp1) { cGenEnv.dmaxInThisStep = cell.dp_tp1; }
            //                if (cGenEnv.vmaxInThisStep < cell.vmax) { cGenEnv.vmaxInThisStep = cell.vmax; }
            //                if (cGenEnv.VNConMinInThisStep > vnCon) { cGenEnv.VNConMinInThisStep = vnCon; }
            //                if (cThisProcess.maxResd < cell.resd)
            //                {
            //                    cThisProcess.maxResd = cell.resd;
            //                    cThisProcess.maxResdCell = "(" + cell.colxary.ToString() + "," + cell.rowyary.ToString() + ")";
            //                }
            //            }
            //        }
            //    }
            //}

            //if (cThisProcess.bAllConvergedInThisGSiteration == true)
            //{
            //    if (cGenEnv.bwritelog_process == true)
            //    {
            //        cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
            //                              cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
            //    }
            //}
            //else
            //{
            //    if (cGenEnv.bwritelog_process == true)
            //    {
            //        cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was not converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
            //                            cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
            //    }
            //}
        }

        private bool calculateContinuityEqUsingNR(stCVAtt[] cvs, int idx, stGlobalValues[] gv, int isBCcell, int bctype, double bcDepth)
        {
            ////double sourceTerm = (cell.sourceAlltoRoute_tp1_dt_m + cell.sourceAlltoRoute_t_dt_m) / 2; //이건 Crank-Nicolson 방법.  dt는 이미 곱해서 있다..
            //double c1_CR = gv[0].dt_sec / gv[0].dx / 2; //이건 CR
            ////double sourceTerm = cell.sourceAll_tp1_dt_m; //dt는 이미 곱해서 있다... 이건 음해법
            //double c1_IM = gv[0].dt_sec / gv[0].dx;//이건 음해법
            //double dp_old = cvs[idx].dp_tp1;
            //double dn = cvs[idx].dp_tp1;
            ////int cxary = cvs.colxary;
            ////int ryary = cvs.rowyary;
            ////int isNRconverged = 1;
            ////cell.converged = 1;
            //int inr = 0;
            //for (inr = 0; inr < cGenEnv.iNRmax_forCE; inr++)
            //{
            //    //calculateWFluxUsingArray(cvs, idx, gv[0], isBCcell);
            //    //calculateEFluxUsingArray(cvs, idx, gv[0], isBCcell);
            //    //calculateNFluxUsingArray(cvs, idx, gv[0], isBCcell);
            //    //calculateSFluxUsingArray(cvs, idx, gv[0], isBCcell);
            //    //calculateWFlux(cxary, ryary, dx, dt_sec);
            //    //calculateEFlux(cxary, ryary, dx, dt_sec);
            //    //calculateNFlux(cxary, ryary, dx, dt_sec);
            //    //calculateSFlux(cxary, ryary, dx, dt_sec);
            //    // 현재 셀의 수위가 올라가려면  -> qe-, qw+, qs-, qn+
            //    double dnp1 = 0;
            //    //NR
            //    //double fdn = dn - cell.dp_t + (cell.qe_tp1 + cell.qe_t - cell.qw_tp1 - cell.qw_t + cell.qs_tp1 + cell.qs_t - cell.qn_tp1 - cell.qn_t) * c1_CR;// - sourceTerm; // 이건 CR
            //    double fn = dn - cvs[idx].dp_t + (cvs[idx].qe_tp1 - cvs[idx].qw_tp1 + cvs[idx].qs_tp1 - cvs[idx].qn_tp1) * c1_IM;//- sourceTerm; //이건 음해법
            //    //double fn = dn - dp_old + (cell.qe_tp1 - cell.qw_tp1 + cell.qs_tp1 - cell.qn_tp1) * c1_IM;//- sourceTerm; //이건 음해법
            //    double eElem = Math.Pow(cvs[idx].dfe, (double)2 / 3) * Math.Sqrt(Math.Abs(cvs[idx].slpe)) / cvs[idx].rc;
            //    double sElem = Math.Pow(cvs[idx].dfs, (double)2 / 3) * Math.Sqrt(Math.Abs(cvs[idx].slps)) / cvs[idx].rc;
            //    //double dfd = 1 + (eElem + sElem) * (5.0 / 3) * c1_CR; // LISFLood 하고 비교할때는 power에서 double을 빼야 // 이건 CR
            //    double dfn = 1 + (eElem + sElem) * (5.0 / 3) * c1_IM; // LISFLood 하고 비교할때는 power에서 double을 빼야 // 이건 음해법
            //    if (dfn == 0) { break; }
            //    dnp1 = dn - fn / dfn;
            //    //dnp1 = cell.dp_tp1 - (cell.qe_tp1 - cell.qw_tp1 + cell.qs_tp1 - cell.qn_tp1) * dt_sec / dx;
            //    //if (cell.isBCcell == true && cell.cvConditionDataType == cVars.ConditionDataType.Depth)
            //    if (isBCcell == 1 && bctype == 2)
            //    {
            //        dnp1 = bcDepth;// cell.bcDepth_dt_m_tp1;
            //    }
            //    if (dnp1 < 0) { dnp1 = 0; }
            //    double resd = dnp1 - dn;
            //    cvs[idx].dp_tp1 = dnp1;
            //    cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
            //    if (gv[0].iNR < inr) { gv[0].iNR = inr; }
            //    if (Math.Abs(resd) < cGenEnv.convergenceConditionh)
            //    {
            //        //isNRconverged = 1;
            //        break;
            //    }
            //    dn = dnp1;
            //    //isNRconverged = -1;
            //}
            //cvs[idx].resd = Math.Abs(cvs[idx].dp_tp1 - dp_old);
            //if (cvs[idx].resd <= cGenEnv.convergenceConditionh)
            //{
            //    //cell.converged = 1;
            //}
            //else
            //{
            //    cThisProcess.bAllConvergedInThisGSiteration = false;
            //    //cell.converged = -1;
            //    ////삭제 대상
            //    //if (cGenEnv.tnow_sec >= watchTime_sec)
            //    //{
            //    //    //cGenEnv.writelog(string.Format("Time : {0}sec. Continuity eq. was not converged in GS. NR is {5}. cell array ({1}, {2}), GS iteration : {3}, dtsec : {4}.",
            //    //    //        cGenEnv.tnow_sec, cxary, ryary, igs, dt_sec, isNRconverged), true);
            //    //}
            //    //if (cGenEnv.vdtest == true)
            //    //{
            //    //    notconverged.Add(string.Format("({0}, {1})", cxary, ryary));
            //    //}
            //}
            ////if (cGenEnv.vdtest == true)
            ////{
            ////    //if (notconverged.Contains(string.Format("({0}, {1})", cxary, ryary)))
            ////    //{
            ////    //    notconverged.Remove(string.Format("({0}, {1})", cxary, ryary));
            ////    //}
            ////    //////////////////////////////
            ////}
            return true;
        }

        private static stFluxData calculateMomentumEQ_DWE_Deterministric(double qt, double ut, double dflow,
         double slp, double mN, double dx, double dt_sec)
        {
            stFluxData flx = new stFluxData();
            // 분모의 qt로 절대값을 쓴다.. 제곱에서 파생된 것이므로..
            //slp = -1 * slp;
            double qapp = qt; //Math.Abs(qt);
            //double slpapp = slp;// Math.Abs(slp);
            double q = 0;
            q = ((qapp - (cGenEnv.gravity * dflow * dt_sec * slp)) /
                                       (1 + ut * dt_sec / dx + cGenEnv.gravity * dt_sec * (mN * mN) * Math.Abs(qapp) / Math.Pow(dflow, (double)7 / 3)));
            flx.q = q;
            flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            flx.dflow = dflow;
            return flx; ;
        }

        public double calculateMomentumEQ_DWEm_Deterministric(double qt, double dflow,
    double slp, double mN, double gravity, double dt_sec)
        {
            //stFluxData flx = new stFluxData();
            // 분모의 qt로 절대값을 쓴다.. 제곱에서 파생된 것이므로..
            //slp = -1 * slp;
            double qapp = qt; //Math.Abs(qt);
            //double slpapp = slp;// Math.Abs(slp);
            double q = 0;
            q = (qapp - (gravity * dflow * dt_sec * slp)) /
                                       (1 + gravity * dt_sec * (mN * mN) * Math.Abs(qapp) / Math.Pow(dflow, (double)7 / 3));
            //flx.q = q;
            //flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            //flx.dflow = dflow;
            //return flx; ;
            return q;
        }



        private static stFluxData getFluxqUsingFourDirLimitUsingDepthCondition
                          (stCVAtt curCell, stFluxData inflx, double dflow, double dx, double dt_sec)
        {
            //todo. 여기서 수심을 기준으로 유량 조건을 검토하고 있는데.. 이것을 유량을 기준으로 수심 조건을 검토하는 것으로 수정 필요. 2018.4.24
            double dLimit = curCell.dp_tp1 / 2;
            //double q = 0;
            stFluxData flx = new stFluxData();
            if (dflow > dLimit)
            {
                flx.q = dLimit * dx / 2 / dt_sec;
                if (inflx.q < 0) { flx.q = 0 - flx.q; }
                flx.v = flx.q / dLimit;
            }
            return flx;
        }


        private bool calculateContinuityEqUsingDeterministric(cDomain dm, int ryary, int cxary, double dx, double dt_sec, int igs)
        {
            //stCVAtt cell = dm.mCells[cxary, ryary]; //cell 변수의 값을 바꾸면, dm.cells에서의 값도 바뀐다.
            //double hp_old = cell.hp_tp1;
            //double dp_old = cell.dp_tp1;
            ////cell.converged = -1;
            //calculateWFlux(cxary, ryary,  dx, dt_sec);
            //calculateEFlux(cxary, ryary,  dx, dt_sec);
            //calculateNFlux(cxary, ryary,  dx, dt_sec);
            //calculateSFlux(cxary, ryary,  dx, dt_sec);
            //    if (cell.isBCcell == 1 && cell.cvConditionDataType == 2)
            //    {
            //    cell.dp_tp1 = cell.bcDepth_dt_m_tp1;
            //}
            //else
            //{
            //    cell.dp_tp1 = dp_old - (cell.qe_tp1 - cell.qw_tp1 + cell.qs_tp1 - cell.qn_tp1) * dt_sec / dx;
            //}
            //if (cGenEnv.tnow_sec > 50 && cxary >= 38 && cxary < 42)
            //if (cell.dp_tp1 < 0) { cell.dp_tp1 = 0; }
            //cell.hp_tp1 = cell.elez + cell.dp_tp1;
            //cell.resd = cell.dp_tp1;
            //if (dp_old > 0)
            //{
            //    cell.resd = cell.dp_tp1 - dp_old;
            //}
            //if (Math.Abs(cell.resd) <= cGenEnv.convergenceConditionh)
            //{
            //    //cell.converged = 1;
            //}
            //else
            //{
            //    cThisProcess.bAllConvergedInThisGSiteration = false;
            //    //cell.converged = -1;
            //}
            return true;
        }



        private void calculateSFlux(int cx, int ry, double dx, double dt_sec)
        {
            //if (mDM.nRows == 1) { return; }
            //stCVAtt cell = mDM.mCells[cx, ry];
            //stFluxData flxs = new stFluxData();//S, y+
            //if (ry == (mDM.nRows - 1) || mDM.mCells[cx, ry + 1].isInDomain == -1)
            //{
            //    if (cell.isBCcell == 1) { flxs = new stFluxData(); }
            //    else
            //    {
            //        //double slp = (cell.hp_tp1 - dm.cells[cx, ry - 1].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
            //        double slp_tm1 = (cell.hp_t - mDM.mCells[cx, ry - 1].hp_t) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
            //        slp_tm1 = slp_tm1 - cHydro.domainOutBedSlope;
            //        if (slp_tm1 <= (-1 * cGenEnv.slpMinLimitforFlow) && cell.dp_tp1 > cGenEnv.dMinLimitforWet)
            //        {
            //            flxs = getFluxToDomainOut(cell, slp_tm1, cell.qs_t, cGenEnv.gravity, dt_sec);
            //        }
            //        else { flxs = new stFluxData(); }
            //    }
            //}
            //else
            //{
            //    //if (cell.dp_tp1 <= cGenEnv.dMinLimitforWet && dm.cells[cx, ry + 1].dp_tp1 <= cGenEnv.dMinLimitforWet)
            //    if (cell.isSimulatingCell == -1)
            //    {
            //        flxs = new stFluxData();
            //    }
            //    else
            //    {
            //        stCVAtt pcell = new stCVAtt();
            //        if (ry > 0) { pcell = mDM.mCells[cx, ry - 1]; }
            //        else { pcell = cell; }
            //        //flxs = getFluxToEastOrSouth(dx, cell, pcell, mDM.cells[cx, ry + 1], cVars.FlowDirection4.S, dt_sec);
            //        //E = 1, S = 3, W = 5, N = 7, NONE = 0
            //        flxs = getFluxToEastOrSouth(dx, cell, pcell, mDM.mCells[cx, ry + 1], 3, dt_sec);
            //    }
            //}
            //cell.vs_tp1 = flxs.v;
            //cell.dfs = flxs.dflow;
            //cell.slps = flxs.slp;
            //cell.qs_tp1 = flxs.q;
        }

        private void calculateNFlux(int cx, int ry, double dx, double dt_sec)
        {
            //if (mDM.nRows == 1) { return; }
            //stCVAtt cell = mDM.mCells[cx, ry];
            //stFluxData flxn = new stFluxData();  //N, y-
            //if (ry == 0 || mDM.mCells[cx, ry - 1].isInDomain == -1)
            //{
            //    if (cell.isBCcell == 1) { flxn = new stFluxData(); }
            //    else
            //    {// n측 최 경계에서는 n 방향으로 자유수면 flx 있다.
            //        //double slp = (dm.cells[cx, ry + 1].hp_tp1 - cell.hp_tp1) / dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
            //        double slp_tm1 = (mDM.mCells[cx, ry + 1].hp_t - cell.hp_t) / dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
            //        slp_tm1 = slp_tm1 + cHydro.domainOutBedSlope;
            //        if (slp_tm1 >= cGenEnv.slpMinLimitforFlow && cell.dp_tp1 > cGenEnv.dMinLimitforWet)
            //        {
            //            flxn = getFluxToDomainOut(cell, slp_tm1, cell.qn_t, cGenEnv.gravity, dt_sec);
            //        }
            //        else { flxn = new stFluxData(); }
            //    }
            //}
            //else
            //{
            //    //if (cell.dp_tp1 <= cGenEnv.dMinLimitforWet && dm.cells[cx, ry-1].dp_tp1 <= cGenEnv.dMinLimitforWet)
            //    if (cell.isSimulatingCell == -1)
            //    {
            //        flxn = new stFluxData();
            //    }
            //    else
            //    {
            //        flxn.v = mDM.mCells[cx, ry - 1].vs_tp1;
            //        flxn.slp = mDM.mCells[cx, ry - 1].slps;
            //        flxn.dflow = mDM.mCells[cx, ry - 1].dfs;
            //        flxn.q = mDM.mCells[cx, ry - 1].qs_tp1;
            //    }
            //}
            //cell.vn_tp1 = flxn.v;
            //cell.slpn = flxn.slp;
            //cell.dfn = flxn.dflow;
            //cell.qn_tp1 = flxn.q;
        }



        private void calculateEFlux(int cx, int ry, double dx, double dt_sec)
        {
            //if (mDM.nCols == 1) { return; }
            //stCVAtt cell = mDM.mCells[cx, ry];
            //stFluxData flxe = new stFluxData();    //E,  x+
            //if (cx == (mDM.nCols - 1) || mDM.mCells[cx + 1, ry].isInDomain == -1)
            //{
            //    if (cell.isBCcell == 1) { flxe = new stFluxData(); }
            //    else
            //    {
            //        //double slp = (cell.hp_tp1 - dm.cells[cx - 1, ry].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
            //        double slp_tm1 = (cell.hp_t - mDM.mCells[cx - 1, ry].hp_t) / dx;
            //        slp_tm1 = slp_tm1 - cHydro.domainOutBedSlope;
            //        if (slp_tm1 <= (-1 * cGenEnv.slpMinLimitforFlow) && cell.dp_tp1 > cGenEnv.dMinLimitforWet)
            //        {
            //            flxe = getFluxToDomainOut(cell, slp_tm1, cell.qe_t, cGenEnv.gravity, dt_sec);
            //        }
            //        else { flxe = new stFluxData(); }
            //    }
            //}
            //else
            //{
            //    //if (cell.dp_tp1 <= cGenEnv.dMinLimitforWet && dm.cells[cx + 1, ry].dp_tp1 <= cGenEnv.dMinLimitforWet)
            //    if (cell.isSimulatingCell == -1)
            //    {
            //        flxe = new stFluxData();
            //    }
            //    else
            //    {
            //        stCVAtt pcell = new stCVAtt();
            //        if (cx > 0) { pcell = mDM.mCells[cx - 1, ry]; }
            //        else { pcell = cell; }
            //        //E = 1, S = 3, W = 5, N = 7, NONE = 0
            //        //flxe = getFluxToEastOrSouth(dx, cell, pcell, mDM.cells[cx + 1, ry], cVars.FlowDirection4.E, dt_sec);
            //        flxe = getFluxToEastOrSouth(dx, cell, pcell, mDM.mCells[cx + 1, ry], 1, dt_sec);
            //    }
            //}
            //cell.ve_tp1 = flxe.v;
            //cell.dfe = flxe.dflow;
            //cell.slpe = flxe.slp;
            //cell.qe_tp1 = flxe.q;
        }


        private void calculateWFlux(int cx, int ry, double dx, double dt_sec)
        {
            //if (mDM.nCols == 1) { return; }
            //stCVAtt cell = mDM.mCells[cx, ry];
            //stFluxData flxw = new stFluxData(); //W, x-
            //if (cx == 0 ||mDM.mCells[cx - 1, ry].isInDomain == -1)
            //{
            //    if (cell.isBCcell == 1)
            //    {
            //        flxw = new stFluxData(); // w측 최 경계에서는 w 방향으로 flx 없다.
            //    }
            //    else
            //    {// w측 최 경계에서는 w 방향으로 자유수면 flx 있다.
            //        //double slp = (cell.hp_tp1 - dm.cells[cx + 1, ry].hp_tp1) / dx; //i+1 셀과의 수면경사를 w 방향에 적용한다.
            //        //double slp = (dm.cells[cx + 1, ry].hp_tp1 - cell.hp_tp1) / dx; //i+1 셀과의 수면경사를 w 방향에 적용한다.
            //        double slp_tm1 = (mDM.mCells[cx + 1, ry].hp_t - cell.hp_t) / dx; //i+1 셀과의 수면경사를 w 방향에 적용한다.
            //        slp_tm1 = slp_tm1 + cHydro.domainOutBedSlope;
            //        if (slp_tm1 >= cGenEnv.slpMinLimitforFlow && cell.dp_tp1 > cGenEnv.dMinLimitforWet)
            //        {
            //            flxw = getFluxToDomainOut(cell, slp_tm1, cell.qw_t, cGenEnv.gravity, dt_sec);
            //        }
            //        else { flxw = new stFluxData(); }
            //    }
            //}
            //else
            //{
            //    //if (cell.dp_tp1 <= cGenEnv.dMinLimitforWet && dm.cells[cx - 1, ry].dp_tp1 <= cGenEnv.dMinLimitforWet)
            //        if (cell.isSimulatingCell ==-1 )
            //        {
            //        flxw = new stFluxData();
            //    }
            //    else
            //    {
            //        flxw.v = mDM.mCells[cx - 1, ry].ve_tp1;
            //        flxw.slp = mDM.mCells[cx - 1, ry].slpe;
            //        flxw.q = mDM.mCells[cx - 1, ry].qe_tp1;
            //        flxw.dflow = mDM.mCells[cx - 1, ry].dfe;
            //    }
            //}
            //cell.slpw = flxw.slp;
            //cell.dfw = flxw.dflow;
            //cell.vw_tp1 = flxw.v;
            //cell.qw_tp1 = flxw.q;
        }



        public void UpdateValuesInThisStepResults()
        {
            //var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            ////var options = new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount * 4 };
            ////int longerLength = nRows;
            //cThisProcess.maxResd = 0;
            //cThisProcess.subregionDflowmax = new double[nRows];
            ////cThisProcess.subregionDmax = new double[nRows];
            //cThisProcess.subregionVmax = new double[nRows];
            //cThisProcess.subregionVNCmin = new double[nRows];
            //cThisProcess.subregionMaxResd = new double[nRows];
            //cThisProcess.subregionMaxResdCell = new string[nRows];

            //Parallel.For(0, nRows, options, delegate (int ry)
            //{
            //    double maxdflow = 0;
            //    double maxd = 0;
            //    double maxv = 0;
            //    double minvnc = 9999;
            //    double maxR = 0;
            //    string maxRCell = "";
            //    for (int cx = 0; cx < nCols; cx++)
            //    {
            //        stCVAtt cell = mDM.mCells[cx, ry];
            //        if (cell.isSimulatingCell == 1)
            //        {
            //            stFluxData flxmax = cHydro.getFD4Max(cell);
            //            cell.fdmax = flxmax.fd;
            //            cell.vmax = flxmax.v;
            //            cell.Qmax_cms = flxmax.q * dx;
            //            if (flxmax.dflow > maxdflow) { maxdflow = flxmax.dflow; }
            //            if (cell.dp_tp1 > maxd) { maxd = cell.dp_tp1; }
            //            if (cell.vmax > maxv) { maxv = cell.vmax; }
            //            double vnCon = 0;
            //            if (cHydro.applyVNC == 1) { vnCon = getVonNeumanConditionValueUsingGPUFunction(cell); }
            //            if (vnCon < minvnc) { minvnc = vnCon; }
            //            if (cell.resd > maxR) { maxR = cell.resd; maxRCell = "(" + cell.colxary.ToString() + "," + cell.rowyary.ToString() + ")"; }
            //        }
            //    }
            //    cThisProcess.subregionDflowmax[ry] = maxdflow;
            //    //cThisProcess.subregionDmax[ry] = maxd;
            //    cThisProcess.subregionVmax[ry] = maxv;
            //    cThisProcess.subregionVNCmin[ry] = minvnc;
            //    cThisProcess.subregionMaxResd[ry] = maxR;
            //    cThisProcess.subregionMaxResdCell[ry] = maxRCell;
            //});

            //for (int nary = 0; nary < nRows; nary++)
            //{
            //    if (cGenEnv.dflowmaxInThisStep < cThisProcess.subregionDflowmax[nary]) { cGenEnv.dflowmaxInThisStep = cThisProcess.subregionDflowmax[nary]; }
            //    //if (cGenEnv.dmaxInThisStep < cThisProcess.subregionDmax[nary]) { cGenEnv.dmaxInThisStep = cThisProcess.subregionDmax[nary]; }
            //    if (cGenEnv.vmaxInThisStep < cThisProcess.subregionVmax[nary]) { cGenEnv.vmaxInThisStep = cThisProcess.subregionVmax[nary]; }
            //    if (cGenEnv.VNConMinInThisStep > cThisProcess.subregionVNCmin[nary]) { cGenEnv.VNConMinInThisStep = cThisProcess.subregionVNCmin[nary]; }
            //    if (cThisProcess.maxResd < cThisProcess.subregionMaxResd[nary])
            //    {
            //        cThisProcess.maxResd = cThisProcess.subregionMaxResd[nary];
            //        cThisProcess.maxResdCell = cThisProcess.subregionMaxResdCell[nary];
            //    }
            //}
        }


        public static void setSimulatingCell(cDomain dm, stCVAtt cell)
        {
            //cell.isSimulatingCell = 1;
            //int cx = cell.colxary;
            //int ry = cell.rowyary;
            //if (cx > 0 && cx < dm.nCols - 1)
            //{
            //    if (dm.mCells[cx - 1, ry].isInDomain == 1)
            //    {
            //        dm.mCells[cx - 1, ry].isSimulatingCell = 1;
            //    }
            //    if (dm.mCells[cx + 1, ry].isInDomain == 1)
            //    {
            //        dm.mCells[cx + 1, ry].isSimulatingCell = 1;
            //    }
            //}
            //if (cx == dm.nCols - 1 && dm.nCols > 1)
            //{
            //    if (dm.mCells[cx - 1, ry].isInDomain == 1)
            //    {
            //        dm.mCells[cx - 1, ry].isSimulatingCell = 1;
            //    }
            //}
            //if (cx == 0 && dm.nCols > 1)
            //{
            //    if (dm.mCells[cx + 1, ry].isInDomain == 1)
            //    {
            //        dm.mCells[cx + 1, ry].isSimulatingCell = 1;
            //    }
            //}
            //if (ry > 0 && ry < dm.nRows - 1)
            //{
            //    if (dm.mCells[cx, ry - 1].isInDomain == 1)
            //    {
            //        dm.mCells[cx, ry - 1].isSimulatingCell = 1;
            //    }
            //    if (dm.mCells[cx, ry + 1].isInDomain == 1)
            //    {
            //        dm.mCells[cx, ry + 1].isSimulatingCell = 1;
            //    }
            //}
            //if (ry == dm.nRows - 1 && dm.nRows > 1)
            //{
            //    if (dm.mCells[cx, ry - 1].isInDomain == 1)
            //    {
            //        dm.mCells[cx, ry - 1].isSimulatingCell = 1;
            //    }
            //}
            //if (ry == 0 && dm.nRows > 1)
            //{
            //    if (dm.mCells[cx, ry + 1].isInDomain == 1)
            //    {
            //        dm.mCells[cx, ry + 1].isSimulatingCell = 1;
            //    }
            //}
        }


        private static stFluxData getFluxUsingDhLimitBetweenTwoCell(stCVAtt curCell,
    stCVAtt tarCell, double dflow, double qAddedToCurCell, double slpTwocell, stFluxData flx, double dx, double dt_sec)
        {
            //double qbak = flx.q;
            //bool isHeightChanged = false;
            //double qoutAsDepth = flx.q * dt_sec / dx;
            //double curHeight = curCell.hp_tp1 - qoutAsDepth;
            //double depthOutNew = 0;
            //if (curHeight < curCell.elez)
            //{
            //    //depthOutNew = curCell.hp_tp1 - curCell.elez;
            //    curHeight = curCell.elez;
            //}
            ////double dflow_new = 0;
            //if (slpTwocell < 0 && qAddedToCurCell >= 0 && flx.q > 0)
            //{
            //    if (curCell.dp_t > 0 && curCell.hp_t > tarCell.hp_t && curHeight < (tarCell.hp_tp1 + qoutAsDepth))
            //    {
            //        depthOutNew = (curCell.hp_tp1 - tarCell.hp_tp1) / 2; //이게 두 셀의 수심이 같아 질 수 있는 유량에 의한 깊이
            //        //depthOutNew = curCell.hp_tp1 - tarCell.hp_tp1;
            //        //dflow_new = curCell.hp_tp1+depthOutNew - Math.Max(curCell.elez, tarCell.elez);
            //        isHeightChanged = true;
            //    }
            //}
            //if (slpTwocell > 0 && flx.q < 0)
            //{
            //    if (tarCell.dp_t > 0 && curCell.hp_t < tarCell.hp_t && curHeight > (tarCell.hp_tp1 + qoutAsDepth))// 이경우 QoutAsDepth은 - 이므로 더해줘야 빠진다..
            //    {
            //        //curHeight = tarCell.hp_tp1;
            //        //double dd = curHeight - curCell.hp_tp1;
            //        //depthOutNew = Math.Abs(QoutAsDepth) - dd;
            //        depthOutNew = (tarCell.hp_tp1 - curCell.hp_tp1) / 2;
            //        //depthOutNew = tarCell.hp_tp1 - curCell.hp_tp1;
            //        //dflow_new = tarCell.hp_tp1 + depthOutNew - Math.Max(curCell.elez, tarCell.elez);
            //        isHeightChanged = true;
            //    }
            //}
            //if (isHeightChanged == true)
            //{
            //    double qnew = depthOutNew * dx / dt_sec;
            //    if (Math.Abs(flx.q) > qnew)
            //    {
            //        flx.q = qnew;
            //        if (qbak < 0) { flx.q = -1 * qnew; }
            //        flx.v = flx.q / flx.dflow;
            //    }
            //}
            return flx;
        }

        private static stFluxData calFluxUsingME_mDWE_Implicit2(stCVAtt currentCell, stCVAtt targetCell,
                double qt, double qtp1, double dx, double dt_sec)
        {
            // 양의 값을 가지는 그래프로 변환해서 계산한다..
            stFluxData flx = new stFluxData();
            //double cur_hp_tp1 = currentCell.hp_t;
            //double targ_hp_tp1 = targetCell.hp_tp1;
            //double dht = targetCell.hp_t - currentCell.hp_t; //+면 자신의 셀이, 대상 셀보다 낮다, q는 -, slp는 +.   -면 자신의 셀이, 대상 셀보다 높다, q는 +, slp는 - 
            //double roughnessC = currentCell.rc;
            //double gdtn2 = cGenEnv.gravity * dt_sec * roughnessC * roughnessC;
            //double gdt = cGenEnv.gravity * dt_sec;
            //double qt_abs = Math.Abs(qt);
            //double qtp1_abs = Math.Abs(qtp1);
            //double qn = qtp1_abs;
            //bool converged = true;
            //double dhtp1 = 0;
            //double flowdepth = 0;
            //double slp = 0;
            //for (int i = 0; i < cGenEnv.iNRmax_forME; i++)
            //{
            //    dhtp1 = targ_hp_tp1 - cur_hp_tp1;
            //    double dhterm = -1 * Math.Abs(dhtp1);
            //    if (dhtp1 == 0) { return new stFluxData(); }
            //    if (dhtp1 > 0 && targetCell.dp_tp1 <= cGenEnv.dMinLimitforWet) { return new stFluxData(); }
            //    if (dhtp1 < 0 && currentCell.dp_tp1 <= cGenEnv.dMinLimitforWet) { return new stFluxData(); }
            //    slp = dhtp1 / dx;
            //    flowdepth = Math.Abs(Math.Max(cur_hp_tp1, targ_hp_tp1) - Math.Max(currentCell.elez, targetCell.elez));
            //    if (flowdepth <= 0) { return new stFluxData(); }
            //    double pressureTerm = flowdepth * gdt * dhterm / dx;
            //    double frictionTerm = qn * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
            //    double fn = qn - qt_abs + pressureTerm + frictionTerm;
            //    double dfn = 1 + 2 * gdtn2 * qn / Math.Pow(flowdepth, 7.0 / 3);
            //    if (dfn != 0)
            //    {
            //        double qnp1 = qn - fn / dfn;
            //        double dq = Math.Abs(qnp1 - qn);
            //        //double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
            //        double cC = cGenEnv.convergenceConditionq;
            //        if (dq <= cC)
            //        {
            //            flx.q = qnp1;
            //            converged = true;
            //            break;
            //        }
            //        qn = qnp1;
            //        cur_hp_tp1 = gethAfterFlowOut(currentCell, cur_hp_tp1, qn, dx, dt_sec);
            //        targ_hp_tp1 = gethAfterFlowIn(targetCell, targ_hp_tp1, qn, dx, dt_sec);
            //        converged = false;
            //    }
            //    else
            //    {
            //        flx.q = qt;
            //        converged = false;
            //        break;
            //    }
            //}
            //if (converged == false)
            //{
            //    cGenEnv.writelog(string.Format("Time : {0}sec. Momentum eq. was not converged. cell ({1}, {2})  ",
            //    cGenEnv.tnow_sec, currentCell.colxary, currentCell.rowyary), cGenEnv.bwritelog_process);
            //}
            //if (dhtp1 > 0) { flx.q = -1 * flx.q; }
            //flx.v = flx.q / flowdepth;
            //flx.dflow = flowdepth;
            //flx.slp = slp;
            return flx;
        }

        private stFluxData getFluxToEastOrSouth(double dx, stCVAtt currentCell,
          stCVAtt preCell, stCVAtt targetCell, int targetCellDir, double dt_sec)
        {
            //double slp = 0;
            ////이건 t 값
            ////double flowDepth = Math.Abs(Math.Max(currentCell.hp_tp1, ht_targetCell) - Math.Max(currentCell.elez, targetCell.elez));
            //double dht = targetCell.hp_t - currentCell.hp_t; //+면 자신의 셀이, 대상 셀보다 낮다, q는 -, slp는 +.   -면 자신의 셀이, 대상 셀보다 높다, q는 +, slp는 - 
            //double dhtp1 = targetCell.hp_tp1 - currentCell.hp_tp1;
            //double dh_inACV = targetCell.hp_tp1 - preCell.hp_tp1;
            //if (dhtp1 == 0) { return new stFluxData(); }
            //if (dhtp1 > 0 && targetCell.dp_tp1 <= cGenEnv.dMinLimitforWet) { return new stFluxData(); }
            //if (dhtp1 < 0 && currentCell.dp_tp1 <= cGenEnv.dMinLimitforWet) { return new stFluxData(); }
            //slp = dhtp1 / dx;
            //if (Math.Abs(slp) < cGenEnv.slpMinLimitforFlow) { return new stFluxData(); }
            //double slp_tm1 = dht / dx;
            //double dflow = Math.Abs(Math.Max(currentCell.hp_tp1, targetCell.hp_tp1) - Math.Max(currentCell.elez, targetCell.elez));
            //if (dflow <= 0) { return new stFluxData(); }
            //double qt = 0; double qtp1 = 0;

            ////double v_ip1 = 0; double q_ip1 = 0;
            //double qAddedToCurCell = 0;
            //double utp1 = 0;
            ////if (targetCellDir == cVars.FlowDirection4.E)
            //if (targetCellDir == 1)
            //{
            //    qt = currentCell.qe_t; qtp1 = currentCell.qe_tp1;
            //    //v_ip1 = targetCell.vw_t; q_ip1 = targetCell.qw_tp1;
            //    utp1 = targetCell.ve_tp1; //q_ip1 = targetCell.qe_tp1;
            //    //vw_tp1 = currentCell.vw_tp1;
            //    qAddedToCurCell = currentCell.qw_tp1;
            //    //if (slp < 0) { qAddedToHigherCell = currentCell.qw_tp1; } 
            //    // 이조건은 물리적으로는 맞지만, 계산순서상 currentCell에서의 유량이 미리 더해져서 계산되므로, target셀의 tp1 시간에서의 유입량을 먼저 고려할 필요는 없다.
            //    //else { qAddedToHigherCell = targetCell.qe_tp1; }
            //}
            ////else if (targetCellDir == cVars.FlowDirection4.S)
            //else if (targetCellDir == 3)
            //{
            //    qt = currentCell.qs_t; qtp1 = currentCell.qs_tp1;
            //    //v_ip1 = targetCell.vn_t; q_ip1 = targetCell.qn_tp1;
            //    utp1 = targetCell.vs_tp1; //q_ip1 = targetCell.qs_tp1;
            //    //vw_tp1 = currentCell.vn_tp1;
            //    qAddedToCurCell = currentCell.qn_tp1;
            //    //if (slp < 0) { qAddedToHigherCell = currentCell.qn_tp1; }
            //    //else { qAddedToHigherCell = targetCell.qs_tp1; }
            //}

            stFluxData flx = new stFluxData();
            //if (cGenEnv.isDWE == true)
            //{
            //    flx = calFluxUsingME_DWE_Implicit(dhtp1, dht, qt, qtp1, dflow, currentCell.rc, dx, dt_sec);
            //    //  flx = calFluxUsingMomentumEqDWImplicit_inACV(dhtp1, dh_inACV, qt, qtp1, dflow, vw_tp1, qw_tp1,
            //    //currentCell.lc.roughnessCoeff, dx, dt_sec, currentCell.colxary, currentCell.rowyary);             
            //    //flx = calculateMomentumEQ_DWE_Deterministric(qt, utp1, dflow, slp, currentCell.lc.roughnessCoeff, dx, dt_sec);
            //}
            //else
            //{
            //    //flx = calFluxUsingME_mDWE_Implicit(dhtp1, dht,
            //    //       qt, qtp1, dflow, currentCell.lc.roughnessCoeff, dx, dt_sec, currentCell.colxary, currentCell.rowyary);
            //    //flx = calFluxUsingME_mDWE_RootEQ(dhtp1, dht,
            //    //       qt, qtp1, dflow, currentCell.lc.roughnessCoeff, dx, dt_sec, currentCell.colxary, currentCell.rowyary);
            //    flx.q = calculateMomentumEQ_DWEm_Deterministric(qt, dflow, slp, currentCell.rc, cGenEnv.gravity, dt_sec);
            //    flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            //    flx.dflow = dflow;
            //}

            //if (cGenEnv.isAnalyticSolution == false)
            //{
            //    if (Math.Abs(flx.q) > 0)
            //    {
            //        if (cGenEnv.applyConstraints == true)
            //        {
            //            flx = getFluxUsingSubCriticalCon(flx);
            //            double iner = slp * qAddedToCurCell;
            //            //if(qAddedToHigherCell>=0)
            //            ////if (iner <= 0)
            //            //{
            //            //flx = getFluxUsingDhLimitBetweenTwoCell(currentCell, targetCell, dflow, qAddedToCurCell, slp, flx, dx, dt_sec);
            //            //}
            //            //else
            //            //{
            //            flx = getFluxLimitBetweenTwoCellUsingDflow(flx, dflow, dx, dt_sec); //이게 clf 모양과 비슷하다..
            //            //flx = getFluxqUsingFourDirLimitUsingDepthCondition(currentCell, flx, dflow, dx, dt_sec); //이건 수렴이 잘 안된다.

            //            //    //flx = getFluxUsingFourDirLimitUsingCellDepth(currentCell, targetCell, flx, dx, dt_sec);
            //            //    //flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
            //            //}
            //        }
            //    }
            //    else { return new stFluxData(); }
            //}
            //flx.slp = slp;
            return flx;
        }

        private static stFluxData calFluxUsingME_DWE_Implicit_UsingGPU(stGlobalValues gv)
        {
            //// 양의 값을 가지는 그래프로 변환해서 계산한다..
            //// df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
            stFluxData flx = new stFluxData();
            //double dhterm = -1 * DeviceFunction.Abs(gv.dh_tp1);
            //double gdtn2 = gv.gravity * gv.dt_sec * gv.rc *gv.rc;
            //double gdt = gv.gravity * gv.dt_sec;
            //double qt_app = DeviceFunction.Abs(gv.qt);
            //double qn = gv.qtp1;// Math.Abs(qtP1_i);
            //double ut = gv.qt / gv.dflow;
            //double qnp1 = gv.qt;
            //for (int i = 0; i < gv.iNRmax_forME; i++)
            //{
            //    double advectionTerm = ut * qn * gv.dt_sec / gv.dx; //이건 엇갈린 격자에서 검사체적면에서의 운동량 사용
            //    double pressureTerm = gv.dflow * gdt * dhterm / gv.dx;
            //    double frictionTerm = qn * qn * gdtn2 / DeviceFunction.Pow(gv.dflow, 7.0 / 3);
            //    double fn = qn - qt_app + advectionTerm + pressureTerm + frictionTerm;
            //    double dfn = 1 - ut * gv.dt_sec / gv.dx + 2 * DeviceFunction.Abs(qn) * gdtn2 / DeviceFunction.Pow(gv.dflow, 7.0 / 3);
            //    if (dfn != 0)
            //    {
            //        qnp1 = qn - fn / dfn;
            //        double dq = DeviceFunction.Abs(qnp1 - qn);
            //        double cC = gv.convergenceConditionh;
            //        if (dq <= cC)
            //        {
            //            converged = true;
            //            break;
            //        }
            //        qn = qnp1;
            //        //ut = qn / flowdepth; // 이것을 사용하면, 1차 도함수가 비선형이 된다.
            //        converged = false;
            //    }
            //    else
            //    {
            //        converged = false;
            //        break;
            //    }
            //    //if (cThisProcess.maxNR_inME < i) { cThisProcess.maxNR_inME = i; }
            //}
            //flx.q = qnp1;
            //if (gv.dh_tp1 > 0)
            //{
            //    flx.q = -1 * flx.q;
            //}
            //flx.v = flx.q / gv.dflow;
            //flx.dflow = gv.dflow;
            return flx;
        }



        private static stFluxData calFluxUsingME_DWE_Implicit(double dh_tp1, double dh_t, double qt_i, double qtP1_i,
            double flowdepth, double roughnessC, double dx, double dt_sec)
        {
            //// 양의 값을 가지는 그래프로 변환해서 계산한다..
            //// df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
            stFluxData flx = new stFluxData();
            ////double dhterm = -1 * Math.Abs(dh_tp1 + dh_t); //이건 압력항 CR
            //double dhterm = -1 * Math.Abs(dh_tp1);
            ////double dhterm = dh_tp1;
            //double gdtn2 = cGenEnv.gravity * dt_sec * roughnessC * roughnessC;
            //double gdt = cGenEnv.gravity * dt_sec;
            ////double ut = qt / flowdepth;
            ////double ut_abs = Math.Abs(ut);
            ////double un = ut_abs; //qtp1의 초기값으로 사용
            //double qt_app = Math.Abs(qt_i);
            //double qn = qtP1_i;// Math.Abs(qtP1_i);
            //double ut = qt_i / flowdepth;
            ////double u_ip1_app = Math.Abs(ut_ip1);
            ////double q_ip1_app = Math.Abs(qtp1_ip1);
            ////double u_ip1_app = ut_ip1;
            ////double q_ip1_app = qtp1_ip1;
            //double qnp1 = qt_i;
            //bool converged = true;

            //for (int i = 0; i < cGenEnv.iNRmax_forCE; i++)
            //{
            //    //un = (un + u_t_app) / 2;
            //    double advectionTerm = ut * qn * dt_sec / dx; //이건 엇갈린 격자에서 검사체적면에서의 운동량 사용
            //    //double advectionTerm = (u_ip1_app * q_ip1_app - ut * qn) * dt_sec / dx; // 이건 두셀에서의 가속도 고려(i 셀은 e, ip1셀은 w 성분
            //    double pressureTerm = flowdepth * gdt * dhterm / dx;
            //    double frictionTerm = qn * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
            //    double fn = qn - qt_app + advectionTerm + pressureTerm + frictionTerm;
            //    double dfn = 1 - ut * dt_sec / dx + 2 * Math.Abs(qn) * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);

            //    if (dfn != 0)
            //    {
            //        qnp1 = qn - fn / dfn;
            //        double dq = Math.Abs(qnp1 - qn);
            //        double cC = cGenEnv.convergenceConditionh;
            //        //double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
            //        if (dq <= cC)
            //        {
            //            //flx.q = qnp1;
            //            converged = true;
            //            break;
            //        }
            //        qn = qnp1;
            //        //ut = qn / flowdepth; // 이것을 사용하면, 1차 도함수가 비선형이 된다.
            //        converged = false;
            //    }
            //    else
            //    {
            //        //flx.q = qt_i;
            //        converged = false;
            //        break;
            //    }
            //    if (cThisProcess.maxNR_inME < i) { cThisProcess.maxNR_inME = i; }
            //}
            //if (converged == false)
            //{
            //    //cGenEnv.writelog(string.Format("Time : {0}sec. Momentum eq. was not converged. cell ({1}, {2})  ",
            //    //cGenEnv.tnow_sec, cx, ry), true);
            //}
            //flx.q = qnp1;
            //if (dh_tp1 > 0)
            //{
            //    flx.q = -1 * flx.q;
            //}
            //flx.v = flx.q / flowdepth;
            //flx.dflow = flowdepth;

            return flx;
        }

        private stFluxData getFluxToDomainOut(stCVAtt currentCell, double slp, double qt, double gravity, double dt_sec)
        {
            stFluxData flx = new stFluxData();
            //double dflow = currentCell.dp_t; // 이것으로 하면 경계셀에서 진동하는 경우 있음.
            double dflow = currentCell.dp_tp1; // 이게 들어오는 수심에 맞게 유출을 하므로, 이게 맞다.
            double rc = currentCell.rc;
            if (dflow <= 0) { return new stFluxData(); }
            //if (Math.Abs(slp) < cGenEnv.slpMinLimitforFlow) { return mNoflx; }
            //flx.q =calculateMomentumEQ_DWEm_Deterministric(qt, dflow, slp, rc, gravity, dt_sec);
            flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            flx.dflow = dflow;
            //flx = calFluxUsingManningEq(slp, dflow, rc,dx );
            if (cGenEnv.isAnalyticSolution == false)
            {
                if (Math.Abs(flx.q) > 0)
                {
                    //flx = getFluxUsingSubCriticalCon(flx);
                    //double dh_qlimit = Math.Abs(dhtp1);
                    //if (dh_qlimit > flowDepth) { dh_qlimit = flowDepth; } // 고도차에 의해서 이런 경우 발생할 수 있음.
                    //flx = getFluxUsingFourDirLimitUsingDflow(flx, dflow, dx, dt_sec); //이게 clf 모양과 비슷하다..
                    //flx = getFluxDominOutUsingCellDepth(currentCell, flx, dx, dt_sec);
                    //double dhtp1 = dflow * Math.Abs(slp);
                    //flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
                }
            }
            flx.slp = slp;
            return flx;
        }


        /// <summary>
        /// w, n의 수문경사를 이용해서 domain 외부로 유출되게.. 계산. domain 외부에서 안쪽으로 유입은 없게 계산
        /// </summary>
        private stFluxData getFluxToDomainOutUsingAry(stCVAtt currentCell, double slp, double qt, double ut, double gravity, double dt_sec)
        {
            stFluxData flx = new stFluxData();
            //double dflow = currentCell.dp_t; // 이것으로 하면 경계셀에서 진동하는 경우 있음.
            double dflow = currentCell.dp_tp1; // 이게 들어오는 수심에 맞게 유출을 하므로, 이게 맞다.
            double rc = currentCell.rc;
            if (dflow <= 0) { return new stFluxData(); }
            //if (Math.Abs(slp) < cGenEnv.slpMinLimitforFlow) { return mNoflx; }
            //flx.q = calculateMomentumEQ_DWEm_Deterministric(qt, dflow, slp, rc, gravity, dt_sec);
            flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
            flx.dflow = dflow;
            //flx = calFluxUsingManningEq(slp, dflow, rc,dx );
            //if (cGenEnv.isAnalyticSolution == false)
            //{
            //    if (Math.Abs(flx.q) > 0)
            //    {
            //        //flx = getFluxUsingSubCriticalCon(flx);
            //        //double dh_qlimit = Math.Abs(dhtp1);
            //        //if (dh_qlimit > flowDepth) { dh_qlimit = flowDepth; } // 고도차에 의해서 이런 경우 발생할 수 있음.
            //        //flx = getFluxUsingFourDirLimitUsingDflow(flx, dflow, dx, dt_sec); //이게 clf 모양과 비슷하다..
            //        //flx = getFluxDominOutUsingCellDepth(currentCell, flx, dx, dt_sec);
            //        //double dhtp1 = dflow * Math.Abs(slp);
            //        //flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
            //    }
            //}
            flx.slp = slp;
            return flx;
        }

        private static double gethAfterFlowOut(stCVAtt cell, double hori, double q, double dx, double dt)
        {
            //double qout = q * dt;
            //double dout = qout / dx;
            //double h = hori - dout;
            //if (h < cell.elez) { return cell.elez; }
            //return h;
            return 0;
        }

        private static double gethAfterFlowIn(stCVAtt cell, double hori, double q, double dx, double dt)
        {
            //double qin = q * dt;
            //double din = qin / dx;
            //double h = hori + din;
            //if (h < cell.elez) { return cell.elez; }
            //return h;
            return 0;
        }

        #region "not used"

        private static stFluxData calFluxUsingME_DWE_Implicit_inACV(double dh_tp1, double dhinACV, double qt_i,
   double qtP1_i, double flowdepth, double uw_tp1, double qw_tp1, double roughnessC, double dx, double dt_sec, int cx, int ry)
        {
            // 양의 값을 가지는 그래프로 변환해서 계산한다..
            // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
            stFluxData flx = new stFluxData();
            //double dhterm = -1 * Math.Abs(dhinACV);
            ////double dhterm = -1 * Math.Abs(dh_tp1);
            ////double dhterm = dh_tp1;
            //double gdtn2 = cGenEnv.gravity * dt_sec * roughnessC * roughnessC;
            //double gdt = cGenEnv.gravity * dt_sec;
            //double qt_app = Math.Abs(qt_i);
            //double qn = Math.Abs(qtP1_i);
            //double ut = qn / flowdepth;
            ////double uw_app = Math.Abs(uw_tp1);
            ////double qw_app = Math.Abs(qw_tp1);
            //double uw_app = uw_tp1;
            //double qw_app = qw_tp1;
            //double qnp1 = qt_i;
            //bool converged = true;
            //for (int i = 0; i < cGenEnv.iNRmax_forCE; i++)
            //{
            //    double advectionTerm = (ut * qn - uw_app * qw_app) * dt_sec / dx; // 이건 w와 e 사이의 가속도 고려
            //    double pressureTerm = flowdepth * gdt * dhterm / dx;
            //    double frictionTerm = qn * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
            //    //double frictionTerm = qt_i * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3); //양해적 기법
            //    double fn = qn - qt_app + advectionTerm + pressureTerm + frictionTerm;
            //    double dfn = 1 + ut * dt_sec / dx + 2 * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
            //    //double dfn = 1 + ut * dt_sec / dx + qt_i * gdtn2 / Math.Pow(flowdepth, 7.0 / 3); //양해적 기법
            //    if (dfn != 0)
            //    {
            //        qnp1 = qn - fn / dfn;
            //        double dq = Math.Abs(qnp1 - qn);
            //        double cC = cGenEnv.convergenceConditionh;
            //        //double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
            //        if (dq <= cC)
            //        {
            //            //flx.q = qnp1;
            //            converged = true;
            //            break;
            //        }
            //        qn = qnp1;
            //        //ut = qn / flowdepth; //todo
            //        converged = false;
            //    }
            //    else
            //    {
            //        converged = false;
            //        break;
            //    }
            //    if (cThisProcess.maxNR_inME < i) { cThisProcess.maxNR_inME = i; }
            //}
            //if (converged == false)
            //{
            //    cGenEnv.writelog(string.Format("Time : {0}sec. Momentum eq. was not converged. cell ({1}, {2})  ",
            //    cGenEnv.tnow_sec, cx, ry), cGenEnv.bwritelog_process);
            //}
            //flx.q = qnp1;

            //if (dh_tp1 > 0)
            //{
            //    flx.q = -1 * flx.q;
            //}
            //flx.v = flx.q / flowdepth;
            //flx.dflow = flowdepth;

            return flx;
        }





        //private void RunSoverInThreadTask_1()
        //{

        //    Parallel.ForEach(Partitioner.Create(0, mCVCount1P4), mPoptions,
        //        (range) =>
        //        {
        //            for (int i = range.Item1; i < range.Item2; i++)
        //            {
        //                int ry = mDM.cvs[i].rowyary;
        //                int cx = mDM.cvs[i].colxary;
        //                if (mDM.cvs[i].isSimulatingCell == true)
        //                {
        //                    calculateContinuityEqUsingNR( ry, cx, mdx, mdt_sec, migs);
        //                   if (cGenEnv.movingDomain == true && mDM.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
        //                    { cDomain.setSimulatingCell(mDM, mDM.cvs[i]); }

        //                }
        //            }
        //        });
        //}

        //private void RunSoverInThreadTask_2()
        //{

        //    Parallel.ForEach(Partitioner.Create(mCVCount1P4, mCVCount2P4), mPoptions,
        //        (range) =>
        //        {
        //            for (int i = range.Item1; i < range.Item2; i++)
        //            {
        //                int ry = mDM.cvs[i].rowyary;
        //                int cx = mDM.cvs[i].colxary;
        //                if (mDM.cvs[i].isSimulatingCell == true)
        //                {
        //                    calculateContinuityEqUsingNR( ry, cx, mdx, mdt_sec, migs);
        //                    if (cGenEnv.movingDomain == true && mDM.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
        //                    { cDomain.setSimulatingCell(mDM, mDM.cvs[i]); }

        //                }
        //            }
        //        });
        //}

        //private void RunSoverInThreadTask_3()
        //{

        //    Parallel.ForEach(Partitioner.Create(mCVCount2P4, mCVCount3P4), mPoptions,
        //        (range) =>
        //        {
        //            for (int i = range.Item1; i < range.Item2; i++)
        //            {
        //                int ry = mDM.cvs[i].rowyary;
        //                int cx = mDM.cvs[i].colxary;
        //                if (mDM.cvs[i].isSimulatingCell == true)
        //                {
        //                    calculateContinuityEqUsingNR(ry, cx, mdx, mdt_sec, migs);
        //                    if (cGenEnv.movingDomain == true && mDM.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
        //                    { cDomain.setSimulatingCell(mDM, mDM.cvs[i]); }

        //                }
        //            }
        //        });
        //}

        //private void RunSoverInThreadTask_4()
        //{

        //    Parallel.ForEach(Partitioner.Create(mCVCount3P4, mTotCountCV), mPoptions,
        //        (range) =>
        //        {
        //            for (int i = range.Item1; i < range.Item2; i++)
        //            {
        //                int ry = mDM.cvs[i].rowyary;
        //                int cx = mDM.cvs[i].colxary;
        //                if (mDM.cvs[i].isSimulatingCell == true)
        //                {
        //                    calculateContinuityEqUsingNR(ry, cx, mdx, mdt_sec, migs);
        //                    if (cGenEnv.movingDomain == true && mDM.cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet)
        //                    { cDomain.setSimulatingCell(mDM, mDM.cvs[i]); }

        //                }
        //            }
        //        });
        //}

        [GpuManaged]
        public void RunSolverUsingGPU_old(stCVAtt[] cvs, stCVAtt_add[] cvsadd, stGlobalValues[] gv, stBCinfo[] bcinfos)
        {
            //var gpu = Gpu.Default;
            //var lp = new LaunchParam(16, 256);
            ////var lp = new LaunchParam(16, 512);
            ////var lp = new LaunchParam(16,16);
            //gpu.Launch(RunSolverUsingGPU_kernel_old, lp, cvs, gv, bcinfos);
            ////gpu.Synchronize();
            //UpdateValuesInThisStepResultsUsing1DArray(cvs, cvsadd);
            ////Gpu.FreeAllImplicitMemory(); // 이거하는데 10ms 정도 소요된다.
        }


        [GpuManaged]
        public static void RunSolverUsingGPU_kernel_old(stCVAtt[] cvs, stGlobalValues[] gv, stBCinfo[] bcinfos)
        {
            //var start = blockIdx.x * blockDim.x + threadIdx.x;
            //var stride = gridDim.x * blockDim.x;

            //for (int igs = 0; igs < gv[0].iGSmax; igs++) // 테스트 결과. 밖에 있는 루프는 순차적으로 실행..  cGenEnv.iGSmax 
            //{
            //    gv[0].bAllConvergedInThisGSiteration = 1;
            //    gv[0].iNR = 0;
            //    for (int i = start; i < cvs.Length; i += stride)
            //    {
            //        //if (igs == 0) { cSimulationSetting.initializeAcellUsingArray_static(cvs, i, gv); }
            //        if (cvs[i].isSimulatingCell == 1)
            //        {
            //            int bcCellid = cConditionData.getbcArrayIndex(bcinfos, i);
            //            int isBCcell = -1;
            //            double bcDepth = 0;
            //            int bctype = 0;
            //            if (bcCellid >= 0) { isBCcell = 1; bcDepth = bcinfos[bcCellid].bcDepth_dt_m_tp1; bctype = bcinfos[bcCellid].bctype; }
            //            calculateContinuityEqUsingNRUsing1DArray(cvs, i, gv, isBCcell, bcDepth, bctype);
            //            //calculateContinuityEqUsingNRUsing1DArray(cvs, i, gv);
            //            if (cvs[i].dp_tp1 > gv[0].dMinLimitforWet)
            //            { cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i); }
            //        }
            //    }
            //    gv[0].iGS = igs;
            //    if (gv[0].bAllConvergedInThisGSiteration == 1)
            //    {
            //        break;
            //    }
            //}
        }

        private static stFluxData calFluxUsingME_mDWE_Implicit(double dh_tp1, double dh_t,
                  double qt, double qtp1, double flowdepth, double roughnessC, double dx, double dt_sec, int cx, int ry)
        {
            // 양의 값을 가지는 그래프로 변환해서 계산한다..
            // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
            stFluxData flx = new stFluxData();
            //double dhterm = -1 * Math.Abs(dh_tp1 + dh_t); //이건 압력항 CR
            double dhterm = -1 * Math.Abs(dh_tp1);
            double gdtn2 = cGenEnv.gravity * dt_sec * roughnessC * roughnessC;
            double gdt = cGenEnv.gravity * dt_sec;
            //double ut = qt / flowdepth;
            //double ut_abs = Math.Abs(ut);
            //double un = ut_abs; //qtp1의 초기값으로 사용
            double qt_abs = Math.Abs(qt);
            //double qn = qt_abs;
            double qtp1_abs = Math.Abs(qtp1);
            double qn = qtp1_abs;
            bool converged = true;
            for (int i = 0; i < cGenEnv.iNRmax_forME; i++)
            {
                #region "solutions"
                //// LISFLood 하고 비교할때는 아래 power에서 double을 빼야 한다.ㅋㅋ clf 1.9b에서 double을 빼고 코딩 되어 있다..
                //double frictionTerm = 0.5 * ut * gdtn2 / Math.Pow(flowdepth, 4 / 3) * (un + ut);
                //double fn = un - ut + 0.5 * gdt * dhterm / dx + frictionTerm;
                //double dfn = 1 + 0.5 * ut * gdtn2 / Math.Pow(flowdepth, 4 / 3);

                // 정식계산,  해석해하고 비교할때는 double 넣어야 한다.(이건 CR과 양해적 CR 기법 혼용) 
                //double frictionTerm = 0.5 * ut * gdtn2 / Math.Pow(flowdepth, (double)4 / 3) * (un + ut);
                //double fn = un - ut + 0.5 * gdt * dhterm / dx + frictionTerm;
                //double dfn = 1 + 0.5 * ut * gdtn2 / Math.Pow(flowdepth, (double)4 / 3);

                //// 정식계산,  해석해하고 비교할때는 double 넣어야 한다.(이건 CR과 양해적 기법 혼용) 
                //double frictionTerm = un * ut_abs * gdtn2 / Math.Pow(flowdepth, (double)4 / 3);
                //double fn = un - ut_abs + 0.5 * gdt * dhterm / dx + frictionTerm;
                //double dfn = 1 + ut_abs * gdtn2 / Math.Pow(flowdepth, (double)4 / 3);
                #endregion
                // 정식계산,  해석해하고 비교할때는 double 넣어야 한다.(유량. 음해법)
                double pressureTerm = flowdepth * gdt * dhterm / dx;
                double frictionTerm = qn * qn * gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
                double fn = qn - qt_abs + pressureTerm + frictionTerm;
                double dfn = 1 + 2 * gdtn2 * qn / Math.Pow(flowdepth, 7.0 / 3);
                if (dfn != 0)
                {
                    double qnp1 = qn - fn / dfn;
                    double dq = Math.Abs(qnp1 - qn);
                    //double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
                    double cC = cGenEnv.convergenceConditionq;
                    if (dq <= cC)
                    {
                        flx.q = qnp1;
                        converged = true;
                        break;
                    }
                    qn = qnp1;
                    converged = false;
                }
                else
                {
                    flx.q = qt;
                    converged = false;
                    break;
                }
            }
            if (converged == false)
            {
                cGenEnv.writelog(string.Format("Time : {0}sec. Momentum eq. was not converged. cell ({1}, {2})  ",
                cGenEnv.tnow_sec, cx, ry), cGenEnv.bwritelog_process);
            }
            if (dh_tp1 > 0) { flx.q = -1 * flx.q; }
            flx.v = flx.q / flowdepth;
            flx.dflow = flowdepth;
            return flx;
        }


        private static stFluxData calFluxUsingME_mDWE_RootEQ(double dh_tp1, double dh_t,
                   double qt, double qtp1, double flowdepth, double roughnessC, double dx, double dt_sec, int cx, int ry)
        {
            // 양의 값을 가지는 그래프로 변환해서 계산한다..
            // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
            stFluxData flx = new stFluxData();
            double dhterm = -1 * Math.Abs(dh_tp1);
            double gdtn2 = cGenEnv.gravity * dt_sec * roughnessC * roughnessC;
            double gdt = cGenEnv.gravity * dt_sec;
            double qt_abs = Math.Abs(qt);
            double qtp1_abs = Math.Abs(qtp1);
            double qn = qtp1_abs;
            double a = gdtn2 / Math.Pow(flowdepth, 7.0 / 3);
            double b = 1;
            double c = flowdepth * gdt * dhterm / dx - qtp1_abs;
            double b2M4ac = Math.Pow(b, 2.0) - 4 * a * c;
            if (b2M4ac < 0) { System.Diagnostics.Debugger.Break(); }
            flx.q = (Math.Sqrt(b2M4ac) - b) / (2 * a);
            if (dh_tp1 > 0) { flx.q = -1 * flx.q; }
            flx.v = flx.q / flowdepth;
            flx.dflow = flowdepth;
            return flx;
        }









        //private static cFluxData getFluxUsingFourDirLimitUsingDh(cFluxData inflx, double dh, double dx, double dt_sec)
        //{
        //    // 이건 과도하게 유량을 제한한다... 안정적이긴 하나, 수위전파가 너무 느리다..
        //    cFluxData flx = new cFluxData();
        //    flx = inflx;
        //    double qmax = Math.Abs(dh) * dx / dt_sec; //1차원 
        //    double qbak = inflx.q;
        //    if (Math.Abs(inflx.q) > qmax)
        //    {
        //        flx.q = qmax;
        //        if (qbak < 0) { flx.q = -1 * flx.q; }
        //        flx.v = qmax / flx.dflow;
        //    }
        //    return flx;
        //}

        private static stFluxData calFluxUsingManningEq(double slp, double flowDepth, double roughness, double dx)
        {
            stFluxData flx = new stFluxData();
            double slpapp = Math.Abs(slp);
            double r = flowDepth * dx / (dx + dx * flowDepth);
            double v = Math.Pow(flowDepth, 2.0 / 3) * Math.Pow(slpapp, 0.5)
                          / roughness;
            flx.v = v;
            if (slp > 0) { flx.v = -1 * v; }
            flx.q = flx.v * flowDepth;
            flx.dflow = flowDepth;
            return flx;
        }


        private double getDAverageOfFiveCell(cDomain dm, int cx, int ry, double d_cur)
        {
            //int cellcount = 1;
            //double hsum = d_cur + dm.mCells[cx, ry].elez;// dm.cells[cx,ry].hp_tp1;

            //if (cx > 0 && dm.mCells[cx - 1, ry].isInDomain == 1)
            //{
            //    if (dm.mCells[cx - 1, ry].hp_tp1 > 0) { cellcount += 1; hsum += dm.mCells[cx - 1, ry].hp_tp1; }
            //}
            //if (cx < (dm.nCols - 1) && dm.mCells[cx + 1, ry].isInDomain  == 1)
            //{
            //    if (dm.mCells[cx + 1, ry].hp_tp1 > 0) { cellcount += 1; hsum += dm.mCells[cx + 1, ry].hp_tp1; }
            //}

            //if (ry > 0 && dm.mCells[cx, ry - 1].isInDomain == 1)
            //{
            //    if (dm.mCells[cx , ry-1].hp_tp1 > 0) { cellcount += 1; hsum += dm.mCells[cx , ry-1].hp_tp1; }
            //}

            //if (ry < (dm.nRows - 1) && dm.mCells[cx, ry + 1].isInDomain == -1)
            //{
            //    if (dm.mCells[cx, ry + 1].hp_tp1 > 0) { cellcount += 1; hsum += dm.mCells[cx, ry + 1].hp_tp1; }
            //}
            //double aved = hsum / cellcount- dm.mCells[cx, ry].dp_tp1;
            //if (aved < 0) { aved = 0; }
            //return aved;
            return 0;
        }

        private stFluxData getFluxLimitBetweenTwoCellUsingDflow(stFluxData inflx, double dflow, double dx, double dt_sec)
        {
            double qmax = Math.Abs(dflow) * dx / 2 / dt_sec;
            double qbak = inflx.q;
            if (Math.Abs(inflx.q) > qmax)
            {
                inflx.q = qmax;
                if (qbak < 0) { inflx.q = -1 * qmax; }
                inflx.v = inflx.q / inflx.dflow;
            }
            return inflx;
        }

        //private double getAverageDepthofFiveCell(cDomain dm, int cxary, int ryary)
        //{
        //    double dc = dm.cells[cxary, ryary].dp_tp1;
        //    double dn = 0, ds = 0, de = 0, dw = 0;
        //    if (ryary > 0)
        //    {
        //        dn = dm.cells[cxary, ryary - 1].dp_tp1;
        //    }
        //    if (ryary < dm.nRows - 1)
        //    {
        //        ds = dm.cells[cxary, ryary + 1].dp_tp1;
        //    }
        //    if (cxary > 0)
        //    {
        //        dw = dm.cells[cxary - 1, ryary].dp_tp1;
        //    }
        //    if (cxary < dm.nCols - 1)
        //    {
        //        de = dm.cells[cxary + 1, ryary].dp_tp1;
        //    }
        //    double dave = (dc + dn + ds + de + dw) / 5;
        //    return dave;
        //}

        //private static cFluxData calFluxUsingMomentumEqNR1(double qini, double qt, double dh_tp1, double dflow, double slp, double mN, double dx, int dt_sec)
        //{
        //    cFluxData flx = new cFluxData();
        //    double qn = qini; //qtp1의 초기값으로 사용
        //    double nc2 = mN * mN;
        //    double gdt = cGenEnv.gravity * dt_sec;
        //    double gdtnc2 = gdt * nc2;
        //    for (int inr = 0; inr<1000; inr++)
        //    {
        //        //double fn = qn - qt + gdt * dflow * dh_tp1 / dx + gdtnc2 * (qn * qn + qt * qt) / 2 / Math.Pow(dflow, (7 / 3));
        //        double fn = qn - qt + gdt * dflow * dh_tp1 / dx + gdtnc2 * (qn * qn) / Math.Pow(dflow, (7 / 3));
        //        double dfn = 1 + 2*gdtnc2 * qn / Math.Pow(dflow, (7 / 3));
        //        double qnp1 = qn - fn / dfn;
        //        double dq = Math.Abs(qnp1 - qn);
        //        double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
        //        if (dq < cC)
        //        {
        //            flx.q = Math.Abs(qnp1); ;
        //            flx.v = flx.q / dflow;
        //            flx.dflow = dflow;
        //            flx.slp = slp;
        //            return flx;
        //        }
        //        qn = qnp1;
        //    }
        //    return flx;
        //}


        //private static cFluxData calFluxUsingMomentumEqNR2(double qtp1_ini, double d, double ht_currentCell,
        //           double ht_targetCell, double qt, double flowdepth, double roughnessC, double dx, int dt_sec)
        //{
        //    // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
        //    cFluxData flx = new cFluxData();
        //    double qn = qtp1_ini; //qtp1의 초기값으로 사용
        //    double dh_t = ht_targetCell - ht_currentCell;
        //    double nc2 = roughnessC * roughnessC;
        //    for (int i = 0; i < 1000; i++)
        //    {
        //        double qn2 = qn * qn;
        //        double dh_tp1 = dx * qn2 * nc2 / Math.Pow(flowdepth, 10 / 3);
        //        double fn_friction = 0.5 * cGenEnv.gravity * nc2 * dt_sec / Math.Pow(flowdepth, 7 / 3) * (qn2 + qt * qt);
        //        double fn = qn - qt + 0.5 * cGenEnv.gravity * flowdepth * dt_sec * (dh_tp1 + dh_t) / dx + fn_friction;
        //        //double dfn = 1 + cGenEnv.gravity * nc2 * dt_sec * qn / Math.Pow(flowdepth, 7 / 3);
        //        double dfn = 1 + cGenEnv.gravity * nc2 * dt_sec * d * qn / Math.Pow(flowdepth, 10 / 3)
        //                             + cGenEnv.gravity * nc2 * dt_sec * qn / Math.Pow(flowdepth, 7 / 3);
        //        double qnp1 = qn - fn / dfn;
        //        //double err = Math.Abs((qnp1 - qn) / qn);
        //        double dq = Math.Abs(qnp1 - qn);
        //        double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
        //        if (dq < cC)
        //        {
        //            flx.q = Math.Abs(qnp1);;
        //            flx.v = flx.q / flowdepth;
        //            flx.dflow = flowdepth;
        //            flx.slp = dh_tp1 / dx;
        //            return flx;
        //        }
        //        qn = qnp1;
        //    }
        //    return flx;
        //}

        //private static cFluxData calFluxUsingMomentumEqNRasPositive3(double dh_tp1, double dh_t,
        //    double qtp1_ini, double qt, double flowdepth, double roughnessC, double dx, int dt_sec, int cx, int ry)
        //{
        //    // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
        //    cFluxData flx = new cFluxData();
        //    double qn = qtp1_ini; //qtp1의 초기값으로 사용
        //    double nc2 = roughnessC * roughnessC;
        //    for (int i = 0; i < 1000; i++)
        //    {
        //        double qn2 = qn * qn;
        //        double fn_friction = 0.5 * cGenEnv.gravity * nc2 * dt_sec / Math.Pow(flowdepth, 7 / 3) * (qn2 + qt * qt);
        //        double fn = qn - qt + 0.5 * cGenEnv.gravity * flowdepth * dt_sec * (dh_tp1 + dh_t) / dx + fn_friction;
        //        double dfn = 1 + cGenEnv.gravity * nc2 * dt_sec * qn / Math.Pow(flowdepth, 7 / 3);
        //        double qnp1 = qn - fn / dfn;
        //        //double err = Math.Abs((qnp1 - qn) / qn);
        //        double dq = Math.Abs(qnp1 - qn);
        //        double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
        //        if (dq <= cC)
        //        {
        //            flx.q = Math.Abs(qnp1);
        //            flx.v = flx.q / flowdepth;
        //            flx.dflow = flowdepth;
        //            flx.slp = dh_tp1 / dx;
        //            return flx;
        //        }
        //        qn = qnp1;
        //    }
        //    cGenEnv.writelog(string.Format("Momentum eq. was not converged. Time : {0}sec, (x, y) : ({1}, {2})  ", cGenEnv.tnow_sec, cx, ry));
        //    return flx;
        //}

        //    private static cFluxData calFluxUsingMomentumEqNRasPositive4(double dh_tp1, double dh_t,
        //double qtp1_ini, double qt, double flowdepth, double roughnessC, double dx, int dt_sec, int cx, int ry)
        //    {
        //        // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
        //        cFluxData flx = new cFluxData();
        //        double qn = qtp1_ini; //qtp1의 초기값으로 사용
        //        double nc2 = roughnessC * roughnessC;
        //        double hterm = dh_tp1 + dh_t;
        //        double gdt = cGenEnv.gravity * dt_sec;
        //        for (int i = 0; i < 1000; i++)
        //        {
        //            double qn2 = qn * qn;
        //            double fn_friction = 0.5 * gdt * nc2 / Math.Pow(flowdepth, 7 / 3) * (qn2 + qt * qt);
        //            double un = qn / flowdepth;
        //            double fn = qn - qt + 0.5 * gdt * qn * hterm / dx / un + fn_friction;
        //            double dfn = 1 + 0.5 * gdt * hterm / dx / un + gdt * nc2 * qn / Math.Pow(flowdepth, 7 / 3);
        //            double qnp1 = qn - fn / dfn;
        //            //double err = Math.Abs((qnp1 - qn) / qn);
        //            double dq = Math.Abs(qnp1 - qn);
        //            double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
        //            if (dq <= cC)
        //            {
        //                flx.q = Math.Abs(qnp1);
        //                flx.v = flx.q / flowdepth;
        //                flx.dflow = flowdepth;
        //                flx.slp = dh_tp1 / dx;
        //                return flx;
        //            }
        //            qn = qnp1;
        //        }
        //        cGenEnv.writelog(string.Format("Momentum eq. was not converged. Time : {0}sec, (x, y) : ({1}, {2})  ", cGenEnv.tnow_sec, cx, ry));
        //        return flx;
        //    }


        //private static cFluxData calFluxUsingMomentumEqNRasPositive5(double dd_tp1, double dd_t,
        //         double utp1_ini, double ut, double flowdepth, double roughnessC, double Sb, double dx, int dt_sec, int cx, int ry)
        //{
        //    // df는 고정, 유속, 유량은 바뀐다.. 즉, 주어진 수심, 수위 조건에서 유량과 유속을 계산한다. 수위는 연속방정식에서 계산..
        //    cFluxData flx = new cFluxData();
        //    double un = ut;// utp1_ini; //qtp1의 초기값으로 사용
        //    double nc2 = roughnessC * roughnessC;
        //    for (int i = 0; i < 1000; i++)
        //    {
        //        double un2 = un * un;
        //        double fn_friction = 0.5 * cGenEnv.gravity * nc2 * dt_sec / Math.Pow(flowdepth, 4 / 3) * (un2 + ut * ut);
        //        double fn = un - ut + 0.5 * cGenEnv.gravity * dt_sec / dx * (dd_tp1 + dd_t) -cGenEnv .gravity * Sb * dt_sec + fn_friction;
        //        double dfn = 1 + cGenEnv.gravity * nc2 * dt_sec * un / Math.Pow(flowdepth, 4 / 3);
        //        double unp1 = un - fn / dfn;
        //        //double err = Math.Abs((qnp1 - qn) / qn);
        //        double du = Math.Abs(unp1 - un);
        //        double cC = Math.Abs(un * cGenEnv.convergenceConditionRatio);
        //        if (du <= cC)
        //        {
        //            flx.v = Math.Abs (unp1);
        //            flx.q = flx.v * flowdepth;
        //            flx.dflow = flowdepth;
        //            return flx;
        //        }
        //        un = unp1;
        //    }
        //    cGenEnv.writelog(string.Format("Momentum eq. was not converged. Time : {0}sec, (x, y) : ({1}, {2})  ", cGenEnv.tnow_sec, cx, ry));
        //    return flx;
        //}

        //private static cFluxData calFluxUsingMomentumEqNRasPositive6(double qini, double qt, 
        //    double dflow, double slp, double mN, double dx, int dt_sec, int cx, int ry)
        //{
        //    cFluxData flx = new cFluxData();
        //    double qn = qini; //qtp1의 초기값으로 사용
        //    double nc2 = mN * mN;
        //    double gdtnc2 = cGenEnv.gravity * dt_sec * nc2;
        //    for (int inr = 0; inr < 1000; inr++)
        //    {
        //        double qnp1 = qt - gdtnc2 * (qn * qn + qt * qt) / Math.Pow(dflow, (7 / 3));
        //        //double dfn = 1 + 0.5 * gdtnc2 * qn / Math.Pow(dflow, (7 / 3));
        //        //double qnp1 = qn - fn / dfn;
        //        if (Math.Abs (qn) < cGenEnv.dMinLimitforWet & Math.Abs(qnp1) < cGenEnv.dMinLimitforWet)
        //        {
        //            return setNoFlux();
        //        }
        //        double dq = Math.Abs(qnp1 - qn);
        //        double cC = Math.Abs(qn * cGenEnv.convergenceConditionRatio);
        //        if (dq < cC)
        //        {
        //            flx.q = Math.Abs(qnp1); ;
        //            flx.v = flx.q / dflow;
        //            flx.dflow = dflow;
        //            flx.slp = slp;
        //            return flx;
        //        }
        //        qn = qnp1;
        //    }
        //    cGenEnv.writelog(string.Format("Momentum eq. was not converged. Time : {0}sec, (x, y) : ({1}, {2})  ", cGenEnv.tnow_sec, cx, ry));
        //    return flx;
        //}




        #endregion
    }
}


