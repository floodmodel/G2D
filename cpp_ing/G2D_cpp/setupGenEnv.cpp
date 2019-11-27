#include <stdio.h>
#include "g2d.h"

using namespace std;

extern generalEnv genEnv;

int setGenEnv()
{
	genEnv.modelSetupIsNormal = 1;
	genEnv.gravity = 9.81; // 1;
	genEnv.dMinLimitforWet_ori = 0.000001;// 0.00001;// �̰� 0�̸�, ���� ���� �������� ������ �κп��� �߻�. ������ ũ�� ���ȴ�..
									// �� ���� 1. �ֺ������� �帧 ����� �� ��(effective ��) ������ ���ǰ�,
									//            2. �� ������ ���� ���� �� ������ �ܺη��� ������ ���� �ȴ�. �ܺο��� �� ������ ������ ����
									//            3. ������(����, ���� ��)�� ���� ���� �߰��� �����ϴ�.
	genEnv.dMinLimitforWet = genEnv.dMinLimitforWet_ori;
	//slpMinLimitforFlow = 0.0001; //����
	genEnv.slpMinLimitforFlow = 0;// ����

	if (genEnv.isAnalyticSolution == true)
	{
		genEnv.dtMaxLimit_sec = 2;// 600; //�ؼ��� �ϰ� ���Ҷ��� 1 �� ���� �� �´´�..
		genEnv.dtMinLimit_sec = 1;// 0.1; 
		genEnv.dtStart_sec = genEnv.dtMinLimit_sec;// 0.1;//1 ;
	}
	else
	{
		genEnv.dtMaxLimit_sec = 30;// 600;
		genEnv.dtMinLimit_sec = 0.01;
		genEnv.dtStart_sec = genEnv.dtMinLimit_sec;
	}
	genEnv.convergenceConditionh = 0.00001;// ���� 0.00001;//0.00001; // 0.00000001; //
	genEnv.convergenceConditionhr = 0.001;// ���� 0.00001;//0.00001; // 0.00000001; //
	genEnv.convergenceConditionq = 0.0001;//0.0000001;//0.00001; //0.1% 
	//maxDegreeParallelism = Environment.ProcessorCount * 2;
	genEnv.dflowmaxInThisStep = -9999;
	genEnv.vmaxInThisStep = -9999;
	genEnv.VNConMinInThisStep = DBL_MAX;

	return 1;
}