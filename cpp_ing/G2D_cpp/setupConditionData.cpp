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
extern bcCellinfo* bci;

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
		bci = new bcCellinfo[prj.bcCellCountAll];
		int idx = 0;
		for (int i = 0; i < prj.bcCount; i++)
		{
		   vector <float> valuesFromAFile= readTextFileToFloatVector(prj.bcDataFile[i]);
		   vector <float> valueGroup;
		   if (ge.isAnalyticSolution == -1){ // �ؼ��ؿ� ���Ҷ��� �̰� ���� ����. 
			   valueGroup.push_back(0); //�׻� 0���� �����ϰ� �Ѵ�. �ް��� ������ȭ�� ���� ���ؼ�,, ������� �ϸ��ϰ� ���Ѵ�. 
		   }
		   valueGroup.insert(valueGroup.end(), valuesFromAFile.begin(), valuesFromAFile.end());
		   prj.bcValues.push_back(valueGroup);
		   vector<cellPosition> aBCcells = prj.bcCellXY[i];
			for (int ci = 0; ci < aBCcells.size(); ci++)
			{
				cellPosition ac = aBCcells[ci];
				bci[idx].cvid = dmcells[ac.x][ac.y].cvid;
				switch (prj.bcDataType[i])
				{
				case conditionDataType::Discharge:
					bci[idx].bctype = 1;
					break;
				case conditionDataType::Depth:
					bci[idx].bctype = 2;
					break;
				case conditionDataType::Height:
					bci[idx].bctype = 3;
					break;
				case conditionDataType::NoneCD:
					bci[idx].bctype = 0;
					break;
				default:
					bci[idx].bctype = 0;
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
		int cellCount = (int) cellgroup.size();
		if (prj.bcDataType[sc] == conditionDataType::Discharge) {
			ndiv = cellCount;
		}
		else { ndiv = 1; }
		for (int nc = 0; nc < cellCount; nc++)
		{
			int cx = cellgroup[nc].x;
			int ry = cellgroup[nc].y;
			int idx = dmcells[cx][ry].cvid;
			float vcurOrder = 0;
			//�� ������ �����Ͱ� 0.1~0.3���� 3���� ���� ���, ���Ǵ� 0~0.3���� 4���� �ڷḦ �̿��Ѵ�.                        
			//dataorder�� 1���� �̰�, 1��° ������(0�� index)�� ������ 0�̴�, (values ����Ʈ �� ä�ﶧ 0�� ���� ���� �־��� ������..)
			//dataorder 4�� vcurOrder= 0.3, vnextOrder=0 �̴�. 
			vector<float> values = prj.bcValues[sc];
			if ((dataOrder) <= values.size() && dataOrder>0) {
				vcurOrder = values[dataOrder - 1] / (float) ndiv;
			}
			float vnextOrder = 0;
			if ((dataOrder) <= values.size() - 1) {// �̰� �������ڷ� ���� ����ϰ�, �� ���Ĵ� 0���� ó��
				vnextOrder = values[dataOrder] / (float) ndiv;
			}
			//if (dataOrder == cdInfo[sc].bcValues.Length + 1)
			//{
			//    vcurOrder = cdInfo[sc].bcValues[dataOrder - 2]; //�̰� ������ �ڷḦ �ѹ� �� �̿��ϴ� ��..
			//}
			cvsAA[idx].bcData_curOrder = vcurOrder;
			cvsAA[idx].bcData_nextOrder = vnextOrder;
			cvsAA[idx].bcData_curOrderStartedTime_sec = (int) dataInterval_min * (dataOrder - 1) * 60;
		}
	}
}

int getbcArrayIndex(int cvid)
{
	for (int i = 0; i < prj.bcCellCountAll; i++)
	{
		if (bci[i].cvid == cvid) { return i; }
	}
	return -1;
}


float getConditionDataAsDepthWithLinear(int bctype, float elev_m,
	float dx, cvattAdd cvaa, float dtsec,
	int dtsec_cdata, double nowt_sec)
{
	float vcurOrder = cvaa.bcData_curOrder;
	float vnextOrder = cvaa.bcData_nextOrder;
	float valueAsDepth_curOrder = 0;
	float valueAsDepth_nextOrder = 0;
	//1:  Discharge,  2: Depth, 3: Height,  4: None
	switch (bctype)
	{
	case 1:
		valueAsDepth_curOrder = (float)(vcurOrder / dx / dx) * dtsec;
		valueAsDepth_nextOrder = (float)(vnextOrder / dx / dx) * dtsec;
		break;
	case 2:
		valueAsDepth_curOrder = vcurOrder;
		valueAsDepth_nextOrder = vnextOrder;
		break;
	case 3:
		valueAsDepth_curOrder = vcurOrder - elev_m;
		valueAsDepth_nextOrder = vnextOrder - elev_m;
		break;
	}
	if (valueAsDepth_curOrder < 0) { valueAsDepth_curOrder = 0; }
	if (valueAsDepth_nextOrder < 0) { valueAsDepth_nextOrder = 0; }
	float bcDepth_dt_m_tp1 = 0.0f;
	if (ge.isAnalyticSolution == false) {
		bcDepth_dt_m_tp1 = (valueAsDepth_nextOrder - valueAsDepth_curOrder)
			* (nowt_sec - cvaa.bcData_curOrderStartedTime_sec) / dtsec_cdata
			+ valueAsDepth_curOrder;
	}
	else {
		bcDepth_dt_m_tp1 = valueAsDepth_curOrder;
	}
	return bcDepth_dt_m_tp1;
}
