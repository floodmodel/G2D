#include "stdafx.h"
#include "g2d.h"
#include "gentle.h"

//using namespace std;

extern fs::path fpn_log;
extern projectFile prj;
extern domaininfo di;
extern domainCell** dmcells;
extern cvatt* cvs;
extern cvattAddAtt* cvsAA;
extern double* rfi_read_mPs;
extern vector<rainfallinfo> rf;

extern globalVinner gvi;
extern thisProcessInner psi;
extern thisProcess ps;

int setRainfallinfo()
{
	rf.clear();
	int rf_order = 0;
	if (_access(prj.rainfallFPN.c_str(), 0) == 0) {
		vector<string> Lines;
		Lines = readTextFileToStringVector(prj.rainfallFPN);
		//COleDateTime olet_start;
		//tm t_start;
		//if (prj.isDateTimeFormat == 1) {
		//	tm  t = stringToDateTime2(prj.startDateTime, true);
		//	//olet_start = COleDateTime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, 0);
		//}
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
			case  weatherDataType::Mean:
				if (isNumeric(Lines[nl]) == true) {
					ar.rainfall = Lines[nl];
					ar.dataFile = prj.rainfallFPN;
				}
				else {
					string outstr = "Rainfall data (" + Lines[nl] + ") in "
						+ prj.rainfallFPN + " is invalid.\n";
					writeLogString(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			case weatherDataType::Raster_ASC:
				if (Lines[nl] != "" && _access(Lines[nl].c_str(), 0) == 0) {
					ar.rainfall = Lines[nl];
					ar.dataFile = ar.rainfall;
				}
				else {
					string outstr = "Rainfall file (" + Lines[nl] + ") in "
						+ prj.rainfallFPN + " is invalid.\n";
					writeLogString(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			}
			if (prj.isDateTimeFormat == 1) {
				//COleDateTime olet;
				//olet = olet_start + COleDateTimeSpan(0, 0, prj.rainfallDataInterval_min * nl, 0);
				//tm t_current;
				//t_current = 
				//ar.dataTime = timeToString(olet, false, 
				//	dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
				timeUnitToShow tUnit = timeUnitToShow::toM; // default는 분단위 까지
				timeElaspedToDateTimeFormat2(prj.startDateTime,
					prj.rainfallDataInterval_min * nl, tUnit,
					dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);

			}
			else {
				ar.dataTime = to_string(prj.rainfallDataInterval_min * nl);
			}
			rf.push_back(ar);
		}
	}
	else {
		string strout = "Rainfall file (" + prj.rainfallFPN + ") is not exist.\n";
		writeLogString(fpn_log, strout, 1, 1);
		return -1;
	}
	writeLogString(fpn_log, "Setting rainfall data was completed. \n",
		prj.writeLog, prj.writeLog);
	return 1;
}

int readRainfallAndGetIntensity(int rforder)
{
	if ((rforder - 1) < (int)rf.size())//강우자료 있으면, 읽어서 세팅
	{
		double rfIntervalSEC = prj.rainfallDataInterval_min * 60.0;
		weatherDataType rftype = prj.rainfallDataType;
		if (rftype == weatherDataType::Mean) {
			double inRF_MEAN_mm = 0.0;
			inRF_MEAN_mm = stof(rf[rforder - 1].rainfall);
			if (inRF_MEAN_mm <= 0) {
				psi.rfReadintensityForMAP_mPs = 0.0;
			}
			else {
				psi.rfAccMAP += inRF_MEAN_mm;
				if (prj.initialRFLoss > 0.0 && psi.saturatedByMAP == 0) {
					if (psi.rfAccMAP < prj.initialRFLoss) {
						inRF_MEAN_mm = 0.0;
					}
					else {
						//inRF_mm = fmod(psi.rfAccMAP, prj.initialRFLoss);// 2022.09.13 이거 주석처리, 아래 줄로 대체
						inRF_MEAN_mm = psi.rfAccMAP - prj.initialRFLoss;
						psi.saturatedByMAP = 1;// 여기 들어오면 초기손실 이상의 강우이다. 
					}
				}
				psi.rfReadintensityForMAP_mPs = inRF_MEAN_mm / 1000.0 / rfIntervalSEC;
			}
			// 우선 여기에 저장했다가, cvs 초기화 할때 셀별로 배분한다. 시간 단축을 위해서
		}
		else if (rftype == weatherDataType::Raster_ASC) {
			ascRasterFile ascf = ascRasterFile(rf[rforder - 1].dataFile);
			omp_set_num_threads(ps.mdp);
			//int nchunk = gvi.nRows / gvi.mdp;
#pragma omp parallel for //schedule(guided)//, nchunk) 
			for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
				double inRF_RASTER_mm = 0.0;
				inRF_RASTER_mm = ascf.valuesFromTL[cvs[i].colx][cvs[i].rowy];
				if (inRF_RASTER_mm <= 0.0) {
					rfi_read_mPs[i] = 0.0;
				}
				else {
					cvsAA[i].rfAccCell += inRF_RASTER_mm;
					if (prj.initialRFLoss > 0.0 && cvsAA[i].saturatedByCellRF == 0) {
						if (cvsAA[i].rfAccCell < prj.initialRFLoss) {
							inRF_RASTER_mm = 0.0;
						}
						else {
							//inRF_mm = fmod(cvsAA[i].rfAccCell, prj.initialRFLoss); // 2022.09.13 이거 주석처리, 아래 줄로 대체
							inRF_RASTER_mm = cvsAA[i].rfAccCell - prj.initialRFLoss;
							if (inRF_RASTER_mm < 0.0) {
								inRF_RASTER_mm = 0.0;
							}
							cvsAA[i].saturatedByCellRF = 1;// 여기 들어온 후에는 초기손실 이상의 강우이다. 
						}
					}
					rfi_read_mPs[i] = inRF_RASTER_mm / 1000.0 / rfIntervalSEC;
				}
			}
		}
		return 0;
	}
	return 1;
}


