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
extern domaininfo di;
extern vector<rainfallinfo> rf;
extern bcinfo* bcs;

int checkBCcellLocation()
{
	int nc = 0;
	for (int i = 0; i < prj.bcCount; i++)
	{
		vector<cellPosition> aBCcells = prj.bcCellXY[i];
		for (int n = 0; n < aBCcells.size(); n++)
		{
			cellPosition ac = aBCcells[n];
			if (ac.x > di.nCols - 1 || ac.x < 0)
			{
				writeLog(fpn_log, "Boundary condition cell x (col) poistion is invalid!! (0 ~ " 
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
			if (ac.y > di.nRows - 1 || ac.y < 0)
			{
				writeLog(fpn_log, "Boundary condition cell y (row) poistion is invalid!! (0 ~ " 
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
		}
		nc += 1;
	}
	prj.bcCellCountAll = nc;
	bcs = new bcinfo[prj.bcCellCountAll];
	return 1;
}

int initBCinfo()
{
	int idx = 0;
	for (int i = 0; i < prj.mBCCount; i++)
	{
		for (int ci = 0; ci < prj.bcData[i].bcCells.Count; ci++)
		{
			bsc[idx].cvid = prj.domain.dminfo[prj.bcData[i].bcCells[ci].x, prj.bcData[i].bcCells[ci].y].cvid;

			switch (prj.bcData[i].conditionDataType)
			{
			case cVars.ConditionDataType.Discharge:
				bcinfo[idx].bctype = 1;
				break;
			case cVars.ConditionDataType.Depth:
				bcinfo[idx].bctype = 2;
				break;
			case cVars.ConditionDataType.Height:
				bcinfo[idx].bctype = 3;
				break;
			case cVars.ConditionDataType.None:
				bcinfo[idx].bctype = 0;
				break;
			}
			idx++;
		}
	}
	return 1;
}
