
#include "stdafx.h"
#include "gentle.h"
#include "g2d.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

fs::path fpn_prj;
fs::path fpn_log;
fs::path fp_prj;

projectFile prj;
generalEnv ge;
domaininfo di;
domainCell **dmcells;
cvatt *cvs;
cvattAddAtt *cvsAA;
double* cvsele;
vector<rainfallinfo> rf;
double* rfi_read_mPs;
bcAppinfo* bcAppinfos; 

thisProcess ps; 
thisProcessInner psi;
globalVinner gvi;
minMaxCVidx mnMxCVidx;
dataForCalDT dataForDT;

int main(int argc, char** args)
{
	string exeName = "G2D";
	setlocale(LC_ALL, "korean");
	version g2dVersion = getCurrentFileVersion();
	char outString[200];
	string vString;
	vString = "G2D v." + to_string(g2dVersion.pmajor) + "."
		+ to_string(g2dVersion.pminor) + "."
		+ to_string(g2dVersion.pbuild) + ". File version : "
		+ to_string(g2dVersion.fmajor) + "."
		+ to_string(g2dVersion.fminor) + "."
		+ to_string(g2dVersion.fbuild) + ". Modified in "
		+ g2dVersion.LastWrittenTime + ".\n";
	cout << vString;
	long elapseTime_Total_sec;
	clock_t  finish_Total, start_Total;
	start_Total = clock();
	if (argc == 1) {
		printf("G2D project file was not entered.");
		g2dHelp();
		return -1;
	}
	
	if (argc == 2) {
		string args1(args[1]);
		if (trim(args1) == "/?" || lower(trim(args1)) == "/help"	){
			g2dHelp();
			return -1;
		}
		string fpnG2P = args[1];
		fs::path in_arg = fs::path(fpnG2P.c_str());
		string fp = in_arg.parent_path().string();	
		if (trim(fp) == "") {
			string fpn_exe = getCurrentExeFilePathName();
			fs::path g2dexef = fs::path(fpn_exe.c_str());
			string fp_exe = g2dexef.parent_path().string();
			fpnG2P = fp_exe + "\\" + fpnG2P;
		}
		int nResult = _access(fpnG2P.c_str(), 0);
		if (nResult == -1) {
			printf("G2D project file(%s) is invalid.", args[1]);
			return -1;
		}
		else if (nResult == 0) {
			fpn_prj = fs::path(fpnG2P.c_str());
			fp_prj = fpn_prj.parent_path();
			fpn_log = fpn_prj;
			fpn_log = fpn_log.replace_extension(".log");
			writeNewLog(fpn_log, vString, 1, -1);
			if (openPrjAndSetupModel() == -1) {
				writeNewLog(fpn_log, "Model setup failed !!!\n", 1, 1);
				return -1;
			}
			if (runG2D() != 1) {
				writeNewLog(fpn_log, "An error was occurred while simulation...\n", 1, 1);
				return -1;
			}
		}
	}
	
	if (argc == 3) {
		string args1(args[1]);
		string args2(args[2]);
		if (args1 == "/" && (trim(args2)=="?" || lower(trim(args2)) == "help")) {
			g2dHelp();
			return -1;
		}
	}

	finish_Total = clock();
	elapseTime_Total_sec = (long)(finish_Total - start_Total) / CLOCKS_PER_SEC;
	tm ts_total = secToHHMMSS(elapseTime_Total_sec);
	sprintf_s(outString, "Simulation was completed. Run time : %dhrs %dmin %dsec.\n",
		ts_total.tm_hour, ts_total.tm_min, ts_total.tm_sec);
	writeLog(fpn_log, outString, 1, 1);

	disposeDynamicVars();
	return 1;
}

void disposeDynamicVars()
{
	if (dmcells != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (dmcells[i] != NULL) { delete[] dmcells[i]; }
		}
		delete[] dmcells;
	}
	if (cvs != NULL) { delete[] cvs; }
	if (cvsAA != NULL) { delete[] cvsAA; }
}

int openPrjAndSetupModel()
{
	char outString[200];
	prj.cpusi = getCPUinfo();
	if (openProjectFile() == 0)
	{
		sprintf_s(outString, "Open %s was failed.\n", fpn_prj.string().c_str());
		writeLog(fpn_log, outString, 1, 1);
		return -1;
	}
	writeLog(fpn_log, prj.cpusi.infoString, 1, 1);
	sprintf_s(outString, "%s project was opened.\n", fpn_prj.string().c_str());
	writeLog(fpn_log, outString, 1, 1);

	string usingGPU = "false";
	if (prj.usingGPU == 1) { usingGPU = "true"; }
	string isparallel = "true";
	if (prj.maxDegreeOfParallelism == 1 && usingGPU=="false") { isparallel = "false"; }
	if (prj.usingGPU == 0) {
		sprintf_s(outString, "Parallel : %s. Max. degree of parallelism : %d. Using GPU : %s.\n",
			isparallel.c_str(), prj.maxDegreeOfParallelism, usingGPU.c_str());
	}
	else {
		sprintf_s(outString, "Parallel : true. Using GPU : true. Threads per block : %d\n", prj.threadsPerBlock);
	}
	writeLog(fpn_log, outString, 1, 1);
		if (prj.usingGPU == 1)
	{
		string gpuinfo = getGPUinfo();
		writeLog(fpn_log, gpuinfo, 1, 1);
	}

	if (setGenEnv() < 0) {
		writeLog(fpn_log, "Setting general environment variables was failed.\n", 1, 1);
		return -1;
	}

	sprintf_s(outString, "iGS(all cells) max : %d, iNR(a cell) max : %d, tolerance : %f\n",
		prj.maxIterationAllCellsOnCPU, prj.maxIterationACellOnCPU, CCh);
	writeLog(fpn_log, outString, 1, 1);

	if (setupDomainAndCVinfo() < 0) {
		writeLog(fpn_log, "Setting domain and control volume data were failed.\n", 1, 1);
		return -1;
	}
	if (prj.isRainfallApplied == 1) {
		if (setRainfallinfo() == -1) {
			writeLog(fpn_log, "Setting rainfall data was failed.\n", 1, 1);
			return -1;
		}
	}
	if (prj.isbcApplied == 1) {
		if (setBCinfo() == -1) {
			writeLog(fpn_log, "Setting boundary condition data was failed.\n", 1, 1);
			return -1;
		}
	}
	if (deleteAlloutputFiles() == -1) {
		writeLog(fpn_log, "Deleting previous output files was failed.\n", 1, 1);
		return -1;
	}

	if (initializeOutputArray() == -1) {
		writeLog(fpn_log, "Initialize output arrays was failed.\n", 1, 1);
		return -1;
	}

	sprintf_s(outString, "%s -> Model setup was completed.\n", fpn_prj.string().c_str());
	writeLog(fpn_log, outString, 1, 1);
	sprintf_s(outString, "The number of effecitve cells : %d\n", di.cellNnotNull);
	writeLog(fpn_log, outString, 1, 1);	
	return 1;
}

int runG2D()
{
	if (prj.usingGPU == 0) {// 1:true, 0: false
#ifdef OnGPU
		writeLog(fpn_log, "Simulator using CUDA was activated in current G2D model. \n", 1, 1);
		writeLog(fpn_log, "Using the G2D model optimized for CPU simulator is recommended.\n", 1, 1); 
#endif		
		writeLog(fpn_log, "Simulation using CPU was started.\n", 1, 1);
		if (simulationControl_CPU() != 1) { return 0; }
	}
	else {
#ifdef OnGPU
		writeLog(fpn_log, "Simulation using GPU was started.\n", 1, 1);
		 if (simulationControl_GPU() !=1) {return 0;}
#else
		writeLog(fpn_log, "Simulator using CUDA was not activated.\n", 1, 1);
		writeLog(fpn_log, "Using the G2D model optimized for GPU simulator is recommended.\n", 1, 1);
		return 0;

#endif
	}	
	return 1;
}

void g2dHelp()
{
	printf("\n\n");
#ifdef OnGPU
	printf("** This simulator is optimized for using GPU.\n");
#else
	printf("** This simulator only supports using CPU. The simulation using GPU is not supported.\n");
#endif
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
	printf("        ��� ������Ʈ �̸��� ��ο� ������ ���Ե� ��� ū����ǥ�� ��� �Է��Ѵ�.\n");
	printf("          ** ����(G2D.exe�� d://G2Drun�� ���� ���)\n");
	printf("              - Case 1. G2D.exe�� �ٸ� ������ ������Ʈ ������ ���� ���\n");
	printf("                d://G2Drun>G2D.exe D://G2DTest//TestProject//test.g2p\n\n");
	printf("                d://G2Drun>G2D.exe \"D://G2DTest//Test Project//test.g2p\"\n");
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
	printf("        If the target project name and path contain spaces, enclose them in double quotes.\n");
	printf("          ** Examples (when G2D.exe is in d://G2Drun)\n");
	printf("              - Case 1. The project file is in a folder other than G2D.exe\n");
	printf("                d://G2Drun>g2d.exe D://G2DTest//TestProject//test.g2p\n");
	printf("                d://G2Drun>g2d.exe \"D://G2DTest//TestProject//test.g2p\"\n");
	printf("              - Case 2. The project file is in the same folder as G2D.exe\n");
	printf("                d://G2Drun>g2d.exe test.gmp\n");
	printf("\n");
	printf("** land cover vat file\n ");
	printf("   - the first value is grid value, the second is land cover name,\n");
	printf("     and the third is roughness coefficient.\n");
}
