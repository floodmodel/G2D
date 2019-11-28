#include <stdio.h>
#include <fstream>
#include <filesystem>
#include <io.h>
#include <process.h>
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
				
				tm t = stringToDateTime(prj.startDateTime);
				CTimeSpan  ts;

				ar.dataTime = 
				//¿©±â¼­ time span
				//ar.dataTime = 
					//.cComTools.GetTimeToPrintOut(true, cGenEnv.eventStartTime, (int)(rainfallinterval_min * nl));
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
