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
extern generalEnv ge;
extern domaininfo di;
extern domainCell** dmcells;
extern cvattAdd* cvsAA;;
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
			nc += 1;
		}
	}

	prj.bcCellCountAll = nc;
	if (prj.bcCellCountAll > 0)
	{
		bcis = new bcCellinfo[prj.bcCellCountAll];
		int idx = 0;
		for (int i = 0; i < prj.bcCount; i++)
		{
		   vector <double> valuesFromAFile=readTextFileToDoubleVector(prj.bcDataFile[i]);
		   vector <double> valueGroup;
		   if (ge.isAnalyticSolution == -1){ // 해석해와 비교할때는 이거 주석처리. 
			   valueGroup.push_back(0); //항상 0에서 시작하게 한다. 급격한 수위변화를 막기 위해서,, 수문곡선은 완만하게 변한다. 
		   }
		   valueGroup.insert(valueGroup.end(), valuesFromAFile.begin(), valuesFromAFile.end());
		   prj.bcValues.push_back(valueGroup);
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
		vector<cellPosition> cellgroup = prj.bcCellXY[sc];
		int cellCount = cellgroup.size();
		if (prj.bcDataType[sc] == conditionType::Discharge) {
			ndiv = cellCount;
		}
		else { ndiv = 1; }
		for (int nc = 0; nc < cellCount; nc++)
		{
			int cx = cellgroup[nc].x;
			int ry = cellgroup[nc].y;
			int idx = dmcells[cx][ry].cvid;
			double vcurOrder = 0;
			//이 조건은 데이터가 0.1~0.3까지 3개가 있을 경우, 모의는 0~0.3까지 4개의 자료를 이용한다.                        
			//dataorder는 1부터 이고, 1번째 데이터(0번 index)는 무조건 0이다, (values 리스트 값 채울때 0을 먼저 만들어서 넣었기 때문에..)
			//dataorder 4의 vcurOrder= 0.3, vnextOrder=0 이다. 
			vector<double> values = prj.bcValues[sc];
			if ((dataOrder) <= values.size()) {
				vcurOrder = values[dataOrder - 1] / ndiv;
			}
			double vnextOrder = 0;
			if ((dataOrder) <= values.size() - 1) {// 이건 마지막자료 까지 사용하고, 그 이후는 0으로 처리
				vnextOrder = values[dataOrder] / ndiv;
			}
			//if (dataOrder == cdInfo[sc].bcValues.Length + 1)
			//{
			//    vcurOrder = cdInfo[sc].bcValues[dataOrder - 2]; //이건 마지막 자료를 한번 더 이용하는 것..
			//}
			cvsAA[idx].bcData_curOrder = vcurOrder;
			cvsAA[idx].bcData_nextOrder = vnextOrder;
			cvsAA[idx].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
		}
	}
}
