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
extern bcCellinfo* bcis;

int setBCinfo()
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

	// 여기서 bcValues 설정하자..

	prj.bcCellCountAll = nc;

	if (prj.bcCellCountAll > 0)
	{
		bcis = new bcCellinfo[prj.bcCellCountAll];
		int idx = 0;
		for (int i = 0; i < prj.bcCount; i++)
		{
			vector<cellPosition> aBCcells = prj.bcCellXY[i];
			for (int ci = 0; ci < aBCcells.size(); ci++)
			{
				cellPosition ac = aBCcells[ci];
				bcis[idx].cvid = dmcells[ac.x][ac.y].cvid;
				switch (prj.bcDataType[i])
				{
				case conditionType::Discharge:
					bcis[idx].bctype = 1;
					break;
				case conditionType::Depth:
					bcis[idx].bctype = 2;
					break;
				case conditionType::Height:
					bcis[idx].bctype = 3;
					break;
				case conditionType::NoneCD:
					bcis[idx].bctype = 0;
					break;
				default:
					bcis[idx].bctype = 0;
					break;
				}
				idx++;
			}
		}
	}
	return 1;
}

void getCellConditionData(int dataOrder, int dataInterval_min)
{
	for (int sc = 0; sc < prj.bcCount; sc++)
	{
		int ndiv;
		vector<cellPosition> aBCcells = prj.bcCellXY[sc];
		int cellCount = aBCcells.size();		
		if (prj.bcDataType[sc] == conditionType::Discharge)		{
			ndiv = cellCount;
		}
		else { ndiv = 1; }
		for (int nc = 0; nc < cellCount; nc++)
		{
			int cx = aBCcells[nc].x;
			int ry = aBCcells[nc].y;
			int idx = dm.dminfo[cx, ry].cvid;
			double vcurOrder = 0;
			double vnextOrder = 0;
			//이 조건은 데이터가 0.1~0.3까지 3개가 있을 경우, 모의는 0~0.3까지 4개의 자료를 이용한다.                        
			//dataorder는 1부터 이고, 1번 데이터는 무조건 0이다, 
			//dataorder 4의 vcurOrder= 0.3, vnextOrder=0 이다. 
			if ((dataOrder) <= cdInfo[sc].bcValues.Length)
			{
				vcurOrder = cdInfo[sc].bcValues[dataOrder - 1] / ndiv;
			}
			if ((dataOrder) <= cdInfo[sc].bcValues.Length - 1)
			{
				vnextOrder = cdInfo[sc].bcValues[dataOrder] / ndiv;
			}
			//if (dataOrder == cdInfo[sc].bcValues.Length + 1)
			//{
			//    vcurOrder = cdInfo[sc].bcValues[dataOrder - 2]; //이건 마지막 자료를 한번 더 이용하는 것..
			//}
			cvsadd[idx].bcData_curOrder = vcurOrder;
			cvsadd[idx].bcData_nextOrder = vnextOrder;
			cvsadd[idx].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
		}
	}
}
