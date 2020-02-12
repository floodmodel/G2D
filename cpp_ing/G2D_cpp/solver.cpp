#include <stdio.h>
#include <iostream>
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
        nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel
        {
            int nrMax = 0;
#pragma omp for schedule(guided, nchunk) 
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
                    nrMax = calculateContinuityEqUsingNRforCPU(i, isBCcell, bcDepth, bctype);
                    if (cvs[i].dp_tp1 > gvi[0].dMinLimitforWet) {
                        setEffectiveCells(i);
                    }
                }
            }
#pragma omp critical
            {
                if (nrMax > psi.iNR) {
                    psi.iNR = nrMax;
                }
            }
        }
        psi.iGS = igs + 1;
        if (psi.bAllConvergedInThisGSiteration == 1) {
            break;
        }    
    }//여기까지 gs iteration    
    return 1;
}

int calculateContinuityEqUsingNRforCPU(int idx, int isBCCell, double dcdtpth, int bctype)
{
    //double sourceTerm = (cell.sourceAlltoRoute_tp1_dt_m + cell.sourceAlltoRoute_t_dt_m) / 2; //이건 Crank-Nicolson 방법.  dt는 이미 곱해서 있다..
    double dp_old = cvs[idx].dp_tp1;
    int inr = 0;
    for (inr = 0; inr < gvi[0].iNRmax; inr++) //cGenEnv.iNRmax_forCE 값 참조
    {
        if (NRinner(idx, isBCCell, dcdtpth, bctype) == 1) { break; }
    }
    cvs[idx].resd = abs(cvs[idx].dp_tp1 - dp_old);
    if (cvs[idx].resd > gvi[0].ConvgC_h) { 
        psi.bAllConvergedInThisGSiteration = -1; 
    }
    return inr + 1; // 현재셀의 nr을 반환해서, omp에서 reduction으로 최대값 찾게 한다.
}

int NRinner(int idx, int isBCCell, double dbdtpth, int bctype)
{
    double c1_IM = psi.dt_sec / gvi[0].dx;//이건 음해법
    double dn = cvs[idx].dp_tp1;
    calWFlux(idx, isBCCell);
    calEFlux(idx, isBCCell);
    calNFlux(idx, isBCCell);
    calSFlux(idx, isBCCell);
    // 현재 셀의 수위가 올라가려면  -> qe-, qw+, qs-, qn+
    double dnp1 = 0.0;
    double qw_tp1 = 0.0;
    double qn_tp1 = 0.0;
    //NR
    double fn = dn - cvs[idx].dp_t + (cvs[idx].qe_tp1 - cvs[idx].qw_tp1 + cvs[idx].qs_tp1 - cvs[idx].qn_tp1) * c1_IM;//- sourceTerm; //이건 음해법
    double eElem = pow(cvs[idx].dfe, 2 / 3.0) * sqrt(abs(cvs[idx].slpe)) / cvs[idx].rc;
    double sElem = pow(cvs[idx].dfs,  2 / 3.0) * sqrt(abs(cvs[idx].slps)) / cvs[idx].rc;
    double dfn = 1 + (eElem + sElem) * (5.0 / 3) * c1_IM;// 이건 음해법
    if (dfn == 0) { return 1; }
    dnp1 = dn - fn / dfn;
    if (isBCCell == 1 && bctype == 2) {// 1:Discharge, 2:Depth, 3:Height, 4:None
        dnp1 = dbdtpth;
    }
    if (dnp1 < 0) { dnp1 = 0; }
    double resd = dnp1 - dn;
    cvs[idx].dp_tp1 = dnp1;
    cvs[idx].hp_tp1 = cvs[idx].dp_tp1 + cvs[idx].elez;
    if (abs(resd) < gvi[0].ConvgC_h) {
        return 1;
    }
    return -1;
}


int runSolverUsingGPU()
{
    return 1;
}


void calWFlux(int idx, int isBCcell)
{
    if (gvi[0].nCols == 1) { return; }
    fluxData flxw; //W, x-
    if (cvs[idx].colx == 0 || cvs[idx].cvaryNum_atW == -1)//w 측 경계셀
    {
        if (isBCcell == 1) {
            flxw = noFlx(); // w측 최 경계에서는 w 방향으로 flx 없다.
        }
        else {// w측 최 경계에서는 w 방향으로 자유수면 flx 있다.
            double slp_tm1 = 0;
            if (cvs[idx].cvaryNum_atE >= 0)
            {
                double he = cvs[cvs[idx].cvaryNum_atE].dp_t + cvs[cvs[idx].cvaryNum_atE].elez;
                double hcur = cvs[idx].dp_t + cvs[idx].elez;
                slp_tm1 = (he - hcur) / gvi[0].dx; //i+1 셀과의 e 수면경사를 w 방향에 적용한다.
            }
            //double slp_tm1 = (cvs[cvs[idx].cvaryNum_atE].hp_t - cvs[idx].hp_t) / gv.dx; //i+1 셀과의 수면경사를 w 방향에 적용한다.
            slp_tm1 = slp_tm1 + gvi[0].domainOutBedSlope;
            if (slp_tm1 >= gvi[0].slpMinLimitforFlow && cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet)
            {
                flxw = calculateMomentumEQ_DWEm_Deterministric(cvs[idx].qw_t, gvi[0].gravity, psi.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1, 0);
            }
            else { flxw = noFlx(); }
        }
    }
    else {
        if (cvs[idx].isSimulatingCell == -1) {
            flxw = noFlx();
        }
        else {
            flxw.v = cvs[cvs[idx].cvaryNum_atW].ve_tp1;
            flxw.slp = cvs[cvs[idx].cvaryNum_atW].slpe;
            flxw.q = cvs[cvs[idx].cvaryNum_atW].qe_tp1;
            flxw.dflow = cvs[cvs[idx].cvaryNum_atW].dfe;
        }
    }
    cvs[idx].qw_tp1 = flxw.q;
}

void calEFlux(int idx, int isBCcell)
{
    if (gvi[0].nCols == 1) { return; }
    fluxData flxe;    //E,  x+
    if (cvs[idx].colx == (gvi[0].nCols - 1) || cvs[idx].cvaryNum_atE == -1)
    {
        if (isBCcell == 1) { flxe = noFlx(); }
        else {
            double slp_tm1 = 0;
            if (cvs[idx].cvaryNum_atW >= 0) {
                //double slp = (cell.hp_tp1 - dm.cells[cx - 1, ry].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                double hw = cvs[cvs[idx].cvaryNum_atW].dp_t + cvs[cvs[idx].cvaryNum_atW].elez;
                double hcur = cvs[idx].dp_t + cvs[idx].elez;
                slp_tm1 = (hcur - hw) / gvi[0].dx;
            }
            //double slp_tm1 = (cvs[idx].hp_t - cvs[cvs[idx].cvaryNum_atW].hp_t) / gv.dx;
            slp_tm1 = slp_tm1 - gvi[0].domainOutBedSlope;
            if (slp_tm1 <= (-1 * gvi[0].slpMinLimitforFlow) && cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet) {
                flxe = calculateMomentumEQ_DWEm_Deterministric(cvs[idx].qe_t, gvi[0].gravity, psi.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1, 0);
            }
            else { flxe = noFlx(); }
        }
    }
    else
    {
        if (cvs[idx].isSimulatingCell == -1) {
            flxe = noFlx();
        }
        else {
            flxe = getFluxToEastOrSouthUsing1DArray(cvs[idx], cvs[cvs[idx].cvaryNum_atE], 1);
        }
    }
    cvs[idx].ve_tp1 = flxe.v;
    cvs[idx].dfe = flxe.dflow;
    cvs[idx].slpe = flxe.slp;
    cvs[idx].qe_tp1 = flxe.q;
}

void calNFlux(int idx, int isBCcell)
{
    if (gvi[0].nRows == 1) { return; }
    fluxData flxn;  //N, y-
    if (cvs[idx].rowy == 0 || cvs[idx].cvaryNum_atN == -1)
    {
        if (isBCcell == 1) { flxn = noFlx(); }
        else
        {// n측 최 경계에서는 n 방향으로 자유수면 flx 있다.
            double slp_tm1 = 0;
            if (cvs[idx].cvaryNum_atS >= 0) {
                //double slp = (dm.cells[cx, ry + 1].hp_tp1 - cell.hp_tp1) / dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
                //double slp_tm1 = (cvs[cvs[idx].cvaryNum_atS].hp_t - cvs[idx].hp_t) / gv.dx; //j+1 셀과의 수면경사를 w 방향에 적용한다.
                double hs = cvs[cvs[idx].cvaryNum_atS].dp_t + cvs[cvs[idx].cvaryNum_atS].elez;
                double hcur = cvs[idx].dp_t + cvs[idx].elez;
                slp_tm1 = (hs - hcur) / gvi[0].dx;
            }
            slp_tm1 = slp_tm1 + gvi[0].domainOutBedSlope;
            if (slp_tm1 >= gvi[0].slpMinLimitforFlow && cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet)
            {
                //flxn = getFluxToDomainOut(cell, slp_tm1, cell.qn_t, cell.vn_t, gv.gravity, dt_sec);
                flxn = calculateMomentumEQ_DWEm_Deterministric(cvs[idx].qn_t, gvi[0].gravity, psi.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1, 0);
            }
            else { flxn = noFlx(); }
        }
    }
    else
    {
        if (cvs[idx].isSimulatingCell == -1) {
            flxn = noFlx();
        }
        else {
            flxn.v = cvs[cvs[idx].cvaryNum_atN].vs_tp1;
            flxn.slp = cvs[cvs[idx].cvaryNum_atN].slps;
            flxn.dflow = cvs[cvs[idx].cvaryNum_atN].dfs;
            flxn.q = cvs[cvs[idx].cvaryNum_atN].qs_tp1;
        }
    }
    cvs[idx].qn_tp1 = flxn.q;
}

void calSFlux(int idx, int isBCcell)
{
    if (gvi[0].nRows == 1) { return; }
    fluxData flxs;//S, y+
    if (cvs[idx].rowy == (gvi[0].nRows - 1) || cvs[idx].cvaryNum_atS == -1)
    {
        if (isBCcell == 1) { flxs = noFlx(); }
        else {
            double slp_tm1 = 0;
            if (cvs[idx].cvaryNum_atN >= 0) {
                //double slp = (cell.hp_tp1 - dm.cells[cx, ry - 1].hp_tp1) / dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                //double slp_tm1 = (cvs[idx].hp_t - cvs[cvs[idx].cvaryNum_atN].hp_t) / gv.dx; //i-1 셀과의 수면경사를 e 방향에 적용한다.
                double hn = cvs[cvs[idx].cvaryNum_atN].dp_t + cvs[cvs[idx].cvaryNum_atN].elez;
                double hcur = cvs[idx].dp_t + cvs[idx].elez;
                slp_tm1 = (hcur - hn) / gvi[0].dx;
            }
            slp_tm1 = slp_tm1 - gvi[0].domainOutBedSlope;
            if (slp_tm1 <= (-1 * gvi[0].slpMinLimitforFlow) && cvs[idx].dp_tp1 > gvi[0].dMinLimitforWet)
            {
                //flxs = getFluxToDomainOut(cell, slp_tm1, cell.qs_t, cell.vs_t, gv.gravity, dt_sec);
                flxs = calculateMomentumEQ_DWEm_Deterministric(cvs[idx].qs_t, gvi[0].gravity, psi.dt_sec, slp_tm1, cvs[idx].rc, cvs[idx].dp_tp1, 0);
            }
            else { flxs = noFlx(); }
        }
    }
    else {
        if (cvs[idx].isSimulatingCell == -1) {
            flxs = noFlx();
        }
        else {
            flxs = getFluxToEastOrSouthUsing1DArray(cvs[idx], cvs[cvs[idx].cvaryNum_atS], 3);
        }
    }
    cvs[idx].vs_tp1 = flxs.v;
    cvs[idx].dfs = flxs.dflow;
    cvs[idx].slps = flxs.slp;
    cvs[idx].qs_tp1 = flxs.q;
}


 fluxData calculateMomentumEQ_DWEm_Deterministric
 (double qt, double gravity, double dt_sec, double slp, double rc, double dflow, double qt_ip1)
 {
     fluxData flx ;
     double qapp = qt; 
     //double q = (qapp - (gravity * dflow * dt_sec * slp)) /
     //                           (1 + gravity * dt_sec * (rc * rc) * DeviceFunction.Sqrt((qapp * qapp + qt_ip1 * qt_ip1) / 2) / DeviceFunction.Pow(dflow, (double)7 / 3));
     double q = (qapp - (gravity * dflow * dt_sec * slp)) /
         (1 + gravity * dt_sec * (rc * rc) * abs(qapp) / pow(dflow, 7.0 / 3.0));
     flx.q = q;
     flx.v = flx.q / dflow;  // Manning 결과와 같다. flx.v = Math.Pow(dflow, 2 / 3) * Math.Abs(slp) / mN; 
     flx.dflow = dflow;
     flx.slp = slp;
     return flx; ;
 }

 fluxData calculateMomentumEQ_DWE_Deterministric(double qt, double dflow,
     double slp, double gravity, double rc, double dx, double dt_sec, double q_ip1, double u_ip1)
 {
     // 이거 잘 안된다. 반복법이 필요.. 2018.12.26.
     fluxData flx ;
     double qapp = qt; //Math.Abs(qt);
     //2019.1.2 관성이 없을 경우에는 
     // slp가 + 면 q는 -, slp가 - 이면 q는 + 가 되어야 함.
     // 이전 t에서 q 가  0 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 무조건 +.
     // 이전 t에서 q 가  - 이면, slp가 + 일때 무조건 q는 - , slp가 - 일때는 q는 - 일수도 있고, + 일수도 있음. => 조건 처리 필요
     // 이전 t에서 q 가 + 이면, slp가 + 일때 q는 - 일수도 있고, + 일수도 있음, slp가 - 일때는 q는 무조건 +. => 조건 처리 필요
 
     double ut = qapp / dflow;
     double q = (qapp - (gravity * dflow * dt_sec * slp)) /
         (1 + ut * dt_sec / dx + gravity * dt_sec * (rc * rc) * abs(qapp) / pow(dflow, 7.0 / 3.0));
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


 fluxData getFluxToEastOrSouthUsing1DArray(cvatt curCell,
     cvatt tarCell, int targetCellDir)
 {
     double slp = 0;
     //double dht = (tarCell.elez+tarCell .dp_t)-(curCell.elez+curCell.dp_t); //+면 자신의 셀이, 대상 셀보다 낮다, q는 -, slp는 +.   -면 자신의 셀이, 대상 셀보다 높다, q는 +, slp는 - 
     double dhtp1 = tarCell.hp_tp1 - curCell.hp_tp1;
     if (dhtp1 == 0) { return noFlx(); }
     if (dhtp1 > 0 && tarCell.dp_tp1 <= gvi[0].dMinLimitforWet) { return noFlx(); }
     if (dhtp1 < 0 && curCell.dp_tp1 <= gvi[0].dMinLimitforWet) { return noFlx(); }
     slp = dhtp1 / gvi[0].dx;
     if (abs(slp) < gvi[0].slpMinLimitforFlow || abs(slp) == 0) { return noFlx(); }
     double dflow = max(curCell.hp_tp1, tarCell.hp_tp1) - max(curCell.elez, tarCell.elez);
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
     if (targetCellDir == 1) {
         qt = curCell.qe_t;
         qtp1 = curCell.qe_tp1; // qtp1
         u_ip1 = tarCell.ve_tp1; q_ip1 = tarCell.qe_tp1;
     }
     else if (targetCellDir == 3) {
         qt = curCell.qs_t;
         qtp1 = curCell.qs_tp1;
         u_ip1 = tarCell.vs_tp1; q_ip1 = tarCell.qs_tp1;
     }

     fluxData flx;
     if (gvi[0].isDWE == 1) {
         //flx = calFluxUsingME_DWE_Implicit_UsingGPU(dhtp1, qt, qtp1, dflow, currentCell.rc, dx, dt_sec);
         flx = calculateMomentumEQ_DWE_Deterministric(qt, dflow, slp, gvi[0].gravity, curCell.rc, gvi[0].dx, psi.dt_sec, q_ip1, u_ip1);
     }
     else {
         //flx = calFluxUsingME_mDWE_Implicit(dhtp1, dht,
         //       qt, qtp1, dflow, currentCell.lc.roughnessCoeff, dx, dt_sec, currentCell.colxary, currentCell.rowyary);
         flx = calculateMomentumEQ_DWEm_Deterministric(qt, gvi[0].gravity, psi.dt_sec, slp, curCell.rc, dflow, q_ip1);
     }

     if (gvi[0].isAnalyticSolution == -1) {
         if (abs(flx.q) > 0) {
             flx = getFluxUsingSubCriticalCon(flx, gvi[0].gravity, gvi[0].froudeNCriteria);
             flx = getFluxUsingFluxLimitBetweenTwoCell(flx, dflow, gvi[0].dx, psi.dt_sec);
             //flx = getFluxqUsingFourDirLimitUsingDepthCondition(currentCell, flx, dflow, dx, dt_sec); //이건 수렴이 잘 안된다.
             //flx = getFluxUsingFourDirLimitUsingCellDepth(currentCell, targetCell, flx, dx, dt_sec);
             //flx = getFluxUsingFourDirLimitUsingDh(flx, dhtp1, dx, dt_sec); // 이건 소스에서 수심이 급격히 올라간다.
         }
     }
     flx.slp = slp;
     return flx;
 }

 fluxData getFluxUsingSubCriticalCon(fluxData inflx, double gravity, double froudNCriteria)
 {
     double v_wave = sqrt(gravity * inflx.dflow);
     double fn = abs(inflx.v) / v_wave;
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

 fluxData getFluxUsingFluxLimitBetweenTwoCell(fluxData inflx, double dflow, double dx, double dt_sec)
 {
     double qmax = abs(dflow) * dx / 2 / dt_sec; // 수위차의 1/2 이 아니라, 흐름 수심의 1/2이므로, 수위 역전 될 수 있다.
     double qbak = inflx.q;
     if (abs(inflx.q) > qmax) {
         inflx.q = qmax;
         if (qbak < 0) { inflx.q = -1 * qmax; }
         inflx.v = inflx.q / inflx.dflow;
     }
     return inflx;
 }


 fluxData noFlx()
 {
     fluxData flx;
     flx.dflow = 0;
     flx.fd = 0;
     flx.q = 0;
     flx.slp = 0;
     flx.v = 0;
     return flx;
 }
