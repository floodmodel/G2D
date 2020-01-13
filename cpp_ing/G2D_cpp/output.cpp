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
extern thisProcess ps;
extern thisProcessInner psi;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern domainCell** dmcells;

double ** oDepth;
double** oHeight;
double** oDischarge;
double** oVMax;
double** oFDofMaxV;

string fpnQMaxPre = "";
string fpnDepthPre = "";
string fpnHeightPre = "";
string fpnVMaxPre = "";
string fpnFDofMaxVPre = "";

string fpnDepthAsc = "";
string fpnDepthImg = "";
string fpnHeightAsc = "";
string fpnHeightImg = "";
string fpnQMaxAsc = "";
string fpnQMaxImg = "";
string fpnVMaxAsc = "";
string fpnVMaxImg = "";
string fpnFDofMaxVAsc = "";
string fpnFDofMaxVImg = "";

int deleteAlloutputFiles()
{
    //모의 시작 할때, 다 지우고 새로 만든다
    vector<string> fpns;
    fs::path fn = fpn_prj.filename();
    string prjNameWithoutExt = fn.replace_extension().string();
    for (const auto& entry : fs::directory_iterator(fp_prj))
    {
        string afile = entry.path().string();
        if (afile.find(CONST_OUTPUT_ASCFILE_EXTENSION, 0) != string::npos ||
            afile.find(CONST_OUTPUT_IMGFILE_EXTENSION, 0) != string::npos ||
            afile.find(CONST_OUTPUT_PROJECTIONFILE_EXTENSION, 0) != string::npos ||
            afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_LCASE, 0) != string::npos ||
            afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_UCASE, 0) != string::npos)
        {
            if (afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_HEIGHT, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_RFGRID, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_BCDATA, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SOURCEALL, 0) != string::npos ||
                afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SINKDATA, 0) != string::npos)
            {
                fpns.push_back(afile);
            }
        }

    }
    if (fpns.size() > 0) {
        writeLog(fpn_log, "Delete all output files...", 1, 1);
        if (confirmDeleteFiles(fpns) == -1) {
            ge.modelSetupIsNormal = -1;
            writeLog(fpn_log, "Some output files were not deleted. Initializing new output files was failed.\n", 1, 1);
            return -1;
        }
        writeLog(fpn_log, " completed.\n", 1, 1);
    }
    return 1;
}

int initializeOutputArray()
{
    int rv = -1;
    if (prj.outputDepth == 1) {
        oDepth = new double* [di.nCols];
    }
    if (prj.outputHeight == 1) {
        oHeight = new double* [di.nCols];
    }
    if (prj.outputDischargeMax == 1) {
        oDischarge = new double* [di.nCols];
    }
    if (prj.outputVelocityMax == 1) {
        oVMax = new double* [di.nCols];
    }
    if (prj.outputFDofMaxV == 1) {
        oFDofMaxV = new double* [di.nCols];
    }

    for (int i = 0; i < di.nCols; ++i) {
        if (prj.outputDepth == 1) {
            oDepth[i] = new double[di.nRows];
        }
        if (prj.outputHeight == 1) {
            oHeight[i] = new double[di.nRows];
        }
        if (prj.outputDischargeMax == 1) {
            oDischarge[i] = new double[di.nRows];
        }
        if (prj.outputVelocityMax == 1) {
            oVMax[i] = new double[di.nRows];
        }
        if (prj.outputFDofMaxV == 1) {
            oFDofMaxV[i] = new double[di.nRows];
        }
    }
    rv = 1;
    return rv;
}

int makeOutputFiles(double nowTsec)
{
    string printT = "";
    if (prj.isDateTimeFormat == 1) {
        printT = timeElaspedToString_yyyymmddHHMM(prj.startDateTime, (int)nowTsec);
    }
    else {
        if (prj.printOutInterval_min < 60.0) {
            string printM = forString(nowTsec / 60.0, 2);
            printT = printM + "m";
        }
        else if (prj.printOutInterval_min >= 60 && prj.printOutInterval_min < 1440) {
            string printH = forString(nowTsec / 3600.0, 2);
            printT = printH + "h";
        }
        else if (prj.printOutInterval_min >= 1440) {
            string printD = forString(nowTsec / 8640.0, 2);
            printT = printD + "d";
        }
    }
    string printT_min_oriString = printT;
    if (prj.isDateTimeFormat == -1) {
        printT = "_" + replaceText(printT, ".", "_");
    }

    setOutputArray();
    if (prj.outputDepth == 1) {
        if (prj.makeASCFile == 1) {
            fpnDepthAsc = fpnDepthPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            startMakeASCTextFileDepth();
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnDepthPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnDepthImg = fpnDepthPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            StartMakeImgFileDepth();
        }
    }
    if (prj.outputHeight == 1) {
        if (prj.makeASCFile == 1) {
            fpnHeightAsc = fpnHeightPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            StartMakeASCTextFileHeight();
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnHeightPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnHeightImg = fpnHeightPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            StartMakeImgFileHeight();
        }
    }
    if (prj.outputDischargeMax == 1) {
        if (prj.makeASCFile == 1) {
            fpnQMaxAsc = fpnQMaxPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            StartMakeASCTextFileDischargeMax();
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnQMaxPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnQMaxImg = fpnQMaxPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            StartMakeImgFileDischargeMax();
        }
    }
    if (prj.outputVelocityMax == 1) {
        if (prj.makeASCFile == 1) {
            fpnVMaxAsc = fpnVMaxPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            StartMakeASCTextFileVelocityMax();
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnVMaxPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnVMaxImg = fpnVMaxPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            StartMakeImgFileVelocityMax();
        }
    }
    if (prj.outputFDofMaxV == 1) {
        if (prj.makeASCFile == 1) {
            fpnFDofMaxVAsc = fpnFDofMaxVPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            StartMakeASCTextFileFDofVMax();
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnFDofMaxVPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnFDofMaxVImg = fpnFDofMaxVPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            StartMakeImgFileFDofVMax();
        }
    }

    COleDateTime printTime = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = printTime - ps.simulationStartTime;
    COleDateTimeSpan tsThisStep = printTime - ps.thisPrintStepStartTime;
    string floodlingCellinfo = "";
    for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
        if (n == 0) {
            floodlingCellinfo = ">" + forString(ps.floodingCellDepthThresholds_m[n] * 100, 3) + "cm" +
                ", No, " + to_string(ps.FloodingCellCounts[n]) +
                ", MeanD, " + forString(ps.FloodingCellMeanDepth[n], 3);
        }
        else {
            floodlingCellinfo = floodlingCellinfo + ", " + ">" + forString(ps.floodingCellDepthThresholds_m[n] * 100, 3) + "cm" +
                ", No, " + to_string(ps.FloodingCellCounts[n]) +
                ", MeanD, " + forString(ps.FloodingCellMeanDepth[n], 3);
        }
    }

    string maxResdCell;
    maxResdCell = "(0, 0)";
    if (psi.maxResdCVID > -1) {
        int xcol = cvs[psi.maxResdCVID].colx;
        int yrow = cvs[psi.maxResdCVID].rowy;
        maxResdCell = "(" + to_string(xcol) + ", " + to_string(yrow) + ")";
    }
    string logString = "T: " + printT_min_oriString
        + ", dt(s): " + forString(psi.dt_sec, 2)
        + ", T in this print(s): " + forString(tsThisStep.GetTotalSeconds(), 2)
        + ", T from starting(m): " + forString(tsTotalSim.GetTotalMinutes(), 2)
        + ", iAllCells: " + to_string(psi.iGS) + ", iACell: " + to_string(psi.iNR)
        + ", maxR(cell), " + forString(psi.maxResd, 5) + maxResdCell
        + ", Eff. cells, " + to_string(ps.effCellCount)
        + ", MaxD, " + forString(ps.FloodingCellMaxDepth, 3)
        + ", Flooding cells(" + floodlingCellinfo + ")";
    writeLog(fpn_log, logString, 1, -1);

    // 이건 특정 행렬을 출력할때만 주석 해제
    //=========================
    if (ge.vdtest == true) {
        //string summary = fidx.Replace("_", "")+"\t";
        string summary = printT_min_oriString + "\t";
        for (int n = 0; n < di.nCols; n++)
            //for (int n = 0; n < di.nRows; n++)
        {
            //summary = summary + oDepth[n, 0].ToString() + "\t";
            summary = summary + to_string(oHeight[n][0]) + "\t";
            //summary = summary + oHeight[0][n].ToString() + "\t";
            //summary = summary + oDepth[10][n].ToString() + "\t";
        }
        summary = summary + "\r\n";
        appendTextToTextFile(prj.fpnTest_willbeDeleted, summary);
    }
    //=========================
    return true;
}

int setOutputArray()
{
    int rv = -1;
    int nullv = di.nodata_value;
    for (int y = 0; y < di.nRows; ++y) {
        for (int x = 0; x < di.nCols; ++x) {
            int i = dmcells[x][y].cvid;
            if (i > -1) //0보다 같거나 크면 domain 안쪽이다. -1 이면 domain 밖이다.
            {
                if (prj.outputDepth == 1)
                {
                    double v = cvs[i].dp_tp1;
                    oDepth[x][y] = v;
                }
                if (prj.outputHeight == 1)
                {
                    double v = cvs[i].hp_tp1;
                    oHeight[x][y] = v;
                }
                if (prj.outputDischargeMax == 1)
                {
                    double v = cvsAA[i].Qmax_cms;
                    oDischarge[x][y] = v;
                }
                if (prj.outputVelocityMax == 1)
                {
                    double v = cvsAA[i].vmax;
                    oVMax[x][y] = v;
                }
                if (prj.outputFDofMaxV == 1)
                {
                    double v = (double)cvsAA[i].fdmax;
                    oFDofMaxV[x][y] = v;
                }
            }
            else {
                if (prj.outputDepth == 1) {
                    oDepth[x][y] = nullv;
                }
                if (prj.outputHeight == 1) {
                    oHeight[x][y] = nullv;
                }
                if (prj.outputDischargeMax == 1) {
                    oDischarge[x][y] = nullv;
                }
                if (prj.outputVelocityMax == 1) {
                    oVMax[x][y] = nullv;
                }
                if (prj.outputFDofMaxV == 1) {
                    oFDofMaxV[x][y] = nullv;
                }
            }
        }
    }
    rv = 1;
    return rv;
}

void startMakeASCTextFileDepth()
{
    ThreadStart ts = new ThreadStart(MakeASCTextFileInnerDepth);
    Thread th = new Thread(ts);
    th.Start();
}
void makeASCTextFileInnerDepth()
{
    makeASCTextFile(fpnDepthAsc, mascHeaderAll, mDepthAry, 5, mascNodataValue);
}
