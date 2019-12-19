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
        if (gvi[0].isparallel > 0) {
            omp_set_num_threads(gvi[0].mdp);
            nchunk = gvi[0].nRows / gvi[0].mdp;
        }
#pragma omp parallel for schedule(guided, nchunk) if (gvi[0].isparallel)
            for (int i = 0; i < gvi[0].cellCountInnerDomain; ++i) {
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
                    calculateContinuityEqUsingNRUsing1DArrayForCPU(cvs, i, gv, isBCcell, bcDepth, bctype);
                    if (cvs[i].dp_tp1 > cGenEnv.dMinLimitforWet) { 
                        cSimulationSetting.setEffectiveCellUsing1DArray(cvs, i); 
                    }
                }
            }
            psi.iGS = igs + 1;
            if (psi.bAllConvergedInThisGSiteration == 1) {
                break;
            }
        //여기까지 gs iteration

        UpdateValuesInThisStepResultsUsing1DArray(cvs, cvsadd);
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


int runSolverUsingGPU()
{

    return 1;
}
