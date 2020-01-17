using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using gentle;
using System.Drawing;

namespace G2DCore
{
   public class cOutput
    {
        private cProject mprj;
        private string mprjPath = "";
        private string mprjNameWithoutExt = "";
        private string mprjFPN = "";
        public string fpnDischargeMaxPre="";
        public string fpnDepthPre = "";
        public string fpnHeightPre = "";
        public string fpnVelocityMaxPre = "";
        public string fpnFDofMaxVelocityPre = "";
        public string fpnBcDataPre = "";
        public string fpnRFGridPre = "";
        public string fpnSinkPre = "";
        public string fpnSourceAllPre = "";

        private string fpnDepthAsc = "";
        private string fpnDepthImg = "";
        private string fpnHeightAsc = "";
        private string fpnHeightImg = "";
        private string fpnDischargeMaxAsc = "";
        private string fpnDischargeMaxImg = "";
        private string fpnVelocityMaxAsc = "";
        private string fpnVelocityMaxImg = "";
        private string fpnFDofMaxVelocityAsc = "";
        private string fpnFDofMaxVelocityImg = "";
        private string fpnRFGridAsc = "";
        private string fpnRFGridImg = "";
        private string mascHeaderAll = "";
        private int mascNodataValue = -9999;
        private int mnCx;
        private int mnRy;

        private double mNullValue;
        private bool mboDischarge = false;
        private bool mboDepth = false;
        private bool mboHeight = false;
        private bool mboVelocityMax = false;
        private bool mboFDofMaxVelocity = false;
        private bool mboRFGrid = false;
        private double mRenMaxVDischarge = 0;
        private double mRenMaxVDepth = 0;
        private double mRenMaxVHeight = 0;
        private double mRenMaxVvelocityMax = 0;
        private double mRenMaxVrf = 0;

        private double[,] mDischargeAry;
        private double[,] mDepthAry;
        private double[,] mHeightAry;
        private double[,] mVelocityMaxAry;
        private double[,] mFDofMaxVelocityAry;
        private double[,] mRFGridAry_m;

        //삭제 대상
        private double[,] mDepthOnlyAry;

        public cOutput(cProject prj) 
            {
            Dataset.projectds.ProjectSettingsRow row =
                    (G2DCore.Dataset.projectds.ProjectSettingsRow)
                    prj.prjds.ProjectSettings.Rows[0];
            mprj = prj;
            mprjPath = prj.prjFilePath;
            mprjNameWithoutExt = Path.GetFileNameWithoutExtension(prj.prjFileName);
            mprjFPN = Path.Combine(prj.prjFilePath, prj.prjFileName);

            if (row.IsOutputDischargeMaxNull()==false && row.OutputDischargeMax.ToLower()=="true")
            {
                mboDischarge = true;
                if (cGenEnv .bMakeImgFile ==true && row.IsDischargeImgRendererMaxVNull() == false 
                    && row.DischargeImgRendererMaxV.ToString ().Trim () !="")
                {
                    mRenMaxVDischarge = row.DischargeImgRendererMaxV;
                }
                else
                {
                    mRenMaxVDepth = 10000;
                }
            }
            else
            { mboDischarge = false; }

            if (row.IsOutputDepthNull() == false && row.OutputDepth.ToLower() == "true")
            {
                mboDepth = true;
                if (cGenEnv.bMakeImgFile == true&&row.IsDepthImgRendererMaxVNull ()==false 
                    && row.DepthImgRendererMaxV.ToString().Trim() != "")
                {
                    mRenMaxVDepth = row.DepthImgRendererMaxV;
                }
                else
                {
                    mRenMaxVDepth = 3;
                }

            }
            else
            { mboDepth = false; }

            if (row.IsOutputHeightNull() == false && row.OutputHeight.ToLower() == "true")
            {
                mboHeight = true;
                if (cGenEnv.bMakeImgFile == true && row.IsHeightImgRendererMaxVNull () == false 
                    && row.HeightImgRendererMaxV.ToString().Trim() != "")
                {
                    mRenMaxVHeight = row.HeightImgRendererMaxV ;
                }
                else
                {
                    mRenMaxVHeight = 200;
                }
            }
            else
            { mboHeight = false; }

            if (row.IsOutputVelocityMaxNull() == false && row.OutputVelocityMax.ToLower() == "true")
            {
                mboVelocityMax = true;
                if (cGenEnv.bMakeImgFile == true && row.IsVelocityMaxImgRendererMaxVNull () == false 
                    && row.VelocityMaxImgRendererMaxV.ToString().Trim() != "")
                {
                    mRenMaxVvelocityMax  = row.VelocityMaxImgRendererMaxV ;
                }
                else
                {
                    mRenMaxVvelocityMax = 10;
                }
            }
            else
            { mboVelocityMax = false; }

            if (row.IsOutputFDofMaxVNull() == false && row.OutputFDofMaxV.ToLower() == "true")
            { mboFDofMaxVelocity = true; }
            else
            { mboFDofMaxVelocity = false; }

            if (row.IsOutputRFGridNull() == false && row.OutputRFGrid.ToLower() == "true")
            {
                mboRFGrid = true;
                if (cGenEnv.bMakeImgFile == true && row.IsRFImgRendererMaxVNull() == false 
                    && row.RFImgRendererMaxV .ToString().Trim() != "")
                {
                    mRenMaxVrf  = row.RFImgRendererMaxV;
                }
                else
                {
                    mRenMaxVrf = 30;
                }
            }
            else
            { mboRFGrid = false; }

            mascHeaderAll = prj.domain.headerStringAll;
            mascNodataValue = prj.domain.nodata_value; 
            mnCx = prj.domain.nCols;
            mnRy = prj.domain.nRows;

            if (mboDepth == true)
            {
                fpnDepthPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_DEPTH);
            }
            if (mboHeight == true)
            {
                fpnHeightPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_HEIGHT);
            }
            if (mboDischarge == true)
            {
                fpnDischargeMaxPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_DISCHARGE);
            }
            if (mboVelocityMax == true)
            {
                fpnVelocityMaxPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_VELOCITY);
            }
            if (mboFDofMaxVelocity == true)
            {
                fpnFDofMaxVelocityPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_FLOWDIRECTION);
            }
            if (mboRFGrid == true)
            {
                fpnRFGridPre = Path.Combine(mprjPath, mprjNameWithoutExt + cVars.CONST_FILENAME_TAG_RFGRID);
            }
        }

        public bool deleteAlloutputFilesExisted()
        {
            try
            { 
                //모의 시작 할때, 다 지우고 새로 만든다
                List<string> fpns = new List<string>();
                string[] allfpn = System.IO.Directory.GetFiles(mprjPath);
                for (int nf = 0; nf < allfpn.Length; nf++)
                {
                    if (allfpn[nf].Contains(cVars.CONST_OUTPUT_ASCFILE_EXTENSION) || allfpn[nf].Contains(cVars.CONST_OUTPUT_IMGFILE_EXTENSION)
                        || allfpn[nf].Contains(cVars.CONST_OUTPUT_PROJECTIONFILE_EXTENSION))
                    {
                        if (allfpn[nf].Contains(mprjNameWithoutExt + "_Discharge") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_Depth") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_Height") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_Velocity") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_FDirection") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_RFGrid") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_BC") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_SourceAll") ||
                            allfpn[nf].Contains(mprjNameWithoutExt + "_Sink"))
                        {
                            fpns.Add(allfpn[nf]);
                        }
                    }
                }
                if (fpns.Count > 0)
                {
                    cGenEnv.writelog("Delete all .out files...", cGenEnv.bwritelog_process);
                    if (gentle.cFile.ConfirmDeleteFiles(fpns) == false)
                    {
                        cGenEnv.modelSetupIsNormal = false;
                        cGenEnv.writelogNconsole("Some output files were not deleted. Initializing new output files was failed.", cGenEnv.bwritelog_process);
                        return false;
                    }
                    cGenEnv.writelog("Delete all .out files... completed.", cGenEnv.bwritelog_process);
                }
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                return false;
            }
        }



        private void StartMakeASCTextFileDepth()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerDepth);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerDepth()
        {
            cTextFile.MakeASCTextFile(fpnDepthAsc, mascHeaderAll, mDepthAry, 5, mascNodataValue);
        }

        private void StartMakeImgFileDepth()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerDepth);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerDepth()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            Bitmap bm;
            bm = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(
                fpnDepthImg, mDepthAry, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT, cImg.RendererIntervalType.Equalinterval,
                  cImg.RendererRange.MinMax, 0, mRenMaxVDepth, mNullValue);
        }

        private void StartMakeASCTextFileHeight()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerHeight);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerHeight()
        {
            cTextFile.MakeASCTextFile(fpnHeightAsc, mascHeaderAll, mHeightAry, 5, mascNodataValue);
        }

        private void StartMakeImgFileHeight()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerHeight);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerHeight()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(
               fpnHeightImg, mHeightAry, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT, cImg.RendererIntervalType.Equalinterval,
                  cImg.RendererRange.MinMax, 0, mRenMaxVHeight, mNullValue);
        }

        private void StartMakeASCTextFileDischargeMax()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerDischargeMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerDischargeMax()
        {
            cTextFile.MakeASCTextFile(fpnDischargeMaxAsc, mascHeaderAll, mDischargeAry, 3, mascNodataValue);
        }

        private void StartMakeImgFileDischargeMax()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerDischargeMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerDischargeMax()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel
                (fpnDischargeMaxImg, mDischargeAry, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT,
                cImg.RendererIntervalType.Equalinterval, cImg.RendererRange.MinMax, 0, mRenMaxVDischarge, mNullValue);
        }
        
        private void StartMakeASCTextFileVelocityMax()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerVelocityMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerVelocityMax()
        {
            cTextFile.MakeASCTextFile(fpnVelocityMaxAsc, mascHeaderAll, mVelocityMaxAry, 3, mascNodataValue);
        }

        private void StartMakeImgFileVelocityMax()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerVelocityMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerVelocityMax()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel
                    (fpnVelocityMaxImg, mVelocityMaxAry, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT,
                    cImg.RendererIntervalType.Equalinterval, cImg.RendererRange.MinMax, 0, mRenMaxVvelocityMax, mNullValue);
        }

        private void StartMakeASCTextFileFDofVMax()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerFDofVMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerFDofVMax()
        {
            cTextFile.MakeASCTextFile(fpnFDofMaxVelocityAsc , mascHeaderAll, mFDofMaxVelocityAry , 0, mascNodataValue);
        }

        private void StartMakeImgFileFDofVMax()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerFDofVMax);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerFDofVMax()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel
                    (fpnFDofMaxVelocityImg, mFDofMaxVelocityAry, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT,
                    cImg.RendererIntervalType.Equalinterval, cImg.RendererRange.MinMax, 0, 8, mNullValue);
        }
                                    
        private void StartMakeASCTextFileRFGrid()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerRFGrid);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeASCTextFileInnerRFGrid()
        {
            cTextFile.MakeASCTextFile(fpnRFGridAsc, mascHeaderAll, mRFGridAry_m,3, mascNodataValue);
        }

        private void StartMakeImgFileRFGrid()
        {
            ThreadStart ts = new ThreadStart(MakeImgFileInnerRFGrid);
            Thread th = new Thread(ts);
            th.Start();
        }
        private void MakeImgFileInnerRFGrid()
        {
            cImg imgMaker = new gentle.cImg(gentle.cImg.RendererType.WaterDepth);
            imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel
                    (fpnRFGridImg, mRFGridAry_m, cVars.CONST_IMG_WIDTH, cVars.CONST_IMG_HEIGHT, cImg.RendererIntervalType.Equalinterval,
                  cImg.RendererRange.MinMax, 0, mRenMaxVrf, mNullValue);
        }

        public bool makeOutputFilesUsing1DArray(stCVAtt[] cvs, stCVAtt_add[] cvsadd, double nowTsec, double printInterval_min, double nullvalue)
        {
            string printT_min = "";
            double printMIN = 0;
            mNullValue = nullvalue;
            //printMIN = ((double)(nowTsec / 60)).ToString("F1");
            printMIN = nowTsec / 60;
            if (cGenEnv.isDateTimeFormat == true)
            {
                printT_min = gentle.cComTools.GetTimeToPrintOut(cGenEnv.isDateTimeFormat, cGenEnv.eventStartTime, (int)nowTsec / 60, "yyyyMMddHHmm");
            }
            else
            {
                if (printInterval_min < 60)
                {
                    string printM = printMIN.ToString("F2");
                    printT_min = printM + "m";
                }
                else if (printInterval_min >= 60 && printInterval_min < 1440)
                {
                    string printH = ((double)(printMIN / 60)).ToString("F2");
                    printT_min = printH + "h";
                }
                else if (printInterval_min >= 1440)
                {
                    string printD = ((double)(printMIN / 1440)).ToString("F2");
                    printT_min = printD + "d";
                }
            }
            cGenEnv.writelog("Making output file at the time " + printT_min, cGenEnv.bwritelog_process);
            string printT_min_oriString = printT_min;
            printT_min = "_" + printT_min.Replace(".", "_");
            if (mboDepth == true) { mDepthAry = new Double[mnCx, mnRy]; }
            if (mboHeight == true) { mHeightAry = new Double[mnCx, mnRy]; }
            if (mboDischarge == true) { mDischargeAry = new Double[mnCx, mnRy]; }
            if (mboVelocityMax == true) { mVelocityMaxAry = new Double[mnCx, mnRy]; }
            if (mboFDofMaxVelocity == true) { mFDofMaxVelocityAry = new Double[mnCx, mnRy]; }
            if (mboRFGrid == true) { mRFGridAry_m = new Double[mnCx, mnRy]; }
            getStringArrayOfCVAttributesUsing1DArray(mprj.domain.dminfo, cvs, cvsadd, nullvalue);
            if (mboDepth == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnDepthAsc = fpnDepthPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileDepth();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnDepthPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnDepthImg = fpnDepthPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileDepth();
                }
            }
            if (mboHeight == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnHeightAsc = fpnHeightPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileHeight();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnHeightPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnHeightImg = fpnHeightPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileHeight();
                }
            }
            if (mboDischarge == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnDischargeMaxAsc = fpnDischargeMaxPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileDischargeMax();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnDischargeMaxPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnDischargeMaxImg = fpnDischargeMaxPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileDischargeMax();
                }
            }
            if (mboVelocityMax == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnVelocityMaxAsc = fpnVelocityMaxPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileVelocityMax();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnVelocityMaxPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnVelocityMaxImg = fpnVelocityMaxPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileVelocityMax();
                }
            }
            if (mboFDofMaxVelocity == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnFDofMaxVelocityAsc = fpnFDofMaxVelocityPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileFDofVMax();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnFDofMaxVelocityPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnFDofMaxVelocityImg = fpnFDofMaxVelocityPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileFDofVMax();
                }
            }

            if (mboRFGrid == true)
            {
                if (cGenEnv.bMakeASCFile == true)
                {
                    fpnRFGridAsc = fpnRFGridPre + printT_min + cVars.CONST_OUTPUT_ASCFILE_EXTENSION;
                    StartMakeASCTextFileRFGrid();
                    if (mprj.fpnDEMprj != "") { File.Copy(mprj.fpnDEMprj, fpnRFGridPre + printT_min + ".prj", true); }
                }
                if (cGenEnv.bMakeImgFile == true)
                {
                    fpnRFGridImg = fpnRFGridPre + printT_min + cVars.CONST_OUTPUT_IMGFILE_EXTENSION;
                    StartMakeImgFileRFGrid();
                }
            }

            DateTime printTime = DateTime.Now;
            TimeSpan tsTotalSim = printTime - cGenEnv.simulationStartTime;
            TimeSpan tsThisStep = printTime - cGenEnv.thisPrintStepStartTime;

            string floodlingCellinfo = "".ToString();
            for (int n = 0; n < cGenEnv.floodingCellDepthThresholds_m.Count; n++)
            {
                if (n == 0)
                {
                    floodlingCellinfo = ">" + (cGenEnv.floodingCellDepthThresholds_m[n] * 100).ToString() + "cm"+
                        ", No, " + cThisProcess.FloodingCellCounts[n].ToString() + 
                        ", MeanD, " + cThisProcess.FloodingCellMeanDepth[n].ToString("F3");
                    //", MaxD, " + cThisProcess.FloodingCellMaxDepth[n].ToString("F2") + 
                }
                else
                {
                    floodlingCellinfo = floodlingCellinfo + ", " + ">" + (cGenEnv.floodingCellDepthThresholds_m[n] *100).ToString() + "cm" +
                        ", No, " + cThisProcess.FloodingCellCounts[n].ToString() +
                        ", MeanD, " + cThisProcess.FloodingCellMeanDepth[n].ToString("F3");
                    //", MaxD, " + cThisProcess.FloodingCellMaxDepth[n].ToString("F2") +
                }
            }

            if (cThisProcess.maxResdCell == "") { cThisProcess.maxResdCell = "(0,0)"; }
            cGenEnv.writelog(string.Format("T: {0}, dt(s): {1}, T in this print(s): {2}, " +
                "T from starting(m): {3}, iAllCells: {4}, iACell: {5}, maxR(cell), {6}, Eff. cells, {7}, MaxD, {8}, Flooding cells({9})", printT_min_oriString, cGenEnv.dt_sec.ToString("F2"),
                tsThisStep.TotalSeconds.ToString("F2"), tsTotalSim.TotalMinutes.ToString("F2"), cGenEnv.iGS, cGenEnv.iNR,
                cThisProcess.maxResd.ToString("F5") + cThisProcess.maxResdCell, cThisProcess.effCellCount, cThisProcess.FloodingCellMaxDepth.ToString ("F3"),
               floodlingCellinfo), true);

            // 이건 특정 행렬을 출력할때만 주석 해제
            //=========================
            if (cGenEnv.vdtest == true)
            {
                //string summary = fidx.Replace("_", "")+"\t";
                string summary = printT_min_oriString + "\t";
                for (int n = 0; n < mnCx; n++)
                //for (int n = 0; n < mnRy; n++)
                {
                    //summary = summary + mDepthAry[n, 0].ToString() + "\t";
                    summary = summary + mHeightAry[n, 0].ToString() + "\t";
                    //summary = summary + mHeightAry[0, n].ToString() + "\t";
                    //summary = summary + mDepthAry[10, n].ToString() + "\t";
                }
                summary = summary + "\r\n";
                System.IO.File.AppendAllText(mprj.fpnTest_willbeDeleted, summary, Encoding.UTF8);
            }
            //=========================

            return true;
        }


        private bool getStringArrayOfCVAttributesUsing1DArray(domainAtt[,] allCells, stCVAtt[] cvs, stCVAtt_add[] cvsadd, double nullvalue)
        {

            var options = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
            //for (int y = 0; y < allCells.GetLength(1); y++)
            Parallel.For(0, allCells.GetLength(1), options, delegate (int y)
            {
                for (int x = 0; x < allCells.GetLength(0); x++)
                {
                    int i = allCells[x, y].cvid;
                    if (i > -1) //0보다 같거나 크면 domain 안쪽이다. -1 이면 domain 밖이다.
                    {
                        if (mboDepth == true)
                        {
                            double v = cvs[i].dp_tp1;
                            mDepthAry[x, y] = v;
                        }
                        if (mboHeight == true)
                        {
                            double v = cvs[i].hp_tp1;
                            mHeightAry[x, y] = v;
                        }
                        if (mboDischarge == true)
                        {
                            double v = cvsadd[i].Qmax_cms;
                            mDischargeAry[x, y] = v;
                        }
                        if (mboVelocityMax == true)
                        {
                            double v = cvsadd[i].vmax;
                            mVelocityMaxAry[x, y] = v;
                        }
                        if (mboFDofMaxVelocity == true)
                        {
                            double v = (double)cvsadd[i].fdmax;
                            mFDofMaxVelocityAry[x, y] = v;
                        }
                        if (mboRFGrid == true)
                        {
                            double v = (cvsadd[i].sourceRFapp_dt_meter * 1000);
                            mRFGridAry_m[x, y] = v;
                        }
                    }
                    else
                    {
                        if (mboDepth == true)
                        {
                            mDepthAry[x, y] = nullvalue;
                        }
                        if (mboHeight == true)
                        {
                            mHeightAry[x, y] = nullvalue;
                        }
                        if (mboDischarge == true)
                        {
                            mDischargeAry[x, y] = nullvalue;
                        }
                        if (mboVelocityMax == true)
                        {
                            mVelocityMaxAry[x, y] = nullvalue;
                        }
                        if (mboFDofMaxVelocity == true)
                        {
                            mFDofMaxVelocityAry[x, y] = nullvalue;
                        }
                        if (mboRFGrid == true)
                        {
                            mRFGridAry_m[x, y] = nullvalue;
                        }
                    }
                }
                //}
            });
            return true;
        }
    }
}

