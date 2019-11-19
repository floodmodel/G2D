#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <io.h>
#include <cctype>

#include "gentle.h"
#include "g2dLib.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern projectFile prj;

int openProjectFile()
{
	ifstream prjfile(fpn_prj);
	char outString[200];
	if (prjfile.is_open())
	{
		string aline;
		while (getline(prjfile, aline))
		{
			string valueString = "";
			if (aline.find("DEMFileDomain") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "DEMFileDomain");
				prj.fpnDEM = "";
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.fpnDEM = valueString;
					}
					else
					{
						sprintf(outString, "DEM file (%s) is invalid.\n", valueString.c_str());
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}
				else
				{
					sprintf(outString, "DEM file (%s) is invalid.\n", valueString.c_str());
					writeLog(fpn_log, outString, 1, 1);
					return -1;
				}
			}

			if (aline.find("LandCoverFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "LandCoverFile");
				prj.fpnLandCover = "";
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.fpnLandCover = valueString;
					}
					else
					{
						sprintf(outString, "Land cover file (%s) is invalid.\n", valueString.c_str());
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}
			}

			if (aline.find("LandCoverVatFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "LandCoverVatFile");
				prj.fpnLandCoverVat = "";
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.fpnLandCoverVat = valueString;
					}
					else
					{
						sprintf(outString, "Land cover VAT file (%s) is invalid.\n", valueString.c_str());
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

			if (aline.find("CalculationTimeStep_sec") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "CalculationTimeStep_sec");
				prj.calculationTimeStep_sec = 1.0;
				if (valueString != "")
				{
					prj.calculationTimeStep_sec = stod(valueString);
				}
			}

			if (aline.find("IsFixedDT") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "IsFixedDT");
				prj.isFixedDT = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.isFixedDT = 1;
					}
				}
			}

			if (aline.find("IsParallel") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "IsParallel");
				prj.isParallel = 1;
				if (valueString != "")
				{
					if (toLower(valueString) == "false")
					{
						prj.isParallel = -1;
					}
				}
			}

			if (aline.find("MaxDegreeOfParallelism") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MaxDegreeOfParallelism");
				prj.maxDegreeOfParallelism = 12;
				if (valueString != "")
				{
					prj.maxDegreeOfParallelism = stoi(valueString);
				}
			}

			if (aline.find("UsingGPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "UsingGPU");
				prj.usingGPU = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.usingGPU = 1;
					}
				}
			}

			if (aline.find("EffCellThresholdForGPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "EffCellThresholdForGPU");
				prj.effCellThresholdForGPU = 42000;
				if (valueString != "")
				{
					prj.effCellThresholdForGPU = stoi(valueString);
				}
			}

			if (aline.find("MaxIterationAllCellsOnCPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MaxIterationAllCellsOnCPU");
				prj.maxIterationAllCellsOnCPU = 7;
				if (valueString != "")
				{
					prj.maxIterationAllCellsOnCPU = stoi(valueString);
				}
			}

			if (aline.find("MaxIterationACellOnCPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MaxIterationACellOnCPU");
				prj.maxIterationACellOnCPU = 5;
				if (valueString != "")
				{
					prj.maxIterationACellOnCPU = stoi(valueString);
				}
			}

			if (aline.find("MaxIterationAllCellsOnGPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MaxIterationAllCellsOnGPU");
				prj.maxIterationAllCellsOnGPU = 7;
				if (valueString != "")
				{
					prj.maxIterationAllCellsOnGPU = stoi(valueString);
				}
			}

			if (aline.find("MaxIterationACellOnGPU") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MaxIterationACellOnGPU");
				prj.maxIterationACellOnGPU = 5;
				if (valueString != "")
				{
					prj.maxIterationACellOnGPU = stoi(valueString);
				}
			}

			if (aline.find("PrintoutInterval_min") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "PrintoutInterval_min");
				prj.printOUTinterval_min = 30.0;
				if (valueString != "")
				{
					prj.printOUTinterval_min = stod(valueString);
				}
			}

			if (aline.find("SimulationDuration_hr") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "SimulationDuration_hr");
				prj.simDuration_hr = 24.0;
				prj.simDuration_min = 24.0 * 60.0;
				if (valueString != "")
				{
					prj.simDuration_hr = stod(valueString);
					prj.simDuration_min = prj.simDuration_hr*60.0;
				}
			}

			if (aline.find("StartDateTime") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "StartDateTime");
				prj.startDateTime = "0";
				if (valueString != "")
				{
					prj.startDateTime = valueString;
				}
			}

			if (aline.find("RainfallDataType") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "RainfallDataType");
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

			if (aline.find("RainfallDataInterval_min") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "RainfallDataInterval_min");
				prj.rainfallDataInterval_min = 0;
				if (valueString != "")
				{
					prj.rainfallDataInterval_min = stoi(valueString);
				}
			}

			if (aline.find("RainfallFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "RainfallFile");
				prj.rainfallFile = "";
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.rainfallFile = valueString;
					}
					else
					{
						sprintf(outString, "Rainfall file (%s) is invalid.\n", valueString.c_str());
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}
			}

			if (aline.find("BCDataInterval_min") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "BCDataInterval_min");
				prj.bcDataInterval_min = 0.0;
				if (valueString != "")
				{
					prj.bcDataInterval_min = stoi(valueString);
					if (prj.bcDataInterval_min < 0)
					{
						sprintf(outString, "Time interval of boundary condition data is invalid.\n");
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}
			}

			if (aline.find("FloodingCellDepthThresholds_cm") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "FloodingCellDepthThresholds_cm");
				prj.floodingCellDepthThresholds_cm.push_back(0.001);
				if (valueString != "")
				{
					prj.floodingCellDepthThresholds_cm.clear();
					prj.floodingCellDepthThresholds_cm = splitToDoubleVector(valueString, ',');
				}
			}

			if (aline.find("OutputDepth") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "OutputDepth");
				prj.outputDepth = 1;
				if (valueString != "")
				{
					if (toLower(valueString) == "false")
					{
						prj.outputDepth = -1;
					}
				}
			}

			if (aline.find("OutputHeight") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "OutputHeight");
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
				valueString = getValueStringFromXmlLine(aline, "OutputVelocityMax");
				prj.outputVelocityMax = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.outputVelocityMax = 1;
					}
				}
			}

			if (aline.find("OutputFDofMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "OutputFDofMaxV");
				prj.outputFDofMaxV = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.outputFDofMaxV = 1;
					}
				}
			}

			if (aline.find("OutputDischargeMax") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "OutputDischargeMax");
				prj.outputDischargeMax = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.outputDischargeMax = 1;
					}
				}
			}

			if (aline.find("OutputRFGrid") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "OutputRFGrid");
				prj.outputRFGrid = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.outputRFGrid = 1;
					}
				}
			}

			if (aline.find("DepthImgRendererMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "DepthImgRendererMaxV");
				prj.depthImgRendererMaxV = 0.0;
				if (valueString != "")
				{
					prj.depthImgRendererMaxV = stod(valueString);
				}
			}

			if (aline.find("HeightImgRendererMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "HeightImgRendererMaxV");
				prj.heightImgRendererMaxV = 0.0;
				if (valueString != "")
				{
					prj.heightImgRendererMaxV = stod(valueString);
				}
			}

			if (aline.find("VelocityMaxImgRendererMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "VelocityMaxImgRendererMaxV");
				prj.velocityMaxImgRendererMaxV = 0.0;
				if (valueString != "")
				{
					prj.velocityMaxImgRendererMaxV = stod(valueString);
				}
			}

			if (aline.find("DischargeImgRendererMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "DischargeImgRendererMaxV");
				prj.dischargeImgRendererMaxV = 0.0;
				if (valueString != "")
				{
					prj.dischargeImgRendererMaxV = stod(valueString);
				}
			}

			if (aline.find("RFImgRendererMaxV") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "RFImgRendererMaxV");
				prj.rfImgRendererMaxV = 0.0;
				if (valueString != "")
				{
					prj.rfImgRendererMaxV = stod(valueString);
				}
			}

			if (aline.find("MakeASCFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MakeASCFile");
				prj.makeASCFile = 1;
				if (valueString != "")
				{
					if (toLower(valueString) == "false")
					{
						prj.makeASCFile = -1;
					}
				}
			}

			if (aline.find("MakeImgFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "MakeImgFile");
				prj.makeImgFile = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.makeImgFile = 1;
					}
				}
			}

			if (aline.find("WriteLog") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "WriteLog");
				prj.writeLog = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.writeLog = 1;
					}
				}
			}

			if (aline.find("RoughnessCoeff") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "RoughnessCoeff");
				prj.roughnessCoeff = 0.045;
				if (valueString != "")
				{
					prj.roughnessCoeff = stod(valueString);
				}
			}
			prj.imperviousR = 1;

			if (aline.find("DomainOutBedSlope") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "DomainOutBedSlope");
				prj.domainOutBedSlope = 0.001;
				if (valueString != "")
				{
					prj.domainOutBedSlope = stod(valueString);
				}
			}

			if (aline.find("InitialConditionType") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "InitialConditionType");
				prj.icDataType = conditionDataType::NoneCD;
				if (valueString != "")
				{
					if (toLower(valueString) == "depth")
					{
						prj.icDataType = conditionDataType::Depth;
					}
					//else if (toLower(valueString) == "discharge")
					//{
					//	prj.icDataType = conditionDataType::Discharge;
					//}
					else if (toLower(valueString) == "height")
					{
						prj.icDataType = conditionDataType::Height;
					}
				}
			}

			if (aline.find("InitialCondition") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "InitialCondition");
				prj.icValue_m = 0;
				prj.icFPN = "";
				prj.icType = fileOrConstant::None;
				if (valueString != "")
				{
					if (ifstream(valueString).good() == true)
					{
						prj.icFPN = valueString;
						prj.icType = fileOrConstant::File;
					}
					else
					{
						prj.icValue_m = stod(valueString);
						prj.icType = fileOrConstant::Constant;
					}
				}
			}

			if (aline.find("FroudeNumberCriteria") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "FroudeNumberCriteria");
				prj.froudeNumberCriteria = 1.0;
				if (valueString != "")
				{
					prj.froudeNumberCriteria = stod(valueString);
				}
			}

			if (aline.find("CourantNumber") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "CourantNumber");
				prj.courantNumber = 1.0;
				if (valueString != "")
				{
					prj.courantNumber = stod(valueString);
				}
			}

			if (aline.find("ApplyVNC") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "ApplyVNC");
				prj.applyVNC = -1;
				if (valueString != "")
				{
					if (toLower(valueString) == "true")
					{
						prj.applyVNC = 1;
					}
				}
			}
					   			 		  		  		 	   		
			if (aline.find("bcCellXY") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "bcCellXY");
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

			if (aline.find("bcDataFile") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "bcDataFile");
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.bcDataFile.push_back(valueString);
					}
					else
					{
						sprintf(outString, "Boundary condition file (%s) is invalid.\n", valueString.c_str());
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}

			}
			
			if (aline.find("bcDataType") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "bcDataType");
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

			prj.bcCount = min(prj.bcDataFile.size(), prj.bcDataType.size());

			if (aline.find("TimeMinuteToChangeDEM") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "TimeMinuteToChangeDEM");
				if (valueString != "")
				{
					prj.timeToChangeDEM_min.push_back(stod(valueString));
				}
			}

			if (aline.find("DEMFileToChange") != string::npos)
			{
				valueString = getValueStringFromXmlLine(aline, "DEMFileToChange");
				if (valueString != "")
				{
					if (access(valueString.c_str(), 0) == 0)
					{
						prj.fpnDEMtoChange.push_back(valueString);
					}
					else
					{
						sprintf(outString, "DEM file (%s) used to change  is invalid.\n", valueString.c_str());
						writeLog(fpn_log, outString, 1, 1);
						return -1;
					}
				}
			}
			prj.DEMtoChangeCount = min(prj.timeToChangeDEM_min.size(), prj.fpnDEMtoChange.size());
		}
	}		
	prjfile.close();
	return 1;
}


