#include <stdio.h>
#include <fstream>
#include <filesystem>
#include <io.h>
#include<ATLComTime.h>
#include <string>
#include <omp.h>
#include "g2d.h"
#include "gentle.h"

using namespace std;

extern fs::path fpn_log;
extern projectFile prj;
extern domaininfo di;
extern domainCell** dmcells;
extern cvatt* cvs;
extern cvattAdd* cvsAA;
extern vector<rainfallinfo> rf;

extern globalVinner gvi[1];
extern thisProcessInner psi;

int setRainfallinfo()
{
	rf.clear();
	int rf_order = 0;
	if (_access(prj.rainfallFPN.c_str(), 0) == 0) {
		vector<string> Lines;
		Lines = readTextFileToStringVector(prj.rainfallFPN);
		COleDateTime olet_start;
		if (prj.isDateTimeFormat == 1) {
			tm  t = stringToDateTime2(prj.startDateTime);
			olet_start = COleDateTime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, 0);
		}
		for (int nl = 0; nl < Lines.size(); ++nl)
		{
			rainfallinfo ar;
			if (trim(Lines[nl]) == "") {
				return 1;
			}
			rf_order++;
			ar.order = rf_order;
			switch (prj.rainfallDataType)
			{
			case  rainfallDataType::TextFileMAP:
				if (isNumeric(Lines[nl]) == true) {
					ar.rainfall = Lines[nl];
					ar.dataFile = prj.rainfallFPN;
				}
				else {
					string outstr = "Rainfall data (" + Lines[nl] + ") in "
						+ prj.rainfallFPN + " is invalid.\n";
					writeLog(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			case rainfallDataType::TextFileASCgrid:
				if (Lines[nl] != "" && _access(Lines[nl].c_str(), 0) == 0) {
					ar.rainfall = Lines[nl];
					ar.dataFile = ar.rainfall;
				}
				else {
					string outstr = "Rainfall file (" + Lines[nl] + ") in "
						+ prj.rainfallFPN + " is invalid.\n";
					writeLog(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			}
			if (prj.isDateTimeFormat == 1) {
				COleDateTime olet;
				olet = olet_start + COleDateTimeSpan(0, 0, prj.rainfallDataInterval_min * nl, 0);
				ar.dataTime = timeToString_yyyymmdd_HHclnMMclnSS(olet, -1);
			}
			else {
				ar.dataTime = to_string(prj.rainfallDataInterval_min * nl);
			}
			rf.push_back(ar);
		}
	}
	else {
		string strout = "Rainfall file (" + prj.rainfallFPN + ") is not exist.\n";
		writeLog(fpn_log, strout, 1, 1);
		return -1;
	}
	writeLog(fpn_log, "Setting rainfall data was completed. \n",
		prj.writeLog, prj.writeLog);
	return 1;
}


int readRainfallAndGetIntensity(int rforder)
{
	if ((rforder - 1) < (int)rf.size())//강우자료 있으면, 읽어서 세팅
	{
		double inRF_mm = 0;
		int rfIntervalSEC = prj.rainfallDataInterval_min * 60;
		rainfallDataType rftype = prj.rainfallDataType;
		rainfallinfo arf = rf[rforder - 1];
		switch (rftype)
		{
		case rainfallDataType::TextFileMAP:
			inRF_mm = stof(arf.rainfall);
			if (inRF_mm > 0) {
				psi.rfisGreaterThanZero = 1;
			}
			else {
				inRF_mm = 0.0;
			}
			psi.rfReadintensityForMAP_mPsec = inRF_mm / 1000.0f / rfIntervalSEC;
			// 우선 여기에 저장했다가, cvs 초기화 할때 셀별로 배분한다. 시간 단축을 위해서
			break;
		case rainfallDataType::TextFileASCgrid:
			string rfFpn = rf[rforder - 1].dataFile;
			ascRasterFile ascf = ascRasterFile(rfFpn);
			omp_set_num_threads(gvi[0].mdp);
			//int nchunk = gvi[0].nRows / gvi[0].mdp;
#pragma omp parallel for schedule(guided)//, nchunk) 
			for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
				int nr = cvs[i].rowy;
				int nc = cvs[i].colx;
				inRF_mm = ascf.valuesFromTL[nc][nr];
				if (inRF_mm <= 0) {
					cvsAA[i].rfReadintensity_mPsec = 0.0;
				}
				else {
					cvsAA[i].rfReadintensity_mPsec = inRF_mm / 1000.0 / rfIntervalSEC;
					psi.rfisGreaterThanZero = 1;
				}
			}
			//for (int nr = 0; nr < gvi[0].nRows; ++nr) {
			//	for (int nc = 0; nc < gvi[0].nCols; nc++) {
			//		if (dmcells[nc][nr].isInDomain == 1) {
			//			int idx = dmcells[nc][nr].cvid;
			//			inRF_mm = (float)ascf.valuesFromTL[nc][nr];
			//			if (inRF_mm <= 0) {
			//				cvsAA[idx].rfReadintensity_mPsec = 0.0;
			//			}
			//			else {
			//				cvsAA[idx].rfReadintensity_mPsec = inRF_mm / 1000.0f / (float)rfIntervalSEC;
			//				psi.rfisGreaterThanZero = 1;
			//			}
			//		}
			//	}
			//}
			break;
		}
		return -1;
	}
	return 1;
}

