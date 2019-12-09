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
		   if (ge.isAnalyticSolution == -1){ // �ؼ��ؿ� ���Ҷ��� �̰� �ּ�ó��. 
			   valueGroup.push_back(0); //�׻� 0���� �����ϰ� �Ѵ�. �ް��� ������ȭ�� ���� ���ؼ�,, ������� �ϸ��ϰ� ���Ѵ�. 
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
			//�� ������ �����Ͱ� 0.1~0.3���� 3���� ���� ���, ���Ǵ� 0~0.3���� 4���� �ڷḦ �̿��Ѵ�.                        
			//dataorder�� 1���� �̰�, 1��° ������(0�� index)�� ������ 0�̴�, (values ����Ʈ �� ä�ﶧ 0�� ���� ���� �־��� ������..)
			//dataorder 4�� vcurOrder= 0.3, vnextOrder=0 �̴�. 
			vector<double> values = prj.bcValues[sc];
			if ((dataOrder) <= values.size()) {
				vcurOrder = values[dataOrder - 1] / ndiv;
			}
			double vnextOrder = 0;
			if ((dataOrder) <= values.size() - 1) {// �̰� �������ڷ� ���� ����ϰ�, �� ���Ĵ� 0���� ó��
				vnextOrder = values[dataOrder] / ndiv;
			}
			//if (dataOrder == cdInfo[sc].bcValues.Length + 1)
			//{
			//    vcurOrder = cdInfo[sc].bcValues[dataOrder - 2]; //�̰� ������ �ڷḦ �ѹ� �� �̿��ϴ� ��..
			//}
			cvsAA[idx].bcData_curOrder = vcurOrder;
			cvsAA[idx].bcData_nextOrder = vnextOrder;
			cvsAA[idx].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
		}
	}
}
