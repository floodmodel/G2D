
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <filesystem>
#include <io.h>

#include "gentle.h"
#include "g2d.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

void g2dHelp();

fs::path fpn_prj;
fs::path fpn_log;
fs::path fp_prj;

projectFile prj;
generalEnv genEnv;
domaininfo di;
domainCell **dmcells;
cvatt * cvs;
cvattAdd * cvsAA;
vector<rainfallinfo> rf;


int main(int argc, char **args)
{
	string exeName = "G2D";
	version g2dVersion = getCurrentFileVersion();
	char outString[200];
	sprintf_s(outString, "G2D v.%d.%d.%d. Built in %s.\n", g2dVersion.major, g2dVersion.minor,
		g2dVersion.build, g2dVersion.LastWrittenTime);
	printf(outString);

	long elapseTime_Total_sec;
	clock_t  finish_Total, start_Total;
	start_Total = clock();

	if (argc == 1)
	{
		printf("G2D project file was not entered.");
		g2dHelp();
		return -1;
	}
	if (args[1] == "/?")
	{
		g2dHelp();
		return -1;
	}

	int nResult = _access(args[1], 0);

	if (nResult == -1)
	{
		printf("G2D project file(%s) is invalid.", args[1]);
		return -1;
	}
	else if (nResult == 0)
	{
		fpn_prj = fs::path(args[1]);
		fp_prj = fpn_prj.parent_path();
		fpn_log = fpn_prj;
		fpn_log = fpn_log.replace_extension(".log");
		writeNewLog(fpn_log, outString, 1, -1);
		if (openPrjSetupRunG2D() == -1)
		{
			writeNewLog(fpn_log, "Model setup failed !!!\n", 1, 1);
			return -1;
		}
	}

	finish_Total = clock();
	elapseTime_Total_sec = (long)(finish_Total - start_Total) / CLOCKS_PER_SEC;
	tm ts_total = secToHHMMSS(elapseTime_Total_sec);
	sprintf_s(outString, "Simulation was completed. Run time : %dhrs %dmin %dsec.\n",
		ts_total.tm_hour, ts_total.tm_min, ts_total.tm_sec);
	writeLog(fpn_log, outString, 1, 1);

	//_getch();

	delete[] dmcells;
}


int openPrjSetupRunG2D()
{
	char outString[200];
	sprintf_s(outString, "G2D was started.\n");
	writeLog(fpn_log, outString, 1, 1);

	if (openProjectFile() < 0)
	{
		sprintf_s(outString, "Open %s was failed.\n", fpn_prj.string().c_str());
		writeLog(fpn_log, outString, 1, 1);
		return -1;
	}

	sprintf_s(outString, "%s project was opened.\n", fpn_prj.string().c_str());
	writeLog(fpn_log, outString, 1, 1);

	if (prj.isParallel == 1)
	{
		string usingGPU = "false";
		if (prj.usingGPU == 1) { usingGPU = "true"; }
		sprintf_s(outString, "Parallel : true. Max. degree of parallelism : %d. Using GPU : %s\n",
			prj.maxDegreeOfParallelism, usingGPU.c_str());
		writeLog(fpn_log, outString, 1, 1);

		string cpuinfo = getCPUinfo();
		//char * aaa = stringToCharP(cpuinfo); 
		writeLog(fpn_log, cpuinfo, 1, 1);

		if (prj.usingGPU == 1)
		{
			string gpuinfo = getGPUinfo();
			writeLog(fpn_log, gpuinfo, 1, 1);
			sprintf_s(outString, "Threshold number of effective cells to convert to GPU calculation : %d\n",
				prj.effCellThresholdForGPU);
			writeLog(fpn_log, outString, 1, 1);
		}

	}
	else
	{
		sprintf_s(outString, "Parallel : false. Using GPU : false\n");
		writeLog(fpn_log, outString, 1, 1);
	}

	if (setGenEnv() < 0) { return -1; }

	sprintf_s(outString, "iGS(all cells) max : %d, iNR(a cell) max : %d, tolerance : %f\n",
		prj.maxIterationAllCellsOnCPU, prj.maxIterationACellOnCPU, genEnv.convergenceConditionh);
	writeLog(fpn_log, outString, 1, 1);

	if (setupDomainAndCVinfo() < 0) { return -1; }

	if (prj.isRainfallApplied == 1)
	{
		setRainfallinfo();
	}

	//mProject.rainfall = new cRainfall();
	//if (mProject.isRainfallApplied == 1)
	//{
	   // rainfall.setValues(mProject);
	//}
	//else
	//{
	   // rainfall.rainfallinterval_min = 0;
	//}







	sprintf_s(outString, "%s  -> Model setup was completed.\n", fpn_prj.string().c_str());
	writeLog(fpn_log, outString, 1, 1);

	if (deleteAlloutputFiles() == -1) { return -1; }
	writeLog(fpn_log, "Calculation using CPU was started.\n", 1, 1);







	return 1;
}


void g2dHelp()
{
	printf("\n");
	printf(" Usage : g2d.exe [The full path and name of the current project file to simulate]\n");
	printf("\n");
	printf("** ���� (in Korean)\n");
	printf("  1. G2D ������ �Է��ڷ�(���������ڷ�, �������, ���� ��)�� �غ��ϰ�,\n");
	printf("     project ����(.g2p)�� �ۼ��Ѵ�.\n");
	printf("  2. �𵨸� ��� ������Ʈ �̸��� argument�� �Ͽ� �����Ѵ�.\n");
	printf("     - Console���� [G2D.exe argument] �� �����Ѵ�.\n");
	printf("     ** ���ǻ��� : ��Ⱓ ������ ���, ��ǻ�� ������Ʈ�� ����, ����� �� ������, \n");
	printf("                       ��Ʈ��ũ�� �����ϰ�, �ڵ�������Ʈ ���� �������� �����Ѵ�.\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          ����\n");
	printf("      - ������Ʈ ���ϰ�ο� �̸�\n");
	printf("        G2D ���� ���ϴ����� �����Ų��.\n");
	printf("        �̶� full path, name�� �־�� ������, \n");
	printf("        ��� ������Ʈ ������ G2D.exe ���ϰ� ������ ������ ���� ��쿡��,\n");
	printf("                 path�� �Է����� �ʾƵ� �ȴ�.\n");
	printf("         ��� ������Ʈ �̸��� ��ο� ������ ���Ե� ��� ū����ǥ�� ��� �Է��Ѵ�.\n\n");
	printf("          ** ����(G2D.exe�� d://G2Drun�� ���� ���)\n");
	printf("              - Case 1. G2D.exe�� �ٸ� ������ ������Ʈ ������ ���� ���\n");
	printf("                d://G2Drun>G2D.exe D://G2DTest//TestProject//test.g2p\n\n");
	printf("              - Case 2. G2D.exe�� ���� ������ ������Ʈ ������ ���� ���\n");
	printf("                d://G2Drun>G2D.exe test.gmp\n");
	printf("\n");
	printf("** land cover vat file \n");
	printf("   - the first value is grid value, the second is land cover name,\n");
	printf("     and the third is roughness coefficient.\n");
	printf("\n");
	printf("\n");
	printf("** Usage (in English)\n");
	printf("  1. Prepare input data (geospatial data, boundary condition, rainfall, etc.) of G2D model\n");
	printf("     and create a project file (.g2p).\n");
	printf("  2. Run the G2D model using the project file (.g2p) as an argument.\n");
	printf("     - Run [G2D.exe  argument] in the console window.\n");
	printf("     ** NOTICE: If you simulate for a long time, it may be shut down due to computer update.\n");
	printf("           So, it is safe to disconnect the network and set the computer not to update automatically.\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          Help\n");
	printf("      - Project file path and name\n");
	printf("        Run the G2D model on file-by-file basis.\n");
	printf("        At this time, full path and name should be used, \n");
	printf("            but if the target project file is in the same folder as G2D.exe file,\n");
	printf("            you do not need to input the file path.\n");
	printf("        If the target project name and path contain spaces, enclose them in double quotes.\n\n");
	printf("          ** Examples (when G2D.exe is in d://G2Drun)\n");
	printf("              - Case 1. The project file is in a folder other than G2D.exe\n");
	printf("                d://G2Drun>g2d.exe D://G2DTest//TestProject//test.g2p\n");
	printf("              - Case 2. The project file is in the same folder as G2D.exe\n\n");
	printf("                d://G2Drun>g2d.exe test.gmp\n");
	printf("\n");
	printf("** land cover vat file\n ");
	printf("   - the first value is grid value, the second is land cover name,\n");
	printf("     and the third is roughness coefficient.\n");
}