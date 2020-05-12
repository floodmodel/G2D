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
extern cvatt* cvs;;
extern cvattAdd* cvsAA;;
extern vector<rainfallinfo> rf;
extern map <int, bcAppinfo> bcApp; 
extern globalVinner gvi[1];
extern thisProcessInner psi;;
extern thisProcess ps;

int setBCinfo()
{
	int nc = 0;
	for (int i = 0; i < prj.bcCount; i++) {
		vector<cellPosition> aBCcells = prj.bcis[i].bcCellXY;
		for (int n = 0; n < aBCcells.size(); n++) {
			cellPosition ac = aBCcells[n];
			if (ac.xCol > di.nCols - 1 || ac.xCol < 0) {
				writeLog(fpn_log, "Boundary condition cell x (col) poistion is invalid!! (0 ~ "
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
			if (ac.yRow > di.nRows - 1 || ac.yRow < 0) {
				writeLog(fpn_log, "Boundary condition cell y (row) poistion is invalid!! (0 ~ "
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
			nc += 1;
		}
	}

	prj.bcCellCountAll = nc;
	if (prj.bcCellCountAll > 0) {
		bcApp.clear();
		prj.bcCVidxList.clear();
		for (int i = 0; i < prj.bcCount; ++i) {
			vector <double> valuesFromAFile = readTextFileToDoubleVector(prj.bcis[i].bcDataFile);
			vector <double> valueGroup;
			if (!isAS) { // �ؼ��ؿ� ���Ҷ��� �̰� ���� ����. 
				valueGroup.push_back(0); //�׻� 0���� �����ϰ� �Ѵ�. �ް��� ������ȭ�� ���� ���ؼ�,, ������� �ϸ��ϰ� ���Ѵ�. 
			}
			valueGroup.insert(valueGroup.end(), valuesFromAFile.begin(), valuesFromAFile.end());
			prj.bcis[i].bcValues=valueGroup;
			vector<cellPosition> aBCcells = prj.bcis[i].bcCellXY;
			for (int ci = 0; ci < aBCcells.size(); ++ci) {
				cellPosition ac = aBCcells[ci];
				int idx = dmcells[ac.xCol][ac.yRow].cvidx;
				bcApp[idx].cvidx = idx;
				prj.bcCVidxList.push_back(idx);
				cvs[idx].isBCcell = 1;
				switch (prj.bcis[i].bcDataType) {  //Discharge : 1, Depth : 2, Height : 3, NoneCD : 0
				case conditionDataType::Discharge:
					bcApp[idx].bctype = 1;
					break;
				case conditionDataType::Depth:
					bcApp[idx].bctype = 2;
					break;
				case conditionDataType::Height:
					bcApp[idx].bctype = 3;
					break;
				case conditionDataType::NoneCD:
					bcApp[idx].bctype = 0;
					break;
				default:
					bcApp[idx].bctype = 0;
					break;
				}
			}
		}
	}
	return 1;
}

void getCellCD(int dataOrder, int dataInterval_min)
{
	for (int sc = 0; sc < prj.bcCount; ++sc) {
		int ndiv;
		vector<cellPosition> cellgroup = prj.bcis[sc].bcCellXY;
		int cellCount = (int)cellgroup.size();
		if (prj.bcis[sc].bcDataType == conditionDataType::Discharge) {
			ndiv = cellCount;
		}
		else { ndiv = 1; }
		for (int nc = 0; nc < cellCount; ++nc) {
			int cx = cellgroup[nc].xCol;
			int ry = cellgroup[nc].yRow;
			int idx = dmcells[cx][ry].cvidx;
			double vcurOrder = 0;
			//�� ������ �����Ͱ� 0.1~0.3���� 3���� ���� ���, ���Ǵ� 0~0.3���� 4���� �ڷḦ �̿��Ѵ�.                        
			//dataorder�� 1���� �̰�, 1��° ������(0�� index)�� ������ 0�̴�, (values ����Ʈ �� ä�ﶧ 0�� ���� ���� �־��� ������..)
			//dataorder 4�� vcurOrder= 0.3, vnextOrder=0 �̴�. 
			vector<double> values = prj.bcis[sc].bcValues;
			if ((dataOrder) <= values.size() && dataOrder > 0) {
				vcurOrder = values[dataOrder - 1] / (double)ndiv;
			}
			double vnextOrder = 0;
			if ((dataOrder) <= values.size() - 1) {// �̰� �������ڷ� ���� ����ϰ�, �� ���Ĵ� 0���� ó��
				vnextOrder = values[dataOrder] / (double)ndiv;
			}
			cvsAA[idx].bcData_curOrder = vcurOrder;
			cvsAA[idx].bcData_nextOrder = vnextOrder;
			cvsAA[idx].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
		}
	}
}

double getCDasDepthWithLinear(int bctype, double elev_m,
	double dx, int i_cvaa)
{
	cvattAdd cvaa = cvsAA[i_cvaa];
	double vcurOrder = cvaa.bcData_curOrder;
	double vnextOrder = cvaa.bcData_nextOrder;
	double valueAsDepth_curOrder = 0;
	double valueAsDepth_nextOrder = 0;
	//1:  Discharge,  2: Depth, 3: Height,  4: None
	switch (bctype)
	{
	case 1://conditionDataType::Discharge:
		valueAsDepth_curOrder = (vcurOrder / dx / dx) * psi.dt_sec;
		valueAsDepth_nextOrder = (vnextOrder / dx / dx) * psi.dt_sec;
		break;
	case 2://conditionDataType::Depth:
		valueAsDepth_curOrder = vcurOrder;
		valueAsDepth_nextOrder = vnextOrder;
		break;
	case 3://conditionDataType::Height:
		valueAsDepth_curOrder = vcurOrder - elev_m;
		valueAsDepth_nextOrder = vnextOrder - elev_m;
		break;
	}
	if (valueAsDepth_curOrder < 0) { valueAsDepth_curOrder = 0; }
	if (valueAsDepth_nextOrder < 0) { valueAsDepth_nextOrder = 0; }
	double bcDepth_dt_m_tp1 = 0.0;
	if (!isAS)  { // �ؼ��� �׽�Ʈ�� �ƴҶ��� �� ���� ���
		bcDepth_dt_m_tp1 = (valueAsDepth_nextOrder - valueAsDepth_curOrder)
			* (ps.tnow_sec - cvaa.bcData_curOrderStartedTime_sec) / ps.dtbc_sec
			+ valueAsDepth_curOrder;
	}
	else {
		bcDepth_dt_m_tp1 = valueAsDepth_curOrder; // �ؼ��� �׽�Ʈ�� �� ����
	}
	return  bcDepth_dt_m_tp1;
}
