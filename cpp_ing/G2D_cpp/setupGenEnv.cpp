#include <stdio.h>
#include "g2d.h"

using namespace std;

extern generalEnv genEnv;

int setGenEnv()
{
	genEnv.modelSetupIsNormal = 1;
	genEnv.gravity = 9.81; // 1;
	genEnv.dMinLimitforWet_ori = 0.000001;// 0.00001;// 이게 0이면, 유량 계산시 수심으로 나누는 부분에서 발산. 유속이 크게 계산된다..
									// 이 값은 1. 주변셀과의 흐름 계산을 할 셀(effective 셀) 결정시 사용되고,
									//            2. 이 값보다 작은 셀은 이 셀에서 외부로의 유출은 없게 된다. 외부에서 이 셀로의 유입은 가능
									//            3. 생성항(강우, 유량 등)에 의한 유량 추가는 가능하다.
	genEnv.dMinLimitforWet = genEnv.dMinLimitforWet_ori;
	//slpMinLimitforFlow = 0.0001; //음해
	genEnv.slpMinLimitforFlow = 0;// 양해

	if (genEnv.isAnalyticSolution == true)
	{
		genEnv.dtMaxLimit_sec = 2;// 600; //해석해 하고 비교할때는 1 이 아주 잘 맞는다..
		genEnv.dtMinLimit_sec = 1;// 0.1; 
		genEnv.dtStart_sec = genEnv.dtMinLimit_sec;// 0.1;//1 ;
	}
	else
	{
		genEnv.dtMaxLimit_sec = 30;// 600;
		genEnv.dtMinLimit_sec = 0.01;
		genEnv.dtStart_sec = genEnv.dtMinLimit_sec;
	}
	genEnv.convergenceConditionh = 0.00001;// 양해 0.00001;//0.00001; // 0.00000001; //
	genEnv.convergenceConditionhr = 0.001;// 양해 0.00001;//0.00001; // 0.00000001; //
	genEnv.convergenceConditionq = 0.0001;//0.0000001;//0.00001; //0.1% 
	//maxDegreeParallelism = Environment.ProcessorCount * 2;
	genEnv.dflowmaxInThisStep = -9999;
	genEnv.vmaxInThisStep = -9999;
	genEnv.VNConMinInThisStep = DBL_MAX;

	return 1;
}