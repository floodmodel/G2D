#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>


#include "gentle.h"
#include "g2d.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;
extern projectFile prj;
extern domaininfo di;
extern generalEnv ge;

extern cvatt* cvs;
extern cvattAdd* cvsAA;

int deleteAlloutputFiles()
{
	//모의 시작 할때, 다 지우고 새로 만든다
	vector<string> fpns;
	fs::path fn = fpn_prj.filename();
	string prjNameWithoutExt = fn.replace_extension().string();
	for (const auto & entry : fs::directory_iterator(fp_prj))
	{
		string afile = entry.path().string();
		if (afile.find(CONST_OUTPUT_ASCFILE_EXTENSION,0) != string::npos  ||
			afile.find(CONST_OUTPUT_IMGFILE_EXTENSION,0) != string::npos 	||
			afile.find(CONST_OUTPUT_PROJECTIONFILE_EXTENSION,0) != string::npos ||
			afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_LCASE,0) != string::npos ||
			afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_UCASE,0) != string::npos )
		{
			if (afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE,0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH,0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_HEIGHT, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_RFGRID, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_BCDATA, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SOURCEALL, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SINKDATA, 0) != string::npos )
			{
				fpns.push_back(afile);
			}
		}

	}
	if (fpns.size() > 0)
	{
		writeLog( fpn_log, "Delete all output files...",1,1);
		if (confirmDeleteFiles(fpns) == -1)
		{
			ge.modelSetupIsNormal = -1;
			writeLog(fpn_log,"Some output files were not deleted. Initializing new output files was failed.\n", 1,1);
			return -1;
		}
		writeLog(fpn_log," completed.\n", 1,1);
	}
	return 1;
}


int makeOutputFiles(double nowTsec, int nullvalue)
{
    string printT_min = "";
    double printMIN = 0;
    //mNullValue = nullvalue;
    //printMIN = ((double)(nowTsec / 60)).ToString("F1");
    printMIN = nowTsec / 60.0;
    if (prj.isDateTimeFormat == true) {
        printT_min = timeElaspedToString_yyyymmdd_HHcolonMM(prj.startDateTime, (int)nowTsec / 60, "yyyyMMddHHmm");
    }
    else {
        if (prj.printOutInterval_min < 60.0) {
            string printM = formattedString(printMIN, 2);
            printT_min = printM + "m";
        }
        else if (prj.printOutInterval_min >= 60 && prj.printOutInterval_min < 1440) {
            string printH = formattedString(printMIN / 60, 2);
            printT_min = printH + "h";
        }
        else if (prj.printOutInterval_min >= 1440) {
            string printD = formattedString(printMIN / 1440, 2);
            printT_min = printD + "d";
        }
    }
    cGenEnv.writelog("Making output file at the time " + printT_min, cGenEnv.bwritelog_process);
    string printT_min_oriString = printT_min;
    printT_min = "_" + printT_min.Replace(".", "_");
    if (prj.outputDepth == true) { mDepthAry = new Double[mnCx, mnRy]; }
    if (prj.outputHeight == true) { mHeightAry = new Double[mnCx, mnRy]; }
    if (prj.outputDischargeMax == true) { mDischargeAry = new Double[mnCx, mnRy]; }
    if (prj.outputVelocityMax == true) { mVelocityMaxAry = new Double[mnCx, mnRy]; }
    if (prj.outputFDofMaxV == true) { mFDofMaxVelocityAry = new Double[mnCx, mnRy]; }
    if (prj.outputRFGrid == true) { mRFGridAry_m = new Double[mnCx, mnRy]; }
    getStringArrayOfCVAttributesUsing1DArray(mprj.domain.dminfo, cvs, cvsadd, nullvalue);
    if (prj.outputDepth == true)
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
    if (prj.outputHeight == true)
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
    if (prj.outputDischargeMax == true)
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
    if (prj.outputVelocityMax == true)
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
    if (prj.outputFDofMaxV == true)
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

    if (prj.outputRFGrid == true)
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
            floodlingCellinfo = ">" + (cGenEnv.floodingCellDepthThresholds_m[n] * 100).ToString() + "cm" +
                ", No, " + cThisProcess.FloodingCellCounts[n].ToString() +
                ", MeanD, " + cThisProcess.FloodingCellMeanDepth[n].ToString("F3");
            //", MaxD, " + cThisProcess.FloodingCellMaxDepth[n].ToString("F2") + 
        }
        else
        {
            floodlingCellinfo = floodlingCellinfo + ", " + ">" + (cGenEnv.floodingCellDepthThresholds_m[n] * 100).ToString() + "cm" +
                ", No, " + cThisProcess.FloodingCellCounts[n].ToString() +
                ", MeanD, " + cThisProcess.FloodingCellMeanDepth[n].ToString("F3");
            //", MaxD, " + cThisProcess.FloodingCellMaxDepth[n].ToString("F2") +
        }
    }

    if (cThisProcess.maxResdCell == "") { cThisProcess.maxResdCell = "(0,0)"; }
    cGenEnv.writelog(string.Format("T: {0}, dt(s): {1}, T in this print(s): {2}, " +
        "T from starting(m): {3}, iAllCells: {4}, iACell: {5}, maxR(cell), {6}, Eff. cells, {7}, MaxD, {8}, Flooding cells({9})", printT_min_oriString, cGenEnv.dt_sec.ToString("F2"),
        tsThisStep.TotalSeconds.ToString("F2"), tsTotalSim.TotalMinutes.ToString("F2"), cGenEnv.iGS, cGenEnv.iNR,
        cThisProcess.maxResd.ToString("F5") + cThisProcess.maxResdCell, cThisProcess.effCellCount, cThisProcess.FloodingCellMaxDepth.ToString("F3"),
        floodlingCellinfo), true);

    // 이건 특정 행렬을 출력할때만 주석 해제
    //=========================
    if (cGenEnv.vdtest == true)
    {
        //string summary = fidx.Replace("_", "")+"\t";
        string summary = printT_min_oriString + "\t";
        for (int n = 0; n < di.nCols; n++)
            //for (int n = 0; n < di.nRows; n++)
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


