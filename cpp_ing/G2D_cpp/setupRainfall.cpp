#include <stdio.h>
#include <fstream>
#include <filesystem>
#include <io.h>
#include<ATLComTime.h>
#include <string>
#include "g2d.h"
#include "gentle.h"

using namespace std;

extern fs::path fpn_log;
extern projectFile prj;
extern vector<rainfallinfo> rf;

int setRainfallinfo()
{
	int rf_order = 0;
	if (_access(prj.rainfallFPN.c_str(), 0) == 0)
	{
		vector<string> Lines;
		Lines =readTextFileToStringVector(prj.rainfallFPN);
		COleDateTime olet_start;
		if (prj.isDateTimeFormat == 1)
		{
			tm  t = stringToDateTime2(prj.startDateTime);
			olet_start = COleDateTime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, 0);
		}
		for (int nl = 0; nl < Lines.size(); ++nl)
		{
			rainfallinfo ar;
			if (trim(Lines[nl])=="")
			{
				return 1;
			}
			rf_order++;
			ar.order = rf_order;
			switch (prj.rainfallDataType)
			{
			case  rainfallDataType::TextFileMAP: 
				if ( isNumeric(Lines[nl])== true)
				{
					ar.rainfall = Lines[nl];
					ar.dataFile = prj.rainfallFPN;
				}
				else
				{
					string outstr = "Rainfall data (" + Lines[nl] + ") in " + prj.rainfallFPN + " is invalid.\n";
					writeLog(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			case rainfallDataType::TextFileASCgrid: 
				if (Lines[nl] != ""&& _access(Lines[nl].c_str(), 0) == 0)
				{
					ar.rainfall = Lines[nl];
					ar.dataFile = ar.rainfall;
				}
				else
				{
					string outstr = "Rainfall file (" + Lines[nl] + ") in " + prj.rainfallFPN + " is invalid.\n";
					writeLog(fpn_log, outstr, -1, 1);
					return -1;
				}
				break;
			}

			if (prj.isDateTimeFormat==1)
			{				
				COleDateTime olet;
				olet=olet_start + COleDateTimeSpan(0, 0, prj.rainfallDataInterval_min*nl, 0);
				ar.dataTime = timeToString(olet, -1);
			}
			else
			{
				ar.dataTime = to_string(prj.rainfallDataInterval_min * nl);
			}
			rf.push_back(ar);
		}
	}
	else
	{
		string strout = "Rainfall file (" + prj.rainfallFPN + ") is not exist.\n";
		writeLog(fpn_log, strout, 1, 1);
		return -1;
	}

	writeLog(fpn_log, "Setting rainfall data was completed. \n",
		prj.writeLog, prj.writeLog);
	return 1;
}
