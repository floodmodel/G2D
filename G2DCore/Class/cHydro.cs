using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Alea;

namespace G2DCore
{
    public class cHydro
    {
        public static double cflNumber;
        public static double froudeNCriteria;
        public static int applyVNC = -1;
        public static double roughness;
        public static double domainOutBedSlope;
        public static double imperviousR;
        public cVars.ConditionDataType icDataType;
        public cVars.InitialConditionType icType;
        public double icValue_meter;
        public string fpn_ic;

        public cHydro()
        {
        }
        
        public void SetValues(cProject prj)
        {
            Dataset.projectds.HydroParsRow row =
                (Dataset.projectds.HydroParsRow)prj.prjds.HydroPars.Rows[0];
            if (prj.LCType != cVars.LandCoverType.File)
            {
                roughness = row.RoughnessCoeff;
                imperviousR = 1;
            }
            else
            {
                roughness = -9999;
                imperviousR = 1;
            }
            icDataType = cVars.ConditionDataType.None;
            if (row.IsInitialConditionTypeNull()==false)
            {
                if (row.InitialConditionType.ToString().ToLower() == "depth")
                { icDataType = cVars.ConditionDataType.Depth; }
                if (row.InitialConditionType.ToString().ToLower() == "height")
                { icDataType = cVars.ConditionDataType.Height; }
            }
            fpn_ic = "";
            icValue_meter = 0;
            icType = cVars.InitialConditionType.Constant;
            if (row.IsInitialConditionNull() == false )
            {
                double v = 0;
                if (double.TryParse(row.InitialCondition, out v)==true)
                {
                    icValue_meter = v;
                }
                else if(File.Exists(row.InitialCondition )==true )
                {
                    fpn_ic = row.InitialCondition;
                    icType = cVars.InitialConditionType.File;
                }                
            }
            froudeNCriteria = row.FroudeNumberCriteria;
            cflNumber = row.CourantNumber;
            if( row.IsApplyVNCNull()==true )
            {
                applyVNC = -1;
            }
            else
            {
                if (row.ApplyVNC == true)
                { applyVNC = 1; }
                else
                {
                    applyVNC = -1;
                }
            }
            if(row.IsDomainOutBedSlopeNull()== true)
            {
                domainOutBedSlope = 0;
            }
            else
            {
                domainOutBedSlope = row.DomainOutBedSlope;
            }
        }

        [GpuManaged]
        public static stFluxData getFD4MaxUsingGPU(G2DCore.stCVAtt cell, stCVAtt wcell, stCVAtt ncell)
        {
            stFluxData flxmax= new G2DCore.stFluxData();
            double vw = DeviceFunction.Abs(wcell.ve_tp1);
            double ve = DeviceFunction.Abs(cell.ve_tp1);
            double vn = DeviceFunction.Abs(ncell.vs_tp1);
            double vs = DeviceFunction.Abs(cell.vs_tp1);
            double vmaxX = DeviceFunction.Max(vw, ve);
            double vmaxY = DeviceFunction.Max(vn, vs);
            double vmax = DeviceFunction.Max(vmaxX, vmaxY);
            if (vmax == 0)
            {
                flxmax.fd = 0;// cVars.FlowDirection4.NONE;
                flxmax.v = 0;
                flxmax.dflow = 0;
                flxmax.q = 0;
                return flxmax;
            }
            flxmax.v = vmax;//E = 1, S = 3, W = 5, N = 7, NONE = 0
            if (vmax == vw) { flxmax.fd = 5; }// cVars.FlowDirection4.W; }
            if (vmax == ve) { flxmax.fd = 1; }// cVars.FlowDirection4.E; }
            if (vmax == vn) { flxmax.fd = 7; }// cVars.FlowDirection4.N; }
            if (vmax == vs) { flxmax.fd = 3; }// cVars.FlowDirection4.S; }
            double dmaxX = DeviceFunction.Max(wcell.dfe, cell.dfe);
            double dmaxY = DeviceFunction.Max(ncell.dfs, cell.dfs);
            flxmax.dflow = DeviceFunction.Max(dmaxX, dmaxY);
            double qmaxX = DeviceFunction.Max(DeviceFunction.Abs(cell.qw_tp1), DeviceFunction.Abs(cell.qe_tp1));
            double qmaxY = DeviceFunction.Max(DeviceFunction.Abs(cell.qn_tp1), DeviceFunction.Abs(cell.qs_tp1));
            flxmax.q = DeviceFunction.Max(qmaxX, qmaxY);
            return flxmax;
        }

        public static stFluxData getFD4MaxUsingCPU(G2DCore.stCVAtt cell, stCVAtt wcell, stCVAtt ncell)
        {
            stFluxData flxmax = new G2DCore.stFluxData();
            double vw = Math.Abs(wcell.ve_tp1);
            double ve = Math.Abs(cell.ve_tp1);
            double vn = Math.Abs(ncell.vs_tp1);
            double vs = Math.Abs(cell.vs_tp1);
            double vmaxX = Math.Max(vw, ve);
            double vmaxY = Math.Max(vn, vs);
            double vmax = Math.Max(vmaxX, vmaxY);
            if (vmax == 0)
            {
                flxmax.fd = 0;// cVars.FlowDirection4.NONE;
                flxmax.v = 0;
                flxmax.dflow = 0;
                flxmax.q = 0;
                return flxmax;
            }
            flxmax.v = vmax;//E = 1, S = 3, W = 5, N = 7, NONE = 0
            if (vmax == vw) { flxmax.fd = 5; }// cVars.FlowDirection4.W; }
            if (vmax == ve) { flxmax.fd = 1; }// cVars.FlowDirection4.E; }
            if (vmax == vn) { flxmax.fd = 7; }// cVars.FlowDirection4.N; }
            if (vmax == vs) { flxmax.fd = 3; }// cVars.FlowDirection4.S; }

            double dmaxX = Math.Max(wcell.dfe, cell.dfe);
            double dmaxY = Math.Max(ncell.dfs, cell.dfs);
            flxmax.dflow = Math.Max(dmaxX, dmaxY);

            double qmaxX = Math.Max(Math.Abs(cell.qw_tp1), Math.Abs(cell.qe_tp1));
            double qmaxY = Math.Max(Math.Abs(cell.qn_tp1), Math.Abs(cell.qs_tp1));
            flxmax.q = Math.Max(qmaxX, qmaxY);
            return flxmax;
        }

        public static double getDTsecUsingConstraints(stBCinfo[] bcinfos, stCVAtt[] cvs, stCVAtt_add[] cvsadd, double tnow_sec, double dtcur_sec, double cfln,
            double dx, double gravity, double dflowmax, double vMax, double vonNeumanCon,
            double half_dtPrint_sec, double half_bcdt_sec, double half_rfdt_sec, int appVNC)
        {
            double dtsecCFL = 0;
            double dtsecCFLusingDepth = 0;
            double dtsecCFLusingV = 0;
            //==================================
            //이건 cfl 조건
            if (dflowmax > 0)
            {
                dtsecCFLusingDepth = cfln * dx / Math.Sqrt(gravity * dflowmax);
                //  아래  것과 결과에 별 차이 없다..
                //   dtsecCFL = cfln * dm.dx / Math.Sqrt(gravity * depthMax);
                dtsecCFL = dtsecCFLusingDepth;
            }
            if (vMax > 0)
            {
                dtsecCFLusingV = cfln * dx / vMax;
                dtsecCFL = dtsecCFLusingV;
            }

            if (dtsecCFLusingDepth > 0 && dtsecCFLusingV > 0)
            {
                dtsecCFL = Math.Min(dtsecCFLusingDepth, dtsecCFLusingV);
            }
            //==================================

            //==================================
            //이건 Von Neuman 안정성 조건
            double dtsecVN = 0;
            if (appVNC == 1)
            { dtsecVN = (vonNeumanCon * dx * dx) / 4; }

            double dtsec=0;
            if (dtsecVN > 0 && dtsecCFL > 0) { dtsec = Math.Min(dtsecCFL, dtsecVN); }
            else { dtsec = Math.Max(dtsecCFL, dtsecVN); }
            if (dtsec > half_dtPrint_sec) { dtsec = half_dtPrint_sec; }
            if (half_bcdt_sec > 0 && dtsec > half_bcdt_sec) { dtsec = half_bcdt_sec; }
            if (half_rfdt_sec > 0 && dtsec > half_rfdt_sec) { dtsec = half_rfdt_sec; }

            if (dtsec == 0)
            {
                dtsec = dtcur_sec*1.5;
                if (dtsec>cGenEnv.dtMaxLimit_sec) { dtsec = cGenEnv.dtMaxLimit_sec; }
            }

            double maxSourceDepth = 0;
            double dtsecCFLusingBC = 0;
            int bcdt_sec = cProject.Current.BConditionInterval_min * 60;
            for (int ib =0; ib<bcinfos.Length;ib++)
            {
                double bcDepth_dt_m_tp1 = 0;
                int cvidx = bcinfos[ib].cvid;
               bcDepth_dt_m_tp1 = cSimulationSetting.getConditionDataAsDepthWithLinear(bcinfos[ib].bctype, cvs[cvidx].elez, cvsadd[cvidx], dx, dtsec, bcdt_sec, tnow_sec);
              if (bcDepth_dt_m_tp1 > maxSourceDepth) { maxSourceDepth = bcDepth_dt_m_tp1; }
            }

            if (maxSourceDepth > 0)
            {
                dtsecCFLusingBC = cfln * dx / Math.Sqrt(gravity * (maxSourceDepth + dflowmax));
                if (dtsecCFLusingBC < dtsec) { dtsec = dtsecCFLusingBC; }
            }

            if (dtsec < cGenEnv.dtMinLimit_sec) { dtsec = cGenEnv.dtMinLimit_sec; }
            if (dtsec > cGenEnv.dtMaxLimit_sec) { dtsec = cGenEnv.dtMaxLimit_sec; }

            double realpart_t = tnow_sec - Math.Truncate(tnow_sec);
            if (dtsec > 5)
            {
                dtsec = Math.Truncate(dtsec); //여기서 부터는 dtsec 정수
                dtsec = dtsec - realpart_t;  // 여기서 부터는 t 정수
            }
            return dtsec;
        }
    }
}
