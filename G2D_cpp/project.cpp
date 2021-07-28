#include "stdafx.h"
#include "gentle.h"
#include "g2d.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;
extern projectFile prj;
extern generalEnv ge;
extern globalVinner gvi;
extern thisProcess ps;

fs::file_time_type prjfileSavedTime;

int openProjectFile()
{
	if (_access(fpn_prj.string().c_str(), 0) != 0) {
		return -1;
	}
	prjfileSavedTime = fs::last_write_time(fpn_prj);
	bcinfo* abci;
	abci = new bcinfo;
	vector<bcinfo> bcisv;

	demToChangeinfo* adc;
	adc = new demToChangeinfo;
	string* bcDataFile = new string{ "" };
	
	projectFileFieldName fn;
	projectFileTable pt;
	int sbProjectSettings = 0; //0::비활성, 1: 활성
	int sbHydroPars = 0; //0:비활성, 1: 활성
	int sbBoundaryConditionData = 0; //0:비활성, 1: 활성
	int sbDEMFileToChange = 0; //0:비활성, 1: 활성

	vector<string> prjFile;
	prjFile = readTextFileToStringVector(fpn_prj.string());
	int LineCount = prjFile.size();
	for (int i = 0; i < LineCount; ++i) {
		string aline = prjFile[i];
		pt.sProjectSettings = getTableStateByXmlLineByLine(aline, pt.nProjectSettings);
		pt.sHydroPars = getTableStateByXmlLineByLine(aline, pt.nHydroPars);
		pt.sBoundaryConditionData = getTableStateByXmlLineByLine(aline, pt.nBoundaryConditionData);
		pt.sDEMFileToChange = getTableStateByXmlLineByLine(aline, pt.nDEMFileToChange);
		if (pt.sProjectSettings == 1 || pt.sProjectSettings == 2) {
			sbProjectSettings = 1;
		}
		if (pt.sHydroPars == 1 || pt.sHydroPars == 2) {
			sbHydroPars = 1;
		}
		if (pt.sBoundaryConditionData == 1 || pt.sBoundaryConditionData == 2) {
			sbBoundaryConditionData = 1;
		}
		if (pt.sDEMFileToChange == 1 || pt.sDEMFileToChange == 2) {
			sbDEMFileToChange = 1;
		}
		if (trim(aline) == "<" + pt.nProjectSettings + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nHydroPars + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nBoundaryConditionData + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nDEMFileToChange + ">") {
			continue;
		}
		if (sbProjectSettings == 1 && pt.sProjectSettings == 0) {
			sbProjectSettings = 0;
			continue;
		}
		if (sbProjectSettings == 1 && pt.sProjectSettings != 0) {
			sbProjectSettings = 1;
			if (readXmlRowProjectSettings(aline) == 0) {
				return 0;
			}
			continue;
		}
		if (sbHydroPars == 1 && pt.sHydroPars == 0) {
			sbHydroPars = 0;
			continue;
		}
		if (sbHydroPars == 1 && pt.sHydroPars != 0) {
			sbHydroPars = 1;
			if (readXmlRowHydroPars(aline) == 0) {
				return 0;
			}
			continue;
		}
		if (sbBoundaryConditionData == 1 && pt.sBoundaryConditionData == 0) {
			sbBoundaryConditionData = 0;
			if (isNormalBCinfo(abci) == 1) {
				if (bcDataFile[0] != "") {// 이 경우에는 bc를 하나 추가한다.
					bcisv.push_back(*abci);
					prj.bcDataFiles.push_back(bcDataFile[0]);
				}
				abci = new bcinfo;				
				bcDataFile[0] = "";
			}
			continue;
		}
		if (sbBoundaryConditionData == 1 && pt.sBoundaryConditionData != 0) {
			sbBoundaryConditionData = 1;
			if (readXmlRowBoundaryConditionData(aline, abci, bcDataFile) == 0) {
				return 0;
			}
			continue;
		}
		if (sbDEMFileToChange == 1 && pt.sDEMFileToChange == 0) {
			sbDEMFileToChange = 0;
			if (isNormalDEMtoChangeinfo(adc) == 1) {
				prj.dcs.push_back(*adc);
				adc = new demToChangeinfo;
			}
			continue;
		}
		if (sbDEMFileToChange == 1 && pt.sDEMFileToChange != 0) {
			sbDEMFileToChange = 1;
			if (readXmlRowDEMFileToChange(aline, adc) == 0) {
				return 0;
			}
			continue;
		}
	}
	//=======================
	fs::path fpn_dem = prj.fpnDEM;
	string demFpnWithoutExt = fpn_dem.replace_extension().string();
	string fpnProection = demFpnWithoutExt + ".prj";
	if (fs::exists(fpnProection) == true) {
		prj.fpnDEMprjection = fpnProection;
	}

	if (prj.fpnLandCover != "" && prj.fpnLandCoverVat != "") {
		prj.usingLCFile = 1;
	}
	else {
		prj.usingLCFile = -1;
	}
	prj.imperviousR = 1;
	if (prj.rainfallDataType != rainfallDataType::NoneRF
		&& prj.rainfallDataInterval_min > 0
		&& prj.rainfallFPN != "") {
		prj.isRainfallApplied = 1;
	}
	else {
		prj.isRainfallApplied = -1;
		prj.rainfallDataInterval_min = 0;
		prj.rainfallDataType = rainfallDataType::NoneRF;
		prj.rainfallFPN = "";
	}

	if (prj.bcDataInterval_min > 0 && bcisv.size() > 0){

		prj.isbcApplied = 1;
		prj.bcCount = bcisv.size();
		prj.bcis = new bcinfo[prj.bcCount];
		copy(bcisv.begin(), bcisv.end(), prj.bcis);
	}
	else {
		prj.isbcApplied = 0;
		prj.bcCount = 0;
		prj.bcDataInterval_min = 0;
		prj.bcis = NULL;
	}

	prj.DEMtoChangeCount = prj.dcs.size();
	if (prj.DEMtoChangeCount > 0) {
		prj.isDEMtoChangeApplied = 1;
	}
	else {
		prj.isDEMtoChangeApplied = 0;
	}

	if (prj.makeImgFile == 1) {
		if (prj.outputDischargeMax == 1 && prj.rendererMaxVDischargeImg == 0.0) {
			prj.rendererMaxVDischargeImg = 10000;
		}
		if (prj.outputDepth == 1 && prj.rendererMaxVdepthImg == 0.0)		{
				prj.rendererMaxVdepthImg = 3;
			}
		if (prj.outputWaterLevel==1 && prj.rendererMaxVwaterLevelimg==0.0)		{
			prj.rendererMaxVwaterLevelimg = 200;
			}
		if (prj.outputVelocityMax == 1 && prj.rendererMaxVMaxVImg==0.0)
		{
			prj.rendererMaxVMaxVImg = 10;
			}		
	}
	
	if (prj.maxDegreeOfParallelism == -1) {
		prj.maxDegreeOfParallelism = prj.cpusi.totalNumOfLP;
	}
	else if (prj.maxDegreeOfParallelism == 0) {
		prj.maxDegreeOfParallelism = 1;
	}
	//===============================
	if (prj.isDateTimeFormat == 1) {
		prj.tTag_length = 0;
	}
	else {
		string simDur = "";
		if (prj.printOutInterval_min < 60.0) {
			int simDur_min = (int) prj.simDuration_hr * 60;
			simDur = to_string(simDur_min);
		}
		else if (prj.printOutInterval_min >= 60 && prj.printOutInterval_min < 1440) {
			simDur = to_string(int(prj.simDuration_hr));
		}
		else if (prj.printOutInterval_min >= 1440) {
			int simDur_d = (int)prj.simDuration_hr /24;
			simDur = to_string(simDur_d);
		}
		prj.tTag_length = strlen(simDur.c_str()) + 3;
	}

#ifdef isVD
	prj.fpnTest_willbeDeleted
		= fp_prj.string() + "\\00_Summary_test.out";
	if (fs::exists(prj.fpnTest_willbeDeleted) == true) {
		confirmDeleteFile(prj.fpnTest_willbeDeleted);
	}
	prj.fpniterAcell_willbeDeleted = fp_prj.string() + "\00_Summary_Acell.out";
	if (fs::exists(prj.fpniterAcell_willbeDeleted) == true) {
		confirmDeleteFile(prj.fpniterAcell_willbeDeleted);
	}
	prj.hvalues_Acell_willbeDeleted = "";
#endif
	return 1;
}

int updateProjectParameters()
{
	fs::file_time_type prjfileSavedTime_rev;
	prjfileSavedTime_rev = fs::last_write_time(fpn_prj);
	if (prjfileSavedTime_rev == prjfileSavedTime) {
		return 1;
	}
	else {
		prjfileSavedTime = prjfileSavedTime_rev;
		int bak_MDP = prj.maxDegreeOfParallelism;
		int bak_iGSmax_CPU = prj.maxIterationAllCellsOnCPU;
		int bak_iNRmax_CPU = prj.maxIterationACellOnCPU;
		double bak_dt_printout_min = prj.printOutInterval_min;
		double bak_dt_printout_sec = prj.printOutInterval_min * 60.0;
		vector<double> bak_FloodingCellThresholds_cm = prj.floodingCellDepthThresholds_cm; 
		int bak_isDEMtoChangeApplied = prj.isDEMtoChangeApplied;// true : 1, false : 0
		vector<demToChangeinfo> bak_dcs = prj.dcs;
		int bak_DEMtoChangeCount = prj.DEMtoChangeCount;

		openProjectFile();
		prj.parChanged = 0;

		if (bak_MDP != prj.maxDegreeOfParallelism
			&& prj.usingGPU!=1) {
			string usingGPU = "false";
			string isparallel = "false";
			if (prj.maxDegreeOfParallelism > 1) { isparallel = "true"; }
			printf("");
			writeLog(fpn_log, "Parallel : " + isparallel
				+ ". Max. degree of parallelism : "
				+ to_string(prj.maxDegreeOfParallelism)
				+ ". Using GPU : " + usingGPU + ".\n", 1, 1);
			// 여기서 omp_set_num_threads(gvi.mdp); 한번만 하면, 나중에 애러난다.
			// omp parallel 구문마다 omp_set_num_threads(gvi.mdp); 해주면 mdp가 잘 변경 된다. 
			ps.mdp= prj.maxDegreeOfParallelism;
			prj.parChanged = 1;
		}
		
		if (bak_iGSmax_CPU != prj.maxIterationAllCellsOnCPU ||
			bak_iNRmax_CPU != prj.maxIterationACellOnCPU)
		{
			if (prj.parChanged == 0) { printf(""); }
			writeLog(fpn_log, "iGS(all cells) max using CPU : " + to_string(prj.maxIterationAllCellsOnCPU)
				+ ", iNR(a cell) max using CPU : " + to_string(prj.maxIterationACellOnCPU)
				+ ", tolerance (m) : " + to_string(CCh) + "\n", 1, 1);
			prj.parChanged = 1;
		}

		if (bak_dt_printout_min != prj.printOutInterval_min) {
			if (prj.parChanged == 0) { printf(""); }
			writeLog(fpn_log, "Print out time step (min) : " + to_string(prj.printOutInterval_min) + "\n", 1, 1);
			prj.parChanged = 1;
		}
		string thresholds = "";
		for (int n = 0; n < prj.floodingCellDepthThresholds_cm.size(); n++) {
			if (n == 0) {
				thresholds = to_string(prj.floodingCellDepthThresholds_cm[n]);
			}
			else {
				thresholds = thresholds + ", " + (to_string(prj.floodingCellDepthThresholds_cm[n]));
			}
		}
		if (bak_FloodingCellThresholds_cm.size() != prj.floodingCellDepthThresholds_cm.size()) {
			if (prj.parChanged == 0) { printf(""); }
			writeLog(fpn_log, "Flooding cell threshold (cm) : " + thresholds + ".\n", 1, 1);
			prj.parChanged = 1;
		}
		else {
			int changed = 0;
			for (int n = 0; n < prj.floodingCellDepthThresholds_cm.size(); n++) {
				if (bak_FloodingCellThresholds_cm[n] != prj.floodingCellDepthThresholds_cm[n]) {
					changed = 1;
				}
			}
			if (changed == 1) {
				if (prj.parChanged == 0) { printf(""); }
				writeLog(fpn_log, "Flooding cell threshold (cm) : " + thresholds + ".\n", 1, 1);
				prj.parChanged = 1;
			}
		}

		// 여기부터 DEM file to change 관련 변수들 확인
		if (bak_isDEMtoChangeApplied != prj.isDEMtoChangeApplied) {
			if (prj.isDEMtoChangeApplied == 0) {
				if (prj.parChanged == 0) { printf(""); }
				writeLog(fpn_log, "DEM files to change were removed." + thresholds + ".\n", 1, 1);
				prj.parChanged = 1;
			}
			else {
				int changed = 0;
				if (bak_DEMtoChangeCount != prj.DEMtoChangeCount) {
					changed = 1;
				}
				else {
					for (int n = 0; n < prj.DEMtoChangeCount; n++) {
						if (prj.dcs[n].fpnDEMtoChange != bak_dcs[n].fpnDEMtoChange
							|| prj.dcs[n].timeToChangeDEM_min != bak_dcs[n].timeToChangeDEM_min) {
							changed = 1;
						}
					}
				}
				if (changed == 1) {
					if (prj.parChanged == 0) { printf(""); }
					for (int n = 0; n < prj.DEMtoChangeCount; n++)
					{
						writeLog(fpn_log, "DEM file to change was revised. File : " + prj.dcs[n].fpnDEMtoChange
							+ ", Time : " + dtos(prj.dcs[n].timeToChangeDEM_min, 2) + "\n.", 1, 1);
					}
					prj.parChanged = 1;
				}
			}
		}
		if (prj.parChanged == 1) {
			writeLog(fpn_log, "Simulation setting was changed.\n", 1, 1);
		}
		return 1;
	}
}

int readXmlRowDEMFileToChange(string aline, demToChangeinfo *dc) 
{
	string vString = "";
	projectFileFieldName fn;
	if (aline.find(fn.TimeMinuteToChangeDEM_01) != string::npos
		|| aline.find(fn.TimeMinuteToChangeDEM_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.TimeMinuteToChangeDEM_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.TimeMinuteToChangeDEM_02);
		}
		if (vString != "" && isNumeric(vString)==true) {
			dc->timeToChangeDEM_min = stod(vString);
		}
		return 1;
	}
	if (aline.find(fn.DEMFileToChange_01) != string::npos
		|| aline.find(fn.DEMFileToChange_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.DEMFileToChange_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.DEMFileToChange_02);
		}
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				dc->fpnDEMtoChange = vString;
			}
			else {
				writeLog(fpn_log, "DEM file ("+ vString 
					+") used to change is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	return 1;
}

int readXmlRowBoundaryConditionData(string aline, bcinfo *bci, string* bcDataFile)
{
	string vString = "";
	projectFileFieldName fn;
	vector<cellPosition > bcCellXY_group;
	if (aline.find(fn.bcCellXY_01) != string::npos
		|| aline.find(fn.bcCellXY_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.bcCellXY_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.bcCellXY_02);
		}
		if (vString != "") {
			vector<string> xys = splitToStringVector(vString, '/');
			for (int i = 0; i < xys.size(); i++) {
				vector<int> axy_v = splitToIntVector(xys[i], ',');
				cellPosition axy;
				axy.xCol = axy_v[0];
				axy.yRow = axy_v[1];
				bcCellXY_group.push_back(axy);
			}
			bci->nCellsInAbc = bcCellXY_group.size();
			bci->bcCellXY = new cellPosition[bci->nCellsInAbc];
			copy(bcCellXY_group.begin(), bcCellXY_group.end(), bci->bcCellXY);
		}
		return 1;
	}
	if (aline.find(fn.bcDataFile_01) != string::npos
		|| aline.find(fn.bcDataFile_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.bcDataFile_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.bcDataFile_02);
		}
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				bcDataFile[0]=vString;
			}
			else {
				writeLog(fpn_log, "Boundary condition file ("
					+ vString +") is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.bcDataType_01) != string::npos
		|| aline.find(fn.bcDataType_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.bcDataType_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.bcDataType_02);
		}
		conditionDataType bcDT;
		bcDT = conditionDataType::NoneCD;
		if (vString != "") {
			if (lower(vString) == "discharge") {
				bcDT = conditionDataType::Discharge;
			}
			else if (lower(vString) == "depth") {
				bcDT = conditionDataType::Depth;
			}
			else if (lower(vString) == "height" || lower(vString) == "waterlevel") {
				bcDT = conditionDataType::WaterLevel;
			}
		}
		bci->bcDataType = bcDT;
		return 1;
	}
	return 1;
}

int readXmlRowHydroPars(string aline)
{
	string vString = "";
	projectFileFieldName fn;

	if (aline.find(fn.RoughnessCoeff) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.RoughnessCoeff);
		prj.roughnessCoeff = 0.045;
		if (vString != "") {
			prj.roughnessCoeff = stof(vString);
		}
		return 1;
	}

	if (aline.find(fn.DomainOutBedSlope) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.DomainOutBedSlope);
		prj.domainOutBedSlope = 0.001;
		if (vString != "") {
			prj.domainOutBedSlope = stof(vString);
		}
		return 1;
	}

	if (aline.find(fn.InitialConditionType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.InitialConditionType);
		prj.icType = conditionDataType::NoneCD;
		prj.isicApplied = 0;
		if (vString != "") {
			if (lower(vString) == "depth") {
				prj.icType = conditionDataType::Depth;
				prj.isicApplied = 1;
			}
			else if (lower(vString) == "height"|| lower(vString) == "waterlevel") {
				prj.icType = conditionDataType::WaterLevel;
				prj.isicApplied = 1;
			}
		}
		return 1;
	}

	if (aline.find(fn.InitialCondition) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.InitialCondition);
		prj.icValue_m = 0;
		prj.icFPN = "";
		prj.icDataType = fileOrConstant::None;
		prj.isicApplied = 0;
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				prj.icFPN = vString;
				prj.icDataType = fileOrConstant::File;
				prj.usingicFile = 1;
				prj.isicApplied = 1;
			}
			else {
				prj.icValue_m = stod_c(vString);
				prj.icDataType = fileOrConstant::Constant;
				prj.usingicFile = 0;
				prj.isicApplied = 1;
			}
		}
		return 1;
	}

	if (aline.find(fn.FroudeNumberCriteria) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.FroudeNumberCriteria);
		prj.froudeNumberCriteria = 1.0;
		if (vString != "") {
			prj.froudeNumberCriteria = stof(vString);
		}
		return 1;
	}

	if (aline.find(fn.CourantNumber) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.CourantNumber);
		prj.courantNumber = 1.0;
		if (vString != "") {
			prj.courantNumber = stof(vString);
		}
		return 1;
	}

	if (aline.find(fn.ApplyVNC) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.ApplyVNC);
		prj.applyVNC = -1;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.applyVNC = 1;
			}
		}
		return 1;
	}
	return 1;
}

int readXmlRowProjectSettings(string aline)
{
	string vString = "";
	projectFileFieldName fn;
	if (aline.find(fn.DomainDEMFile_01) != string::npos
		|| aline.find(fn.DomainDEMFile_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.DomainDEMFile_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.DomainDEMFile_02);
		}
		prj.fpnDEM = "";
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				prj.fpnDEM = vString;
			}
			else {
				writeLog(fpn_log, "DEM file "+ vString +" is invalid.\n", 1, 1);
				return 0;
			}
		}
		else {
			writeLog(fpn_log, "DEM file is invalid.\n", 1, 1);
			return 0;
		}
		return 1;
	}
	if (aline.find(fn.LandCoverFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.LandCoverFile);
		prj.fpnLandCover = "";
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				prj.fpnLandCover = vString;
			}
			else {
				writeLog(fpn_log, "Land cover file (%s) is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.LandCoverVatFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.LandCoverVatFile);
		prj.fpnLandCoverVat = "";
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				prj.fpnLandCoverVat = vString;
			}
			else {
				writeLog(fpn_log, "Land cover VAT file (%s) is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.CalculationTimeStep_sec_01) != string::npos
		|| aline.find(fn.CalculationTimeStep_sec_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.CalculationTimeStep_sec_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.CalculationTimeStep_sec_02);
		}
		prj.calculationTimeStep_sec = dtMIN_sec;
		if (vString != "") {
			prj.calculationTimeStep_sec = stof(vString);
		}
		return 1;
	}
	if (aline.find(fn.IsFixedDT) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.IsFixedDT);
		prj.isFixedDT = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.isFixedDT = 1;
			}
		}
		return 1;
	}
	if (aline.find(fn.MaxDegreeOfParallelism_01) != string::npos
		|| aline.find(fn.MaxDegreeOfParallelism_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.MaxDegreeOfParallelism_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.MaxDegreeOfParallelism_02);
		}
		prj.maxDegreeOfParallelism = -1;
		if (vString != "") {
			prj.maxDegreeOfParallelism = stoi(vString);
		}
		return 1;
	}
	if (aline.find(fn.UsingGPU) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.UsingGPU);
		prj.usingGPU = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.usingGPU = 1;
			}
		}
		return 1;
	}

	if (aline.find(fn.ThreadsPerBlock) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.ThreadsPerBlock);
		prj.threadsPerBlock = 256;
		if (vString != "") {
			prj.threadsPerBlock = stoi(vString);
		}
		return 1;
	}

	if (aline.find(fn.MaxIterationAllCells_01) != string::npos
		|| aline.find(fn.MaxIterationAllCells_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.MaxIterationAllCells_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.MaxIterationAllCells_02);
		}
		prj.maxIterationAllCellsOnCPU = 7;
		if (vString != "") {
			prj.maxIterationAllCellsOnCPU = stoi(vString);
		}
		return 1;
	}
	if (aline.find(fn.MaxIterationACell_01) != string::npos
		|| aline.find(fn.MaxIterationACell_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.MaxIterationACell_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.MaxIterationACell_02);
		}
		prj.maxIterationACellOnCPU = 5;
		if (vString != "") {
			prj.maxIterationACellOnCPU = stoi(vString);
		}
		return 1;
	}

	if (aline.find(fn.PrintoutInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.PrintoutInterval_min);
		prj.printOutInterval_min = 30.0;
		if (vString != "") {
			prj.printOutInterval_min = stof(vString);
		}
		return 1;
	}
	if (aline.find(fn.SimulationDuration_hr) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.SimulationDuration_hr);
		prj.simDuration_hr = 24.0;
		prj.simDuration_min = 24.0 * 60.0;
		if (vString != "") {
			prj.simDuration_hr = stof(vString);
			prj.simDuration_min = prj.simDuration_hr * 60.0f;
		}
		return 1;
	}
	if (aline.find(fn.StartDateTime) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.StartDateTime);
		prj.startDateTime = "0";
		prj.isDateTimeFormat = 0;
		if (vString != "") {
			prj.startDateTime = vString;
		}
		if (prj.startDateTime != "0") {
			prj.isDateTimeFormat = 1;
		}
		return 1;
	}
	if (aline.find(fn.RainfallDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.RainfallDataType);
		prj.rainfallDataType = rainfallDataType::NoneRF;
		if (vString != "") {
			if (lower(vString) == "textfilemap") {
				prj.rainfallDataType = rainfallDataType::TextFileMAP;
			}
			else if (lower(vString) == "textfileascgrid") {
				prj.rainfallDataType = rainfallDataType::TextFileASCgrid;
			}
		}
		return 1;
	}
	if (aline.find(fn.RainfallDataInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.RainfallDataInterval_min);
		prj.rainfallDataInterval_min = 0;
		if (vString != "") {
			prj.rainfallDataInterval_min = stoi(vString);
		}
		return 1;
	}
	if (aline.find(fn.RainfallFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.RainfallFile);
		prj.rainfallFPN = "";
		if (vString != "") {
			if (_access(vString.c_str(), 0) == 0) {
				prj.rainfallFPN = vString;
			}
			else {
				string logstr = "Rainfall file (" + vString + ") is invalid.\n";
				writeLog(fpn_log, logstr, 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.InitialRainfallLoss_mm) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.InitialRainfallLoss_mm);
		prj.initialRFLoss = 0.0f;
		if (vString != "") {
			prj.initialRFLoss = stof(vString);
			if (prj.initialRFLoss < 0.0f) {
				writeLog(fpn_log, "Initial rainfall loss value is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.BCDataInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.BCDataInterval_min);
		prj.bcDataInterval_min = 0;
		if (vString != "") {
			prj.bcDataInterval_min = stoi(vString);
			if (prj.bcDataInterval_min < 0) {
				writeLog(fpn_log, "Time interval of boundary condition data is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.FloodingCellDepthThresholds_cm) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.FloodingCellDepthThresholds_cm);
		prj.floodingCellDepthThresholds_cm.push_back(0.001);
		if (vString != "") {
			prj.floodingCellDepthThresholds_cm.clear();
			prj.floodingCellDepthThresholds_cm = splitToDoubleVector(vString, ',');
		}
		return 1;
	}

	if (aline.find(fn.CellLocationsToPrint) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.CellLocationsToPrint);
		prj.cellLocationsToPrint.clear();
		prj.printCellValue = 0;
		if (vString != "") {
			vector<string> cells = splitToStringVector(vString, '/');
			for (int i = 0; i < cells.size(); ++i) {
				vector<int> axy = splitToIntVector(cells[i], ',');
				cellPosition acell;
				acell.xCol = axy[0];
				acell.yRow = axy[1];
				prj.cellLocationsToPrint.push_back(acell);
			}
			if (prj.cellLocationsToPrint.size() > 0) {
				prj.printCellValue = 1;
			}
		}
		return 1;
	}

	if (aline.find(fn.OutputDepth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputDepth);
		prj.outputDepth = 1;
		if (vString != "") {
			if (lower(vString) == "false") {
				prj.outputDepth = 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.OutputPrecision_Depth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputPrecision_Depth);
		prj.outputPrecision_Depth = 5;
		if (vString != ""){
			if (isNumericInt(vString) == true) {
				prj.outputPrecision_Depth = stoi(vString);
			}
			else {
				writeLog(fpn_log, "OutputDepth_precision is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}

	if (aline.find(fn.OutputWaterLevel_01) != string::npos
		|| aline.find(fn.OutputWaterLevel_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputWaterLevel_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.OutputWaterLevel_02);
		}
		prj.outputWaterLevel = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.outputWaterLevel = 1;
			}
		}
		return 1;
	}
	if (aline.find(fn.OutputPrecision_WaterLevel_01) != string::npos
		|| aline.find(fn.OutputPrecision_WaterLevel_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputPrecision_WaterLevel_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.OutputPrecision_WaterLevel_02);
		}
		prj.outputPrecision_WaterLevel = 5;
		if (vString != "") {
			if (isNumericInt(vString) == true) {
				prj.outputPrecision_WaterLevel = stoi(vString);
			}
			else {
				writeLog(fpn_log, "OutputWaterLevel_precision is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}

	if (aline.find(fn.OutputVelocityMax) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputVelocityMax);
		prj.outputVelocityMax = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.outputVelocityMax = 1;
			}
		}
		return 1;
	}
	if (aline.find(fn.OutputPrecision_VelocityMax) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputPrecision_VelocityMax);
		prj.outputPrecision_VMax = 5;
		if (vString != "") {
			if (isNumericInt(vString) == true) {
				prj.outputPrecision_VMax = stoi(vString);
			}
			else {
				writeLog(fpn_log, "OutputVelocityMax_precision is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}

	if (aline.find(fn.OutputDischargeMax) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputDischargeMax);
		prj.outputDischargeMax = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.outputDischargeMax = 1;
			}
		}
		return 1;
	}
	if (aline.find(fn.OutputPrecision_DischargeMax) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputPrecision_DischargeMax);
		prj.outputPrecision_QMax = 5;
		if (vString != "") {
			if (isNumericInt(vString) == true) {
				prj.outputPrecision_QMax = stoi(vString);
			}
			else {
				writeLog(fpn_log, "OutputDischargeMax_precision is invalid.\n", 1, 1);
				return 0;
			}
		}
		return 1;
	}

	if (aline.find(fn.OutputFDofMaxV) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.OutputFDofMaxV);
		prj.outputFDofMaxV = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.outputFDofMaxV = 1;
			}
		}
		return 1;
	}

	if (aline.find(fn.DepthImgRendererMaxV) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.DepthImgRendererMaxV);
		prj.rendererMaxVdepthImg = 0.0;
		if (vString != "") {
			prj.rendererMaxVdepthImg = stof(vString);
		}
		return 1;
	}
	if (aline.find(fn.WaterLevelimgRendererMaxV_01) != string::npos
		|| aline.find(fn.WaterLevelimgRendererMaxV_02) != string::npos
		|| aline.find(fn.WaterLevelimgRendererMaxV_03) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.WaterLevelimgRendererMaxV_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.WaterLevelimgRendererMaxV_02);
		}
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fn.WaterLevelimgRendererMaxV_03);
		}
		prj.rendererMaxVwaterLevelimg = 0.0;
		if (vString != "") {
			prj.rendererMaxVwaterLevelimg = stof(vString);
		}
		return 1;
	}
	if (aline.find(fn.VelocityMaxImgRendererMaxV) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.VelocityMaxImgRendererMaxV);
		prj.rendererMaxVMaxVImg = 0.0;
		if (vString != "") {
			prj.rendererMaxVMaxVImg = stod(vString);
		}
		return 1;
	}
	if (aline.find(fn.DischargeImgRendererMaxV) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.DischargeImgRendererMaxV);
		prj.rendererMaxVDischargeImg = 0.0;
		if (vString != "") {
			prj.rendererMaxVDischargeImg = stod(vString);
		}
		return 1;
	}

	if (aline.find(fn.MakeASCFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.MakeASCFile);
		prj.makeASCFile = 1;
		if (vString != "") {
			if (lower(vString) == "false") {
				prj.makeASCFile = 0;
			}
		}
		return 1;
	}
	if (aline.find(fn.MakeImgFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.MakeImgFile);
		prj.makeImgFile = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.makeImgFile = 1;
			}
		}
		return 1;
	}
	if (aline.find(fn.WriteLog) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fn.WriteLog);
		prj.writeLog = 0;
		if (vString != "") {
			if (lower(vString) == "true") {
				prj.writeLog = 1;
			}
		}
		return 1;
	}
	return 1;
}


int isNormalBCinfo(bcinfo* bci)
{
	if (bci->nCellsInAbc < 1) { return 0; }
	if (bci->bcDataType == conditionDataType::NoneCD) { return 0; }
	return 1;
}

int isNormalDEMtoChangeinfo(demToChangeinfo* dci)
{
	if (dci->timeToChangeDEM_min==0) { return 0; }
	if (dci->fpnDEMtoChange == "") { return 0; }
	return 1;
}