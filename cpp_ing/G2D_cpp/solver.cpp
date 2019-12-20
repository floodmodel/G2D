#include <stdio.h>
#include <math.h>
#include <omp.h>
#include "g2d.h"

using namespace std;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern vector<rainfallinfo> rf;
extern bcCellinfo* bci;

extern thisProcessInner psi;
extern globalVinner gvi[1];

int runSolverUsingCPU()
{
    int igs = 0;
    for (igs = 0; igs < gvi[0].iGSmax; igs++)
    {
        psi.bAllConvergedInThisGSiteration = 1;
        psi.iNR = 0;
        int nchunk;
            omp_set_num_threads(gvi[0].mdp);
            nchunk = gvi[0].nRows / gvi[0].mdp;
#pragma omp parallel for schedule(guided, nchunk) 
        for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
            if (cvs[i].isSimulatingCell == 1) {
                int bcCellidx = getbcCellArrayIndex(i);
                int isBCcell = -1;
                double bcDepth = 0;
                int bctype = 0;
                if (bcCellidx >= 0) {
                    isBCcell = 1;
                    bcDepth = bci[bcCellidx].bcDepth_dt_m_tp1;
                    bctype = bci[bcCellidx].bctype;
                }
                calculateContinuityEqUsingNRUsing1DArrayForCPU(cvs, i, gvi, isBCcell, bcDepth, bctype);
                if (cvs[i].dp_tp1 > gvi[0].dMinLimitforWet) {
                    cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i);
                }
            }
        }
        psi.iGS = igs + 1;
        if (psi.bAllConvergedInThisGSiteration == 1) {
            break;
        }
        //여기까지 gs iteration

        updateValuesInThisStepResults();
        //if (gv[0].bAllConvergedInThisGSiteration == 1) {
        //    if (cGenEnv.bwritelog_process == true) {
        //        cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
        //            cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
        //    }
        //}
        //else {
        //    if (cGenEnv.bwritelog_process == true) {
        //        cGenEnv.writelog(String.Format("Time : {0}sec. GS iteration was not converged for all cell in this time step. Con. eq. NR max iteration: {2}. dt: {3}",
        //            cGenEnv.tnow_sec, igs, cThisProcess.maxNR_inME, cGenEnv.dt_sec), cGenEnv.bwritelog_process);
        //    }
        //}
    }
    return 1;
}



void updateValuesInThisStepResults()
{    
    psi.dflowmaxInThisStep = -9999;
    psi.vmaxInThisStep = -9999;
    psi.VNConMinInThisStep = DBL_MAX;
    psi.maxResd = 0;
#pragma omp parallel
    {
        double maxdflow = 0;
        double maxv = 0;
        double minvnc = 9999;
        cellResidual maxRes;
        maxRes.residual = 0.0;
#pragma omp for nowait
        for (int idx = 0; idx < gvi[0].nCellsInnerDomain; ++idx)
        {
            if (cvs[idx].isSimulatingCell == 1)
            {
                fluxData flxmax;
                if (cvs[idx].cvaryNum_atW >= 0 && cvs[idx].cvaryNum_atN >= 0) {//  이경우는 4개 방향 성분에서 max 값 얻고
                    flxmax = cHydro.getFD4MaxUsingGPU(cvs[idx],
                        cvs[cvs[idx].cvaryNum_atW], cvs[cvs[idx].cvaryNum_atN]);
                }
                else {// 이경우는 s, e에서 max 값 얻는다
                    flxmax = cHydro.getFD4MaxUsingGPU(cvs[idx], cvs[idx], cvs[idx]);
                }
                cvsAA[idx].fdmax = flxmax.fd;
                cvsAA[idx].vmax = flxmax.v;
                cvsAA[idx].Qmax_cms = flxmax.q * gvi[0].dx;
                if (flxmax.dflow > maxdflow) { 
                    maxdflow = flxmax.dflow; 
                }
                if (cvsAA[idx].vmax > maxv) {
                    maxv = cvsAA[idx].vmax;
                }
                double vnCon = 0;
                if (cHydro.applyVNC == 1) {
                    vnCon = getVonNeumanConditionValueUsingGPUFunction(cvs[idx]);
                }
                if (vnCon < minvnc) {
                    minvnc = vnCon;
                }
                if (cvs[idx].resd > maxRes.residual) {
                    maxRes.residual = cvs[idx].resd;
                    maxRes.cvid = idx;
                }
            }
        }

#pragma omp critical
        {
            if (psi.dflowmaxInThisStep < maxdflow) {
                psi.dflowmaxInThisStep = maxdflow;
            }
            if (psi.vmaxInThisStep < maxv) {
                psi.vmaxInThisStep = maxv;
            }
            if (psi.VNConMinInThisStep > minvnc) {
                psi.VNConMinInThisStep = minvnc;
            }
            if (psi.maxResd < maxRes.residual) {
                psi.maxResd = maxRes.residual;
                psi.maxResdCVID = maxRes.cvid;
            }
        }
    }
}



int runSolverUsingGPU()
{

    return 1;
}
