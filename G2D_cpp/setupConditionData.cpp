#include "stdafx.h"
#include "g2d.h"
#include "gentle.h"
#include "g2d_cuda.cuh"

using namespace std;

extern fs::path fpn_log;
extern projectFile prj;
extern generalEnv ge;
extern domaininfo di;
extern domainCell** dmcells;
extern cvatt* cvs;;
extern cvattAddAtt* cvsAA;;
extern bcAppinfo * bcAppinfos; 
extern globalVinner gvi;
extern thisProcessInner psi;;
extern thisProcess ps;

int setBCinfo()
{
	int nc = 0;
	for (int i = 0; i < prj.bcCount; i++) {
		for (int n = 0; n < prj.bcis[i].nCellsInAbc; n++) {
			cellPosition ac = prj.bcis[i].bcCellXY[n];
			if (ac.xCol > di.nCols - 1 || ac.xCol < 0) {
				writeLog(fpn_log, "ERROR : Boundary condition cell x (col) poistion is invalid!! (0 ~ "
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
			if (ac.yRow > di.nRows - 1 || ac.yRow < 0) {
				writeLog(fpn_log, "ERROR : Boundary condition cell y (row) poistion is invalid!! (0 ~ "
					+ to_string(di.nCols - 1) + ").", 1, 1);
				return -1;
			}
			nc += 1;
		}
	}

	prj.bcCellCountAll = nc;
	if (prj.bcCellCountAll > 0) {
		bcAppinfos = new bcAppinfo[nc];
		int bcci = 0; //bc cell indices for all cells
		for (int i = 0; i < prj.bcCount; ++i) {
			vector <double> valuesFromAFile = readTextFileToDoubleVector(prj.bcDataFiles[i]);
			vector <double> valueGroup;
#ifndef isAS // 해석해와 비교할때는 이거 적용 않함. 
			valueGroup.push_back(0); //항상 0에서 시작하게 한다. 급격한 수위변화를 막기 위해서,, 수문곡선은 완만하게 변한다. 
#endif
			valueGroup.insert(valueGroup.end(), valuesFromAFile.begin(), valuesFromAFile.end());
			prj.bcis[i].bcValueCount = valueGroup.size();
			prj.bcis[i].bcValues = new double[prj.bcis[i].bcValueCount];
			copy(valueGroup.begin(), valueGroup.end(), prj.bcis[i].bcValues);
			for (int ci = 0; ci < prj.bcis[i].nCellsInAbc; ++ci) {
				cellPosition ac = prj.bcis[i].bcCellXY[ci];
				int idx = dmcells[ac.xCol][ac.yRow].cvidx;
				if (idx < 0) {
					writeLog(fpn_log, "ERROR : The location of boundary condition cell ["+
						to_string(ac.xCol) + ", "+ to_string(ac.yRow) +"] is invalid.\n", 1, 1);
					return -1;
				}
				bcAppinfos[bcci].cvidx = idx;
				bcAppinfos[bcci].bcDepth_dt_m_tp1 = 0.0;
				cvs[idx].isBCcell = 1;
				switch (prj.bcis[i].bcDataType) {  //Discharge : 1, Depth : 2, water level : 3, NoneCD : 0
				case conditionDataType::Discharge:
					bcAppinfos[bcci].bctype = 1;
					break;
				case conditionDataType::Depth:
					bcAppinfos[bcci].bctype = 2;
					break;
				case conditionDataType::WaterLevel:
					bcAppinfos[bcci].bctype = 3;
					break;
				case conditionDataType::NoneCD:
					bcAppinfos[bcci].bctype = 0;
					break;
				default:
					bcAppinfos[bcci].bctype = 0;
					break;
				}
				++bcci;
			}

		}
	}
	return 1;
}

void getCellCD(int dataOrder, int dataInterval_min)
{
	for (int sc = 0; sc < prj.bcCount; ++sc) {
		bcinfo abci = prj.bcis[sc];
		int ndiv;
		if (abci.bcDataType == conditionDataType::Discharge) {
			ndiv = abci.nCellsInAbc;
		}
		else { ndiv = 1; }
		for (int nc = 0; nc < abci.nCellsInAbc; ++nc) {
			int cx = abci.bcCellXY[nc].xCol;
			int ry = abci.bcCellXY[nc].yRow;
			
			double vcurOrder = 0.0;
			//이 조건은 데이터가 0.1~0.3까지 3개가 있을 경우, 모의는 0~0.3까지 4개의 자료를 이용한다.                        
			//dataorder는 1부터 이고, 1번째 데이터(0번 index)는 무조건 0이다, (values 리스트 값 채울때 0을 먼저 만들어서 넣었기 때문에..)
			//dataorder 4의 vcurOrder= 0.3, vnextOrder=0 이다. 
			if ((dataOrder) <= abci.bcValueCount && dataOrder > 0) {
				vcurOrder = abci.bcValues[dataOrder - 1] / (double)ndiv;
			}
			double vnextOrder = 0.0;
			if ((dataOrder) <= abci.bcValueCount - 1) {// 이건 마지막자료 까지 사용하고, 그 이후는 0으로 처리
				vnextOrder = abci.bcValues[dataOrder] / (double)ndiv;
			}
			int t_curOrderStarted_sec= dataInterval_min * (dataOrder - 1) * 60;
			int idx = dmcells[cx][ry].cvidx;
			int bci = getBcAppinfoidx(bcAppinfos, prj.bcCellCountAll, idx);
			bcAppinfos[bci].bcData_curOrder = vcurOrder;
			bcAppinfos[bci].bcData_nextOrder = vnextOrder;
			bcAppinfos[bci].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
		}
	}   	  
}


