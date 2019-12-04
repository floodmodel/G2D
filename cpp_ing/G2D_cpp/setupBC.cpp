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
extern domainCell** dmcells;
extern vector<rainfallinfo> rf;
extern bcinfo* bcs;

int initBCinfo()
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
	if (prj.bcCellCountAll > 0)
	{
		bcs = new bcinfo[prj.bcCellCountAll];
		int idx = 0;
		for (int i = 0; i < prj.bcCount; i++)
		{
			vector<cellPosition> aBCcells = prj.bcCellXY[i];
			for (int ci = 0; ci < aBCcells.size(); ci++)
			{
				cellPosition ac = aBCcells[ci];
				bcs[idx].cvid = dmcells[ac.x][ac.y].cvid;
				switch (prj.bcDataType[i])
				{
				case conditionType::Discharge:
					bcs[idx].bctype = 1;
					break;
				case conditionType::Depth:
					bcs[idx].bctype = 2;
					break;
				case conditionType::Height:
					bcs[idx].bctype = 3;
					break;
				case conditionType::NoneCD:
					bcs[idx].bctype = 0;
					break;
				default:
					bcs[idx].bctype = 0;
					break;
				}
				idx++;
			}
		}
	}
	return 1;
}
