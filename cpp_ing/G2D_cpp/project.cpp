#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <io.h>
#include <cctype>

#include "gentle.h"
#include "g2d.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;
extern projectFile prj;

int openProjectFile()
{
	ifstream prjfile(fpn_prj);
	char outString[200];
	if (prjfile.is_open() == false) { return -1; }
	string aline;
	projectFileFieldName fn;
	prj.isDateTimeFormat = -1;
	while (getline(prjfile, aline))
	{
		string valueString = "";
		if (aline.find(fn.DomainDEMFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.DomainDEMFile);
			prj.fpnDEM = "";
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.fpnDEM = valueString;
				}
				else
				{
					sprintf_s(outString, "DEM file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
			else
			{
				sprintf_s(outString, "DEM file (%s) is invalid.\n", valueString.c_str());
				writeLog(fpn_log, outString, 1, 1);
				return -1;
			}
		}

		if (aline.find(fn.LandCoverFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.LandCoverFile);
			prj.fpnLandCover = "";
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.fpnLandCover = valueString;
				}
				else
				{
					sprintf_s(outString, "Land cover file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}

		if (aline.find(fn.LandCoverVatFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.LandCoverVatFile);
			prj.fpnLandCoverVat = "";
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.fpnLandCoverVat = valueString;
				}
				else
				{
					sprintf_s(outString, "Land cover VAT file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}

		if (prj.fpnLandCover != "" && prj.fpnLandCoverVat != "")
		{
			prj.usingLCFile = 1;
		}
		else
		{
			prj.usingLCFile = -1;
		}

		if (aline.find(fn.CalculationTimeStep_sec) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.CalculationTimeStep_sec);
			prj.calculationTimeStep_sec = 1.0;
			if (valueString != "")
			{
				prj.calculationTimeStep_sec = stof(valueString);
			}
		}

		if (aline.find(fn.IsFixedDT) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.IsFixedDT);
			prj.isFixedDT = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.isFixedDT = 1;
				}
			}
		}

		if (aline.find(fn.IsParallel) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.IsParallel);
			prj.isParallel = 1;
			if (valueString != "")
			{
				if (toLower(valueString) == "false")
				{
					prj.isParallel = -1;
				}
			}
		}

		if (aline.find(fn.MaxDegreeOfParallelism) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MaxDegreeOfParallelism);
			prj.maxDegreeOfParallelism = -1;
			if (valueString != "")
			{
				prj.maxDegreeOfParallelism = stoi(valueString);
			}
		}

		if (aline.find(fn.UsingGPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.UsingGPU);
			prj.usingGPU = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.usingGPU = 1;
				}
			}
		}

		if (aline.find(fn.EffCellThresholdForGPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.EffCellThresholdForGPU);
			prj.effCellThresholdForGPU = 42000;
			if (valueString != "")
			{
				prj.effCellThresholdForGPU = stoi(valueString);
			}
		}

		if (aline.find(fn.MaxIterationAllCellsOnCPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MaxIterationAllCellsOnCPU);
			prj.maxIterationAllCellsOnCPU = 7;
			if (valueString != "")
			{
				prj.maxIterationAllCellsOnCPU = stoi(valueString);
			}
		}

		if (aline.find(fn.MaxIterationACellOnCPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MaxIterationACellOnCPU);
			prj.maxIterationACellOnCPU = 5;
			if (valueString != "")
			{
				prj.maxIterationACellOnCPU = stoi(valueString);
			}
		}

		if (aline.find(fn.MaxIterationAllCellsOnGPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MaxIterationAllCellsOnGPU);
			prj.maxIterationAllCellsOnGPU = 7;
			if (valueString != "")
			{
				prj.maxIterationAllCellsOnGPU = stoi(valueString);
			}
		}

		if (aline.find(fn.MaxIterationACellOnGPU) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MaxIterationACellOnGPU);
			prj.maxIterationACellOnGPU = 5;
			if (valueString != "")
			{
				prj.maxIterationACellOnGPU = stoi(valueString);
			}
		}

		if (aline.find(fn.PrintoutInterval_min) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.PrintoutInterval_min);
			prj.printOUTinterval_min = 30.0;
			if (valueString != "")
			{
				prj.printOUTinterval_min = stof(valueString);
			}
		}

		if (aline.find(fn.SimulationDuration_hr) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.SimulationDuration_hr);
			prj.simDuration_hr = 24.0;
			prj.simDuration_min = 24.0 * 60.0;
			if (valueString != "")
			{
				prj.simDuration_hr = stof(valueString);
				prj.simDuration_min = prj.simDuration_hr * 60.0;
			}
		}


		if (aline.find(fn.StartDateTime) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.StartDateTime);
			prj.startDateTime = "0";
			if (valueString != "")
			{
				prj.startDateTime = valueString;
			}
			if (prj.startDateTime != "0")
			{
				prj.isDateTimeFormat = 1;
			}
		}

		if (aline.find(fn.RainfallDataType) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.RainfallDataType);
			prj.rainfallDataType = rainfallDataType::NoneRF;
			if (valueString != "")
			{
				if (toLower(valueString) == "textfilemap")
				{
					prj.rainfallDataType = rainfallDataType::TextFileMAP;
				}
				else if (toLower(valueString) == "textfileascgrid")
				{
					prj.rainfallDataType = rainfallDataType::TextFileASCgrid;
				}
			}
		}

		if (aline.find(fn.RainfallDataInterval_min) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.RainfallDataInterval_min);
			prj.rainfallDataInterval_min = 0;
			if (valueString != "")
			{
				prj.rainfallDataInterval_min = stoi(valueString);
			}
		}

		if (aline.find(fn.RainfallFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.RainfallFile);
			prj.rainfallFPN = "";
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.rainfallFPN = valueString;
				}
				else
				{
					sprintf_s(outString, "Rainfall file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}

		prj.isRainfallApplied = -1;
		if (prj.rainfallDataType != rainfallDataType::NoneRF && prj.rainfallDataInterval_min > 0 && prj.rainfallFPN != "")
		{
			prj.isRainfallApplied = 1;
		}

		if (aline.find(fn.BCDataInterval_min) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.BCDataInterval_min);
			prj.bcDataInterval_min = 0;
			if (valueString != "")
			{
				prj.bcDataInterval_min = stoi(valueString);
				if (prj.bcDataInterval_min < 0)
				{
					sprintf_s(outString, "Time interval of boundary condition data is invalid.\n");
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}

		if (aline.find(fn.FloodingCellDepthThresholds_cm) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.FloodingCellDepthThresholds_cm);
			prj.floodingCellDepthThresholds_cm.push_back(0.001f);
			if (valueString != "")
			{
				prj.floodingCellDepthThresholds_cm.clear();
				prj.floodingCellDepthThresholds_cm = splitToFloatVector(valueString, ',');
			}
		}

		if (aline.find(fn.OutputDepth) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputDepth);
			prj.outputDepth = 1;
			if (valueString != "")
			{
				if (toLower(valueString) == "false")
				{
					prj.outputDepth = -1;
				}
			}
		}

		if (aline.find(fn.OutputHeight) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputHeight);
			prj.outputHeight = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.outputHeight = 1;
				}
			}
		}

		if (aline.find("OutputVelocityMax") != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputVelocityMax);
			prj.outputVelocityMax = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.outputVelocityMax = 1;
				}
			}
		}

		if (aline.find(fn.OutputFDofMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputFDofMaxV);
			prj.outputFDofMaxV = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.outputFDofMaxV = 1;
				}
			}
		}

		if (aline.find(fn.OutputDischargeMax) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputDischargeMax);
			prj.outputDischargeMax = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.outputDischargeMax = 1;
				}
			}
		}

		if (aline.find(fn.OutputRFGrid) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.OutputRFGrid);
			prj.outputRFGrid = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.outputRFGrid = 1;
				}
			}
		}

		if (aline.find(fn.DepthImgRendererMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.DepthImgRendererMaxV);
			prj.depthImgRendererMaxV = 0.0;
			if (valueString != "")
			{
				prj.depthImgRendererMaxV = stof(valueString);
			}
		}

		if (aline.find(fn.HeightImgRendererMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.HeightImgRendererMaxV);
			prj.heightImgRendererMaxV = 0.0;
			if (valueString != "")
			{
				prj.heightImgRendererMaxV = stof(valueString);
			}
		}

		if (aline.find(fn.VelocityMaxImgRendererMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.VelocityMaxImgRendererMaxV);
			prj.velocityMaxImgRendererMaxV = 0.0;
			if (valueString != "")
			{
				prj.velocityMaxImgRendererMaxV = stod(valueString);
			}
		}

		if (aline.find(fn.DischargeImgRendererMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.DischargeImgRendererMaxV);
			prj.dischargeImgRendererMaxV = 0.0;
			if (valueString != "")
			{
				prj.dischargeImgRendererMaxV = stod(valueString);
			}
		}

		if (aline.find(fn.RFImgRendererMaxV) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.RFImgRendererMaxV);
			prj.rfImgRendererMaxV = 0.0;
			if (valueString != "")
			{
				prj.rfImgRendererMaxV = stof(valueString);
			}
		}

		if (aline.find(fn.MakeASCFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MakeASCFile);
			prj.makeASCFile = 1;
			if (valueString != "")
			{
				if (toLower(valueString) == "false")
				{
					prj.makeASCFile = -1;
				}
			}
		}

		if (aline.find(fn.MakeImgFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.MakeImgFile);
			prj.makeImgFile = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.makeImgFile = 1;
				}
			}
		}

		if (aline.find(fn.WriteLog) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.WriteLog);
			prj.writeLog = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.writeLog = 1;
				}
			}
		}

		if (aline.find(fn.RoughnessCoeff) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.RoughnessCoeff);
			prj.roughnessCoeff = 0.045f;
			if (valueString != "")
			{
				prj.roughnessCoeff = stof(valueString);
			}
		}
		prj.imperviousR = 1;

		if (aline.find(fn.DomainOutBedSlope) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.DomainOutBedSlope);
			prj.domainOutBedSlope = 0.001f;
			if (valueString != "")
			{
				prj.domainOutBedSlope = stof(valueString);
			}
		}

		if (aline.find(fn.InitialConditionType) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.InitialConditionType);
			prj.icType = conditionDataType::NoneCD;
			if (valueString != "")
			{
				if (toLower(valueString) == "depth")
				{
					prj.icType = conditionDataType::Depth;
				}
				else if (toLower(valueString) == "height")
				{
					prj.icType = conditionDataType::Height;
				}
			}
		}

		prj.usingicFile = -1;
		prj.isicApplied = -1;
		if (aline.find(fn.InitialCondition) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.InitialCondition);
			prj.icValue_m = 0;
			prj.icFPN = "";
			prj.icDataType = fileOrConstant::None;
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.icFPN = valueString;
					prj.icDataType = fileOrConstant::File;
					prj.usingicFile = 1;
					prj.isbcApplied = 1;
				}
				else
				{
					prj.icValue_m = stof(valueString);
					prj.icDataType = fileOrConstant::Constant;
					prj.usingicFile = -1;
					prj.isbcApplied = 1;
				}
			}
		}

		if (aline.find(fn.FroudeNumberCriteria) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.FroudeNumberCriteria);
			prj.froudeNumberCriteria = 1.0;
			if (valueString != "")
			{
				prj.froudeNumberCriteria = stof(valueString);
			}
		}

		if (aline.find(fn.CourantNumber) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.CourantNumber);
			prj.courantNumber = 1.0;
			if (valueString != "")
			{
				prj.courantNumber = stof(valueString);
			}
		}

		if (aline.find(fn.ApplyVNC) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.ApplyVNC);
			prj.applyVNC = -1;
			if (valueString != "")
			{
				if (toLower(valueString) == "true")
				{
					prj.applyVNC = 1;
				}
			}
		}

		if (aline.find(fn.bcCellXY) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.bcCellXY);
			if (valueString != "")
			{
				vector<string> xys = splitToStringVector(valueString, '/');
				vector<cellPosition > bcCellXY_group;
				for (int i = 0; i < xys.size(); i++)
				{
					vector<int > axy_v = splitToIntVector(xys[i], ',');
					cellPosition axy;
					axy.x = axy_v[0];
					axy.y = axy_v[1];
					bcCellXY_group.push_back(axy);
				}
				prj.bcCellXY.push_back(bcCellXY_group);
				//vector<cellPosition> bcCellXY_select= prj.bcCellXY[0]; // 이렇게 사용하면 되겠다.
			}
		}

		if (aline.find(fn.bcDataFile) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.bcDataFile);
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.bcDataFile.push_back(valueString);
				}
				else
				{
					sprintf_s(outString, "Boundary condition file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}

		if (aline.find(fn.bcDataType) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.bcDataType);
			conditionDataType bcDT;
			bcDT = conditionDataType::NoneCD;
			if (valueString != "")
			{
				if (toLower(valueString) == "discharge")
				{
					bcDT = conditionDataType::Discharge;
				}
				else if (toLower(valueString) == "depth")
				{
					bcDT = conditionDataType::Depth;
				}
				else if (toLower(valueString) == "height")
				{
					bcDT = conditionDataType::Height;
				}
			}
			prj.bcDataType.push_back(bcDT);
		}

		if (prj.bcDataInterval_min > 0 && prj.bcCellXY.size() > 0
			&& prj.bcDataFile.size() > 0 && prj.bcDataType.size() > 0)
		{
			prj.isbcApplied = 1;
			prj.bcCount = min((int)prj.bcDataFile.size(), (int)prj.bcDataType.size());
		}
		else
		{
			prj.isbcApplied = -1;
			prj.bcCount = 0;
		}

		if (aline.find(fn.TimeMinuteToChangeDEM) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.TimeMinuteToChangeDEM);
			if (valueString != "")
			{
				prj.timeToChangeDEM_min.push_back(stof(valueString));
			}
		}

		if (aline.find(fn.DEMFileToChange) != string::npos)
		{
			valueString = getValueStringFromXmlLine(aline, fn.DEMFileToChange);
			if (valueString != "")
			{
				if (_access(valueString.c_str(), 0) == 0)
				{
					prj.fpnDEMtoChange.push_back(valueString);
				}
				else
				{
					sprintf_s(outString, "DEM file (%s) used to change  is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}
		}
		prj.DEMtoChangeCount = min((int)prj.timeToChangeDEM_min.size(), (int)prj.fpnDEMtoChange.size());
		if (prj.DEMtoChangeCount > 0) {
			prj.isDEMtoChangeApplied = 1;
		}
		else {
			prj.isDEMtoChangeApplied = -1;
		}
	}
	prjfile.close();


	// 삭제 대상
	prj.fpnTest_willbeDeleted
		= fp_prj.string() + "\00_Summary_test.out";
	if (fs::exists(prj.fpnTest_willbeDeleted) == true)
	{
		confirmDeleteFile(prj.fpnTest_willbeDeleted);
	}
	prj.fpniterAcell_willbeDeleted = fp_prj.string() + "\00_Summary_Acell.out";
	if (fs::exists(prj.fpniterAcell_willbeDeleted) == true)
	{
		confirmDeleteFile(prj.fpniterAcell_willbeDeleted);
	}
	prj.hvalues_Acell_willbeDeleted = "";
	// 여기까지 삭제 대상
	return 1;
}


