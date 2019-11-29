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
	if (access(prj.rainfallFile.c_str(), 0) == 0)
	{
		vector<string> Lines;
		Lines =readTextFileToStringVector(prj.rainfallFile);
		tm  t;
		COleDateTime olet;

		if (prj.isDateTimeFormat == 1)
		{
			//t = stringToDateTime(prj.startDateTime);
			t = stringToDateTime2(prj.startDateTime);
			olet = COleDateTime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, 0);
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
				ar.rainfall = Lines[nl];
				ar.dataFile = prj.rainfallFile;
				break;
			case rainfallDataType::TextFileASCgrid: 
				ar.rainfall = Lines[nl];
				ar.dataFile = ar.rainfall;
				break;
			}

			if (prj.isDateTimeFormat==1)
			{
				olet += COleDateTimeSpan(0, 0, prj.rainfallDataInterval_min*nl, 0);
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
		string strout = "Rainfall file (" + prj.rainfallFile + ") is not exist.\n";
		writeLog(fpn_log, strout, 1, 1);
		return -1;
	}
	return 1;
}
