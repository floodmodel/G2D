#include "stdafx.h"
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
extern globalVinner gvi;
extern cvatt* cvs;
//extern cvattAddAtt* cvsAA;
extern cvattMaxValueAndFD* cvsMVnFD;
extern domainCell** dmcells;
extern minMaxFlux mnMxFluxFromAllcells;

//extern thread* th_makeASCTextFileDepth;
// 전역 thread로 전역 array를 쓰고, main에서 join 하면,, 
// thread 작업 도중에 array 값이 바뀌어서, 제대로 출력이 안된다..
// 그래서 local thread로 작업하고, output process 종료 전에 join 한다. 
thread* th_makeASCTextFileDepth;
thread* th_makeASCTextFileWaterLevel;
thread* th_makeASCTextFileDischargeMax;
thread* th_makeASCTextFileVelocityMax;
thread* th_makeASCTextFileFDofVMax;
thread* th_makeASCTextFileFDofQMax;

thread* th_makeImgFileDepth;
thread* th_makeImgFileWaterLevel;
thread* th_makeImgFileQMax;
thread* th_makeImgFileVMax;
thread* th_makeImgFileFDofVMax;
thread* th_makeImgFileFDofQMax;


double** oAryDepth;
double** oAryWaterLevel;
double** oAryQMax;
double** oAryVMax;
double** oAryFDofMaxV;
double** oAryFDofMaxQ;

string fpnQMaxPre = "";
string fpnDepthPre = "";
string fpnWaterLevelPre = "";
string fpnVMaxPre = "";
string fpnFDofMaxVPre = "";
string fpnFDofMaxQPre = "";

string fpnDepthAsc = "";
string fpnDepthImg = "";
string fpnWaterLevelAsc = "";
string fpnWaterLevelimg = "";
string fpnQMaxAsc = "";
string fpnQMaxImg = "";
string fpnVMaxAsc = "";
string fpnVMaxImg = "";
string fpnFDofMaxVAsc = "";
string fpnFDofMaxVImg = "";
string fpnFDofMaxQAsc = "";
string fpnFDofMaxQImg = "";

string fpnCellValue_Depth = "";
string fpnCellValue_WaterLevel = "";
string fpnCellValue_QMax = "";
string fpnCellValue_VMax = "";
string fpnCellValue_FDofMaxV = "";
string fpnCellValue_FDofMaxQ = "";

string cellValues_Depth = "";
string cellValues_WaterLevel = "";
string cellValues_QMax = "";
string cellValues_VMax = "";
string cellValues_FDofMaxV = "";
string cellValues_FDofMaxQ = "";


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
			afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_UCASE, 0) != string::npos||
			afile.find(CONST_OUTPUT_CELLVALUE_EXTENSION, 0) != string::npos)
		{
			if (afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_WATERLEVEL, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXV, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXQ, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_WATERLEVEL + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXV + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXQ + CONST_FILENAME_TAG_CELLVALUE, 0) != string::npos)
				     // afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_RFGRID, 0) != string::npos ||
					 //afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_BCDATA, 0) != string::npos ||
					 //afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SOURCEALL, 0) != string::npos ||
					 //afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SINKDATA, 0) != string::npos)
			{
				fpns.push_back(afile);
			}
		}
	}
    if (fpns.size() > 0) {
        writeLog(fpn_log, "Delete all output files... \n", 1, 1);
        if (confirmDeleteFiles(fpns) == 0) {
            ge.modelSetupIsNormal = 0;
            writeLog(fpn_log, "Some output files were not deleted. Initializing new output files was failed.\n", 1, 1);
            return -1;
        }
        writeLog(fpn_log, "Delete all output files completed.\n", 1, 1);
    }
    return 1;
}

int initializeOutputArrayAndFile()
{
    int rv = -1;
    if (prj.outputDepth == 1) {
        oAryDepth = new double* [di.nCols];
    }
    if (prj.outputWaterLevel == 1) {
        oAryWaterLevel = new double* [di.nCols];
    }
    if (prj.outputDischargeMax == 1) {
        oAryQMax = new double* [di.nCols];
    }
    if (prj.outputVelocityMax == 1) {
        oAryVMax = new double* [di.nCols];
    }
    if (prj.outputFDofMaxV == 1) {
        oAryFDofMaxV = new double* [di.nCols];
    }
	if (prj.outputFDofMaxQ == 1) {
		oAryFDofMaxQ = new double* [di.nCols];
	}

    for (int i = 0; i < di.nCols; ++i) {
        if (prj.outputDepth == 1) {
            oAryDepth[i] = new double[di.nRows];
        }
        if (prj.outputWaterLevel == 1) {
            oAryWaterLevel[i] = new double[di.nRows];
        }
        if (prj.outputDischargeMax == 1) {
            oAryQMax[i] = new double[di.nRows];
        }
        if (prj.outputVelocityMax == 1) {
            oAryVMax[i] = new double[di.nRows];
        }
        if (prj.outputFDofMaxV == 1) {
            oAryFDofMaxV[i] = new double[di.nRows];
        }
		if (prj.outputFDofMaxQ == 1) {
			oAryFDofMaxQ[i] = new double[di.nRows];
		}
    }
    //=================
    fs::path fn = fpn_prj.filename();
    string prjNameWithoutExt = fn.replace_extension().string();
	string header_cellValueFile = "";
	if (prj.printCellValue == 1) {
		int ncells = prj.cellLocationsToPrint.size();
		header_cellValueFile = "DataTime, ";
		for (int n = 0; n < ncells; ++n) {
			if (n == ncells - 1) {
				header_cellValueFile += "(" + to_string(prj.cellLocationsToPrint[n].xCol) + ", "
					+ to_string(prj.cellLocationsToPrint[n].yRow) + ")";
			}
			else
			{
				header_cellValueFile += "(" + to_string(prj.cellLocationsToPrint[n].xCol) + ", "
					+ to_string(prj.cellLocationsToPrint[n].yRow) + "), ";
			}

		}
	}
    if (prj.outputDepth == 1) {
        string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH;
        fs::path tmpn = fp_prj / fnOnly;
        fpnDepthPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE+ CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_Depth = tmpn.string();
			std::ofstream outfile(fpnCellValue_Depth, ios::out);
			outfile << header_cellValueFile<<endl;
			outfile.close();
		}
    }
    if (prj.outputWaterLevel == 1) {
        string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_WATERLEVEL;
        fs::path tmpn = fp_prj / fnOnly;
        fpnWaterLevelPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE + CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_WaterLevel = tmpn.string();
			std::ofstream outfile(fpnCellValue_WaterLevel, ios::out);
			outfile << header_cellValueFile << endl;
			outfile.close();
		}
    }
    if (prj.outputDischargeMax == 1) {
        string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE;
        fs::path tmpn = fp_prj / fnOnly;
        fpnQMaxPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE + CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_QMax = tmpn.string();
			std::ofstream outfile(fpnCellValue_QMax, ios::out);
			outfile << header_cellValueFile << endl;
			outfile.close();
		}
    }
    if (prj.outputVelocityMax == 1) {
        string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY;
        fs::path tmpn = fp_prj / fnOnly;
        fpnVMaxPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE + CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_VMax = tmpn.string();
			std::ofstream outfile(fpnCellValue_VMax, ios::out);
			outfile << header_cellValueFile << endl;
			outfile.close();
		}
    }
    if (prj.outputFDofMaxV == 1) {
        string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXV;
        fs::path tmpn = fp_prj / fnOnly;
        fpnFDofMaxVPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE + CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_FDofMaxV = tmpn.string();
			std::ofstream outfile(fpnCellValue_FDofMaxV, ios::out);
			outfile << header_cellValueFile << endl;
			outfile.close();
		}
    }
	if (prj.outputFDofMaxQ == 1) {
		string fnOnly = prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION_MAXQ;
		fs::path tmpn = fp_prj / fnOnly;
		fpnFDofMaxQPre = tmpn.string();
		if (prj.printCellValue == 1) {
			fnOnly = fnOnly + CONST_FILENAME_TAG_CELLVALUE + CONST_OUTPUT_CELLVALUE_EXTENSION;
			tmpn = fp_prj / fnOnly;
			fpnCellValue_FDofMaxQ = tmpn.string();
			std::ofstream outfile(fpnCellValue_FDofMaxQ, ios::out);
			outfile << header_cellValueFile << endl;
			outfile.close();
		}
	}
    //=================
    rv = 1;
    return rv;
}


int makeOutputFiles(double nowTsec, int iGSmax)
{
    string printT = "";
    if (prj.isDateTimeFormat == 1) {
        printT = timeElaspedToDateTimeFormat2(prj.startDateTime, 
            (int)nowTsec, timeUnitToShow::toM, 
            dateTimeFormat::yyyymmddHHMMSS);
    }
	else {
		if (prj.printOutInterval_min < 60.0) {
			string printM = dtos_L(nowTsec / 60.0, ps.tTag_length, 2);
			printT = printM + "m";
		}
		else if (prj.printOutInterval_min >= 60 && prj.printOutInterval_min < 1440) {
			string printH = dtos_L(nowTsec / 3600.0, ps.tTag_length, 2);
			printT = printH + "h";
		}
		else if (prj.printOutInterval_min >= 1440) {
			string printD = dtos_L(nowTsec / 8640.0, ps.tTag_length, 2);
			printT = printD + "d";
		}
	}
    string printT_min_oriString = printT;
    //if (prj.isDateTimeFormat == 0) {
	printT = "_" + replaceText(printT, ".", "_");
    //}
    setOutputArray();
	if (prj.printCellValue == 1) {
		setCellValuePrintLine(printT_min_oriString);
	}
    int num_x = di.nCols;
    int num_y = di.nRows;
    if (prj.outputDepth == 1) {
        if (prj.makeASCFile == 1) {
            fpnDepthAsc = fpnDepthPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            th_makeASCTextFileDepth = new thread(makeASCTextFileDepth);
            if (prj.fpnDEMprjection != "") {
				fs::copy(prj.fpnDEMprjection, fpnDepthPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnDepthImg = fpnDepthPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            th_makeImgFileDepth=new thread(makeImgFileDepth);
        }
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_Depth, ios::app);
			outfile << cellValues_Depth << endl;
			outfile.close();
		}
    }
    if (prj.outputWaterLevel == 1) {
        if (prj.makeASCFile == 1) {
            fpnWaterLevelAsc = fpnWaterLevelPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            th_makeASCTextFileWaterLevel = new thread(makeASCTextFileWaterLevel);
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnWaterLevelPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnWaterLevelimg = fpnWaterLevelPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            th_makeImgFileWaterLevel = new thread(makeImgFileWaterLevel);
        }
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_WaterLevel, ios::app);
			outfile << cellValues_WaterLevel << endl;
			outfile.close();
		}
    }
    if (prj.outputDischargeMax == 1) {
        if (prj.makeASCFile == 1) {
            fpnQMaxAsc = fpnQMaxPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            th_makeASCTextFileDischargeMax = new thread(makeASCTextFileDischargeMax);
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnQMaxPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnQMaxImg = fpnQMaxPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            th_makeImgFileQMax = new thread(makeImgFileDischargeMax);
        }
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_QMax, ios::app);
			outfile << cellValues_QMax << endl;
			outfile.close();
		}
    }
    if (prj.outputVelocityMax == 1) {
        if (prj.makeASCFile == 1) {
            fpnVMaxAsc = fpnVMaxPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            th_makeASCTextFileVelocityMax = new thread(makeASCTextFileVelocityMax);
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnVMaxPre + printT + ".prj");
            }
        }
        if (prj.makeImgFile == 1) {
            fpnVMaxImg = fpnVMaxPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
            th_makeImgFileVMax = new thread(makeImgFileVelocityMax);
        }
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_VMax, ios::app);
			outfile << cellValues_VMax << endl;
			outfile.close();
		}
    }
    if (prj.outputFDofMaxV == 1) {
        if (prj.makeASCFile == 1) {
            fpnFDofMaxVAsc = fpnFDofMaxVPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
            th_makeASCTextFileFDofVMax = new thread(makeASCTextFileFDofVMax);
            if (prj.fpnDEMprjection != "") {
                fs::copy(prj.fpnDEMprjection, fpnFDofMaxVPre + printT + ".prj");
            }
        }
        // FD는 이미지 출력하지 않는다.
        //if (prj.makeImgFile == 1) {
        //    fpnFDofMaxVImg = fpnFDofMaxVPre + printT + CONST_OUTPUT_IMGFILE_EXTENSION;
        //    //StartMakeImgFileFDofVMax();
        //}
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_FDofMaxV, ios::app);
			outfile << cellValues_FDofMaxV << endl;
			outfile.close();
		}
    }
	if (prj.outputFDofMaxQ == 1) {
		if (prj.makeASCFile == 1) {
			fpnFDofMaxQAsc = fpnFDofMaxQPre + printT + CONST_OUTPUT_ASCFILE_EXTENSION;
			th_makeASCTextFileFDofQMax = new thread(makeASCTextFileFDofQMax);
			if (prj.fpnDEMprjection != "") {
				fs::copy(prj.fpnDEMprjection, fpnFDofMaxQPre + printT + ".prj");
			}
		}
		// FD는 이미지 출력하지 않는다.
		if (prj.printCellValue == 1) {
			std::ofstream outfile(fpnCellValue_FDofMaxQ, ios::app);
			outfile << cellValues_FDofMaxQ << endl;
			outfile.close();
		}
	}
    COleDateTime printTime = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = printTime - ps.simulationStartTime;
    COleDateTimeSpan tsThisStep = printTime - ps.thisPrintStepStartTime;
    string floodlingCellinfo = "";
    for (int n = 0; n < ps.floodingCellDepthThresholds_m.size(); n++) {
        if (n == 0) {
            floodlingCellinfo = ">" + dtos(ps.floodingCellDepthThresholds_m[n] * 100, 2) + "cm" +
                ", No, " + to_string(ps.FloodingCellCounts[n]) +
                ", MeanD, " + dtos(ps.FloodingCellMeanDepth[n], 3);
        }
        else {
            floodlingCellinfo += ", >" + dtos(ps.floodingCellDepthThresholds_m[n] * 100, 2) + "cm" +
                ", No, " + to_string(ps.FloodingCellCounts[n]) +
                ", MeanD, " + dtos(ps.FloodingCellMeanDepth[n], 3);
        }
    }

    string maxResdCell;
    maxResdCell = "(0, 0)";
	if (ps.maxResdCVidx > -1) {
		int xcol = cvs[ps.maxResdCVidx].colx;
		int yrow = cvs[ps.maxResdCVidx].rowy;
		maxResdCell = "(" + to_string(xcol) + ", " + to_string(yrow) + ")";
	}
	string gsString = "";
	if (prj.usingGPU == 0) {
		gsString = ", iAllCells: ";
	}
	else {
		gsString = ", iAllCellsLimit: ";
	}
    string logString = "T: " + printT_min_oriString
        + ", dt(s): " + dtos(gvi.dt_sec, 2)
        + ", T in this print(s): " + dtos(tsThisStep.GetTotalSeconds(), 0)
        + ", T from starting(m): " + dtos(tsTotalSim.GetTotalSeconds()/60.0, 2)
        + gsString + to_string(iGSmax) 
        + ", maxR(cell), " + dtos(ps.maxResd, 5) + maxResdCell
        + ", Eff. cells, " + to_string(psi.effCellCount)
        + ", MaxD, " + dtos(ps.FloodingCellMaxDepth, 3)
        + ", Flooding cells(" + floodlingCellinfo + ")\n";
    writeLog(fpn_log, logString, 1, -1);

#ifdef isVD 	// 이건 특정 행렬을 출력할때만 사용
        //string summary = fidx.Replace("_", "")+"\t";
        string summary = printT_min_oriString + "\t";
        for (int n = 0; n < di.nCols; n++)
            //for (int n = 0; n < di.nRows; n++)
        {
            //summary = summary + oDepth[n, 0].ToString() + "\t";
            summary = summary + to_string(oAryWaterLevel[n][0]) + "\t";
        }
        summary = summary + "\n";
        appendTextToTextFile(prj.fpnTest_willbeDeleted, summary);
#endif
    //=========================
    joinOutputThreads();
	
	//if (oAryDepth != NULL) { delete[] oAryDepth; }
	//if (oAryWaterLevel != NULL) { delete[] oAryWaterLevel; }
	//if (oAryQMax != NULL) { delete[] oAryQMax; }
	//if (oAryVMax != NULL) { delete[] oAryVMax; }
	//if (oAryFDofMaxV != NULL) { delete[] oAryFDofMaxV; }
	//if (oAryFDofMaxQ != NULL) { delete[] oAryFDofMaxQ; }
    return true;
}

void joinOutputThreads()
{
     if (th_makeASCTextFileDepth != NULL && th_makeASCTextFileDepth->joinable() == true) {
        th_makeASCTextFileDepth->join();
    }
    if (th_makeASCTextFileWaterLevel != NULL && th_makeASCTextFileWaterLevel->joinable() == true) {
        th_makeASCTextFileWaterLevel->join();
    }
    if (th_makeASCTextFileDischargeMax != NULL && th_makeASCTextFileDischargeMax->joinable() == true) {
        th_makeASCTextFileDischargeMax->join();
    }
    if (th_makeASCTextFileVelocityMax != NULL && th_makeASCTextFileVelocityMax->joinable() == true) {
        th_makeASCTextFileVelocityMax->join();
    }
    if (th_makeASCTextFileFDofVMax != NULL && th_makeASCTextFileFDofVMax->joinable() == true) {
        th_makeASCTextFileFDofVMax->join();
    }
	if (th_makeASCTextFileFDofQMax != NULL && th_makeASCTextFileFDofQMax->joinable() == true) {
		th_makeASCTextFileFDofQMax->join();
	}

    if (th_makeImgFileDepth != NULL && th_makeImgFileDepth->joinable() == true) {
        th_makeImgFileDepth->join();
    }
    if (th_makeImgFileWaterLevel != NULL && th_makeImgFileWaterLevel->joinable() == true) {
        th_makeImgFileWaterLevel->join();
    }
    if (th_makeImgFileQMax != NULL && th_makeImgFileQMax->joinable() == true) {
        th_makeImgFileQMax->join();
    }
    if (th_makeImgFileVMax != NULL && th_makeImgFileVMax->joinable() == true) {
        th_makeImgFileVMax->join();
    }
}

int setOutputArray()
{
	int rv = -1;
	int nullv = di.nodata_value;
#pragma omp parallel for
	for (int y = 0; y < di.nRows; ++y) {
		for (int x = 0; x < di.nCols; ++x) {
			int i = dmcells[x][y].cvidx;
			double depthv = -1.0;
			if (i > -1) {//i가 0과 같거나 크면 domain 안쪽이다. -1 이면 domain 밖이다.
				depthv= cvs[i].dp_tp1;
			}
			if (depthv > 0) { // domain 안쪽 && 수심이 있는 경우
				if (prj.outputDepth == 1) {
					oAryDepth[x][y] = depthv;
				}
				if (prj.outputWaterLevel == 1) {
					oAryWaterLevel[x][y] = cvs[i].hp_tp1;
				}
				if (prj.outputDischargeMax == 1) {
					oAryQMax[x][y] = cvsMVnFD[i].Qmax_cms;
				}
				if (prj.outputVelocityMax == 1) {
					oAryVMax[x][y] = cvsMVnFD[i].vmax;
				}
				if (prj.outputFDofMaxV == 1) {
					oAryFDofMaxV[x][y] = cvsMVnFD[i].fdmaxV;
				}
				if (prj.outputFDofMaxQ == 1) {
					oAryFDofMaxQ[x][y] = cvsMVnFD[i].fdmaxQ;
				}
			}
			else {
				if (prj.outputDepth == 1) {
					oAryDepth[x][y] = nullv;
				}
				if (prj.outputWaterLevel == 1) {
					oAryWaterLevel[x][y] = nullv;
				}
				if (prj.outputDischargeMax == 1) {
					oAryQMax[x][y] = nullv;
				}
				if (prj.outputVelocityMax == 1) {
					oAryVMax[x][y] = nullv;
				}
				if (prj.outputFDofMaxV == 1) {
					oAryFDofMaxV[x][y] = nullv;
				}
				if (prj.outputFDofMaxQ == 1) {
					oAryFDofMaxQ[x][y] = nullv;
				}
			}
		}
	}
	rv = 1;
	return rv;
}

void setCellValuePrintLine(string t_print) {

	cellValues_Depth = "";
	cellValues_WaterLevel = "";
	cellValues_QMax = "";
	cellValues_VMax = "";
	cellValues_FDofMaxV = "";
	if (prj.outputDepth == 1) {
		cellValues_Depth = t_print + ", ";
	}
	if (prj.outputWaterLevel == 1) {
		cellValues_WaterLevel = t_print + ", ";
	}
	if (prj.outputDischargeMax == 1) {
		cellValues_QMax = t_print + ", ";
	}
	if (prj.outputVelocityMax == 1) {
		cellValues_VMax = t_print + ", ";
	}
	if (prj.outputFDofMaxV == 1) {
		cellValues_FDofMaxV = t_print + ", ";
	}
	if (prj.outputFDofMaxQ == 1) {
		cellValues_FDofMaxQ = t_print + ", ";
	}
	int ncells = prj.cellLocationsToPrint.size();
	for (int n = 0; n < ncells; ++n) {
		int x = prj.cellLocationsToPrint[n].xCol;
		int y = prj.cellLocationsToPrint[n].yRow;
		int cvid = dmcells[x][y].cvidx;
		double depthv = -1.0;
		if (cvid > -1) {
			depthv = cvs[cvid].dp_tp1;
		}

		if (prj.outputDepth == 1) {
			string ostring = "";
			if (depthv >0) { // 수심이 있는 곳만 출력한다.
				ostring = dtos(oAryDepth[x][y], prj.outputPrecision_Depth);
			}
			else {
				ostring = to_string(di.nodata_value);
			}
			if (n == ncells - 1) {
				cellValues_Depth += ostring;
			}
			else {
				cellValues_Depth += ostring + ", ";
			}
		}
		if (prj.outputWaterLevel == 1) {
			string ostring = "";
  			if (depthv > 0) { // 수심이 있는 곳만 출력한다.
				ostring = dtos(oAryWaterLevel[x][y], prj.outputPrecision_WaterLevel); 
			}
			else {
				ostring = to_string(di.nodata_value);
			}
			if (n == ncells - 1) {
				cellValues_WaterLevel += ostring;
			}
			else {
				cellValues_WaterLevel += ostring + ", ";
			}
		}
		if (prj.outputDischargeMax == 1) {
			string ostring = "";
			if (depthv > 0) { // 수심이 있는 곳만 출력한다.
				ostring = dtos(oAryQMax[x][y], prj.outputPrecision_QMax);
			}
			else {
				ostring = to_string(di.nodata_value);
			}
			if (n == ncells - 1) {
				cellValues_QMax += ostring;
			}
			else {
				cellValues_QMax += ostring + ", ";
			}
		}
		if (prj.outputVelocityMax == 1) {
			string ostring = "";
			if (depthv > 0) { // 수심이 있는 곳만 출력한다.
				ostring = dtos(oAryVMax[x][y], prj.outputPrecision_VMax);
			}
			else {
				ostring = to_string(di.nodata_value);
			}
			if (n == ncells - 1) {
				cellValues_VMax += ostring;
			}
			else {
				cellValues_VMax += ostring + ", ";
			}
		}
		if (prj.outputFDofMaxV == 1) {
			if (n == ncells - 1) {
				cellValues_FDofMaxV += dtos(oAryFDofMaxV[x][y], 0);
			}
			else {
				cellValues_FDofMaxV += dtos(oAryFDofMaxV[x][y], 0) + ", ";
			}
		}
		if (prj.outputFDofMaxQ == 1) {
			if (n == ncells - 1) {
				cellValues_FDofMaxQ += dtos(oAryFDofMaxQ[x][y], 0);
			}
			else {
				cellValues_FDofMaxQ += dtos(oAryFDofMaxQ[x][y], 0) + ", ";
			}
		}
	}
}

void makeASCTextFileDepth()
{
    makeASCTextFile(fpnDepthAsc, di.headerStringAll, 
        oAryDepth, di.nCols, di.nRows, prj.outputPrecision_Depth, di.nodata_value);
}

void makeASCTextFileWaterLevel()
{
    makeASCTextFile(fpnWaterLevelAsc, di.headerStringAll, 
        oAryWaterLevel, di.nCols, di.nRows, prj.outputPrecision_WaterLevel, di.nodata_value);
}

void makeASCTextFileDischargeMax()
{
    makeASCTextFile(fpnQMaxAsc, di.headerStringAll,
        oAryQMax, di.nCols, di.nRows, prj.outputPrecision_QMax, di.nodata_value);
}

void makeASCTextFileVelocityMax()
{
    makeASCTextFile(fpnVMaxAsc, di.headerStringAll,
        oAryVMax, di.nCols, di.nRows, prj.outputPrecision_VMax, di.nodata_value);
}

void makeASCTextFileFDofVMax()
{
    makeASCTextFile(fpnFDofMaxVAsc, di.headerStringAll,
        oAryFDofMaxV, di.nCols, di.nRows, 0, di.nodata_value);
}

void makeASCTextFileFDofQMax()
{
	makeASCTextFile(fpnFDofMaxQAsc, di.headerStringAll,
		oAryFDofMaxQ, di.nCols, di.nRows, 0, di.nodata_value);
}

//fd는 이미지 출력하지 않는다.==============
void makeImgFileDepth()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnDepthImg, oAryDepth,
        di.nCols, di.nRows, rendererType::Depth, prj.rendererMaxVdepthImg, di.nodata_value);   
}

void makeImgFileWaterLevel()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnWaterLevelimg,oAryWaterLevel,
        di.nCols, di.nRows, rendererType::Depth, prj.rendererMaxVwaterLevelimg, di.nodata_value);
}

void makeImgFileDischargeMax()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnQMaxImg, oAryQMax,
        di.nCols, di.nRows, rendererType::Depth, prj.rendererMaxVFlow, di.nodata_value);
}

void makeImgFileVelocityMax()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnVMaxImg,  oAryVMax,
        di.nCols, di.nRows, rendererType::Depth, prj.rendererMaxVMaxVImg, di.nodata_value);
}
// ==========

