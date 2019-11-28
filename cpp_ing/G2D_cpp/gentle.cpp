#include <iostream>
#include <string>
#include <stdio.h>
#include <intrin.h>
#include <io.h>
#include <stdlib.h>
#include <sstream>
#include <list>
#include<map>
#include <time.h>
#include<ATLComTime.h>
#include <filesystem>
#include <algorithm>
#include "cpuinfodelegate.h"
#include "gpuinfodelegate.h"

#include "gentle.h"

#ifdef _WIN32
	#include <windows.h>
#elif MACOS
	#include <sys/param.h>
	#include <sys/sysctl.h>
#else
	#include <unistd.h>
#endif

//#include "gentle_function.h"

using namespace std;
namespace fs = std::filesystem;


ascRasterFile::ascRasterFile(string fpn_ascRasterFile)
{
	fs::path fpnASC = fpn_ascRasterFile;
	int r = 0;
	ifstream ascFile(fpnASC);
	if (ascFile.is_open())
	{
		string aline;
		int r = 0;
		while (getline(ascFile, aline))
		{
			if (r > 7) { break; }//7번 읽는다.. 즉, 7줄을 읽고, 끝낸다. header는 최대 6줄이다.
			linesForHeader[r] = aline;
			r++;
		}
	}
	else
	{
		string outstr;
		outstr = "ASCII file (" + fpn_ascRasterFile + ") are invalid. It could not be opened.\n";
		cout << outstr;
	}
	header = getAscRasterHeader(linesForHeader, separator);
	headerStringAll = makeHeaderString(header.nCols, header.nRows,
		header.xllcorner, header.yllcorner, header.cellsize, header.dx, header.dy, header.nodataValue);
	extent = getAscRasterExtent(header);
	valuesFromTL = new double*[header.nCols]; //x를 먼저 할당하고, 아래에서 y를 할당한다.
	for (int i = 0; i < header.nCols; ++i)
	{
		valuesFromTL[i] = new double[header.nRows];
	}
	bool isBigSize = false;
	if (header.nCols * header.nRows > BigSizeThreshold) { isBigSize = true; }
	int headerEndingIndex = header.headerEndingLineIndex;
	int dataStaringIndex = header.dataStartingLineIndex;
	if (isBigSize == false)
	{
		//int rcountMax = header.nRows + header.headerEndingLineIndex+1;
		vector<string> allLinesv = readTextFileToStringVector(fpn_ascRasterFile);
		int lyMax = allLinesv.size();
#pragma omp parallel for
		for (int ly = header.dataStartingLineIndex; ly < lyMax; ++ly)
		{
			vector<string> values = splitToStringVector(allLinesv[ly], ' ');
			int y = ly - dataStaringIndex;
			int nX = values.size();
			for (int x = 0; x < nX; ++x)
			{
				if (isNumeric(values[x]) == true)
				{
					valuesFromTL[x][y] = stod(values[x]);
				}
				else
				{
					valuesFromTL[x][y] = header.nodataValue;
				}
			}
		}
	}
	else
	{
		int nl = 0;
		int y = 0;
		string aline;
		while (getline(ascFile, aline))
		{

			linesForHeader[nl] = aline;

			if (nl > headerEndingIndex)
			{
				vector<string> values = values = splitToStringVector(aline, ' ');
				for (int x = 0; x < values.size(); ++x)
				{
					double v = 0;
					if (isNumeric(values[x]) == true)
					{
						valuesFromTL[x][y] = stod(values[x]);
					}
					else
					{
						valuesFromTL[x][y] = header.nodataValue;
					}
				}
				y++;
			}
			nl++;
		}
	}
}

ascRasterFile::~ascRasterFile()
{
	for (__int32 i = 0; i < header.nCols; ++i)
	{
		if (valuesFromTL[i] != NULL)
		{
			delete[] valuesFromTL[i];
		}
	}
	//delete[] valuesFromTL;
}

ascRasterHeader ascRasterFile::getAscRasterHeader(string inputLInes[], char separator)
{
	ascRasterHeader header;
	header.dataStartingLineIndex = -1;
	for (int ln = 0; ln < 7; ++ln)
	{
		string aline = inputLInes[ln];
		vector<string> LineParts = splitToStringVector(aline, separator);
		int iv = 0;
		double dv = 0;
		switch (ln)
		{
		case 0:
			header.nCols = stoi(LineParts[1]);
			break;
		case 1:
			header.nRows = stoi(LineParts[1]);
			break;
		case 2:
			header.xllcorner = stod(LineParts[1]);
			break;
		case 3:
			header.yllcorner = stod(LineParts[1]);
			break;
		case 4:
			if (toLower(LineParts[0]) == "dx")
			{
				if (isNumeric(LineParts[1]) == true)
				{
					header.dx = stof(LineParts[1]);
				}
				else
				{
					header.dx = -1;
				}
			}
			else if (toLower(LineParts[0]) == "cellsize")
			{
				if (isNumeric(LineParts[1]) == true)
				{
					header.cellsize = stof(LineParts[1]);
				}
				else
				{
					header.cellsize = -1;
				}
			}
			else
			{
				header.cellsize = -1;
			}
			break;
		case 5:
			if (toLower(LineParts[0]) == "nodata_value")
			{
				if (isNumeric(LineParts[1]) == false)
				{
					header.nodataValue = -9999;
				}
				else
				{
					header.nodataValue = stoi(LineParts[1]);
				}
			}
			else if (toLower(LineParts[0]) == "dy")
			{
				if (isNumeric(LineParts[1]) == true)
				{
					header.dy = stof(LineParts[1]);
				}
				else
				{
					header.dy = -1;
				}
			}
			else
			{
				header.nodataValue = -9999;
			}
			break;
		case 6:
			if (toLower(LineParts[0]) == "nodata_value")
			{
				if (isNumeric(LineParts[1]) == false)
				{
					header.nodataValue = -9999;
				}
				else
				{
					header.nodataValue = stoi(LineParts[1]);
				}
			}
			break;
		}
		if (ln > 4)
		{
			if (LineParts.size() > 0)
			{
				if (isNumeric(LineParts[0]) == true)
				{
					header.dataStartingLineIndex = ln;
					header.headerEndingLineIndex = ln - 1;
					return header;
				}
			}
		}
	}
	return header;
}

ascRasterExtent ascRasterFile::getAscRasterExtent(ascRasterHeader header)
{
	ascRasterExtent ext;
	ext.bottom = header.yllcorner;
	ext.top = header.yllcorner + header.nRows * header.cellsize;
	ext.left = header.xllcorner;
	ext.right = header.xllcorner + header.nCols * header.cellsize;
	ext.extentWidth = ext.right - ext.left;
	ext.extentHeight = ext.top - ext.bottom;
	return ext;
}

string ascRasterFile::makeHeaderString(int ncols, int nrows, double xll, double yll, float cellSize, float dx, float dy, int nodataValue)
{
	string headerall = "";
	headerall =  "ncols " + to_string(ncols) + "\n";
	headerall = headerall + "nrows " + to_string(nrows) + "\n";
	headerall = headerall + "xllcorner " + to_string(xll) + "\n";
	headerall = headerall + "yllcorner " + to_string(yll) + "\n";

	if (dx != dy && dx > 0 && dy > 0)
	{
		headerall = headerall + "dx" + " " + to_string(dx) + "\n";
		headerall = headerall + "dy" + " " + to_string(dy) + "\n";
	}
	else
	{
		headerall = headerall + "cellsize " + to_string(cellSize) + "\n";
	}
	
	headerall = headerall + "NODATA_value " + to_string(nodataValue) + "\n";
	return headerall;
}


int confirmDeleteFiles(vector<string> filePathNames)
{
	bool bAlldeleted  = false;
	int n = 0;
	while (!(bAlldeleted == true))
	{
		n += 1;
		for (string fpn : filePathNames)
		{
			if (fs::exists(fpn) == true)
			{
				
				std::remove(fpn.c_str());
			}
		}
		for (string fpn : filePathNames)
		{
			if (fs::exists(fpn) == false)
			{
				bAlldeleted = true;
				break;
			}
			else
			{
				bAlldeleted = false;
				break;
			}
		}
		if (n > 100) { return -1; }
	}
	return 1;
}

int confirmDeleteFile(string filePathNames)
{
	bool bAlldeleted = false;
	int n = 0;
	while (!(bAlldeleted == true))
	{
		n += 1;
		if (fs::exists(filePathNames) == true)
		{
			std::remove(filePathNames.c_str());
		}
		if (fs::exists(filePathNames) == false)
		{
			bAlldeleted = true;
			break;
		}
		else
		{
			bAlldeleted = false;
			break;
		}
		if (n > 100) { return -1; }
	}
	return 1;
}

bool isNumeric(string instr)
{
	return atoi(instr.c_str()) != 0 || instr.compare("0") == 0;
}
string getCPUinfo()
{
	CPUInfoDelegate *cpuInfo = new CPUInfoDelegate();
	std::vector<CPUInfo> cpuInfoVector = cpuInfo->cpuInfoVector();

		SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int CPUCount = 1;
	string infoStr;
	infoStr ="  " + std::to_string(cpuInfo->numberOfCPUInfoItems()) + " CPU(s) installed.\n";
	for (std::vector<CPUInfo>::iterator iter = cpuInfoVector.begin(); iter != cpuInfoVector.end(); ++iter) 
	{
		//std::cout << "CPU Manufacturer = " << iter->manufacturer() << std::endl;
		//std::cout << "Current CPU Clock Speed = " << iter->currentClockSpeed() << std::endl;
		//std::cout << "CPU Architecture = " << iter->architecture() << std::endl;
		//std::cout << "CPU L2 Cache Size = " << iter->L2CacheSize() << std::endl;
		//std::cout << "CPU L3 Cache Size = " << iter->L3CacheSize() << std::endl;
		//std::cout << "Current CPU Temperature = " << iter->currentTemperature() << std::endl;

		infoStr += "  CPU #" + to_string(CPUCount) + ".\n";
		infoStr += "    CPU name : " + iter->name() + '\n';
		infoStr += "    Number of CPU cores : " + iter->numberOfCores() + '\n';
		infoStr += "    Number of logical processors : " + std::to_string(sysInfo.dwNumberOfProcessors) + '\n';
		CPUCount++;
	}
	delete cpuInfo;
	return infoStr;
}


version getCurrentFileVersion()
{
	TCHAR fpn_exe[MAX_PATH];
	DWORD size = GetModuleFileName(NULL, fpn_exe, sizeof(fpn_exe));
	DWORD infoSize = 0;
	version ver;

	// 파일로부터 버전정보데이터의 크기가 얼마인지를 구합니다.
	infoSize = GetFileVersionInfoSize(fpn_exe, 0);
	if (infoSize == 0) return ver;

	// 버퍼할당
	char* buffer = NULL;
	buffer = new char[infoSize];

	if (buffer)
	{
		// 버전정보데이터를 가져옵니다.
		if (GetFileVersionInfo(fpn_exe, 0, infoSize, buffer) != 0)
		{
			VS_FIXEDFILEINFO* pFineInfo = NULL;
			UINT bufLen = 0;
			// buffer로 부터 VS_FIXEDFILEINFO 정보를 가져옵니다.
			if (VerQueryValue(buffer, "\\", (LPVOID*)&pFineInfo, &bufLen) != 0)
			{
				ver.major = HIWORD(pFineInfo->dwFileVersionMS);
				ver.minor = LOWORD(pFineInfo->dwFileVersionMS);
				ver.build = HIWORD(pFineInfo->dwFileVersionLS);
				//ver.LastWrittenTime = new char[30];
				struct _stat buf;
				if (_stat(fpn_exe, &buf) != 0)
				{
					switch (errno) {
					case ENOENT:
						fprintf(stderr, "File %s not found.\n", fpn_exe);
					case EINVAL:
						fprintf(stderr, "Invalid parameter to _stat.\n");
					default:
						fprintf(stderr, "Unexpected error in _stat.\n");
					}
					sprintf(ver.LastWrittenTime, "");
				}
				else
				{
					//printf("%s\n", fpn_exe);
					//printf("\tTime Creation     : %s\n", timeToString(localtime(&buf.st_ctime)));
					//printf("\tTime Last Written : %s\n", timeToString(localtime(&buf.st_mtime)));
					//printf("\tTime Last Access  : %s\n", timeToString(localtime(&buf.st_atime)));
					sprintf(ver.LastWrittenTime, timeToString(localtime(&buf.st_mtime)));
				}
			}
		}
		delete[] buffer;
	}
	return ver;
}

string getGPUinfo()
{
	GPUInfoDelegate *gpuInfo = new GPUInfoDelegate();
	std::vector<GPUInfo> gpuInfoVector = gpuInfo->gpuInfoVector();
	//std::cout << "This computer has " << gpuInfo->numberOfGPUInfoItems() << " GPU(s) installed" << std::endl;
	//int gpuCount = 1;
	//for (std::vector<GPUInfo>::const_iterator iter = gpuInfoVector.begin(); iter != gpuInfoVector.end(); iter++) {
	//	std::cout << "Information for GPU #" << gpuCount << ": " << std::endl;
	//	std::cout << "GPU Name = " << iter->name() << std::endl;
	//	std::cout << "GPU Manufacturer = " << iter->manufacturer() << std::endl;
	//	std::cout << "GPU Adapter RAM = " << iter->adapterRAM() << std::endl;
	//	std::cout << "GPU Refresh Rate = " << iter->refreshRate() << std::endl;
	//	std::cout << "GPU Driver Version = " << iter->driverVersion() << std::endl;
	//	std::cout << "GPU Video Architecture = " << iter->videoArchitecture() << std::endl;
	//	std::cout << "GPU Video Mode Description = " << iter->videoModeDescription() << std::endl;
	//	std::cout << std::endl;
	//	gpuCount++;
	//}

	//SYSTEM_INFO sysInfo;
	//GetSystemInfo(&sysInfo);
	//int CPUCount = 1;
	string infoStr;
	infoStr = "  " + std::to_string(gpuInfo->numberOfGPUInfoItems()) + " GPU(s) installed.\n";
	int gpuCount = 1;
	for (std::vector<GPUInfo>::const_iterator iter = gpuInfoVector.begin(); iter != gpuInfoVector.end(); ++iter)
	{
		infoStr += "  GPU #" + to_string(gpuCount) + ".\n";
		infoStr += "    GPU name : " + iter->name() + '\n';
		infoStr += "    GPU adapter ram : " + iter->adapterRAM() + '\n';
		infoStr += "    GPU driver version : " + iter->driverVersion() + '\n';
		gpuCount++;
	}
	delete gpuInfo;
	return infoStr;
}

string getValueStringFromXmlLine(string aLine, string fieldName)
{
	int len_fiedlName = 0;
	int pos1 = 0;
	string strToFind = "<"+ fieldName+">";
	pos1 = aLine.find(strToFind, 0);
	if (pos1 >= 0)
	{
		//pos1 = aLine.find("<DEMFile>", 0);
		len_fiedlName = strToFind.length();
		int pos2 = 0;
		string strToFind2 = "</" + fieldName + ">";
		pos2 = aLine.find(strToFind2);
		if (pos2 >= 0)
		{
			string valueString = "";
			int valueSize = 0;
			valueSize = pos2 - pos1 - len_fiedlName;
			valueString = aLine.substr(pos1 + len_fiedlName, valueSize);
			return valueString;
		}
		return "";
	}
	else
	{
		return "";
	}

}

// key별 속성을 vector<string>으로 저장해서 반환
map <int, vector<string>> readVatFile(string vatFPN, char seperator)
{
	map <int, vector<string>> values;
	ifstream vatFile(vatFPN);
	if (vatFile.is_open())
	{
		string aline;
		int r = 0;
		while (getline(vatFile, aline))
		{
			vector<string> parts = splitToStringVector(aline, seperator);
			int attValue = 0;
			if (parts.size() >1)
			{
				attValue = stoi(parts[0]);
				if (values.count(attValue) == 0)
				{
					parts.erase(parts.begin());
					values.insert(make_pair(attValue, parts));
				}
			}
			else
			{
				string outstr;
				outstr = "Values in VAT file (" + vatFPN + ") are invalid, or have no attributes.\n";
				cout << outstr;
				cout << "Each grid value must have one more attribute. \n";
			}
			r++;
		}
	}
	return values;
}

vector<string> readTextFileToStringVector(string fpn)
{
	ifstream txtFile(fpn);
	string aline;
	vector<string> linesv;
	while (!txtFile.eof())
	{
		getline(txtFile, aline);
		if (aline.size() > 0)
		{
			linesv.push_back(aline);
		}		
	}
	return linesv;
}


tm secToHHMMSS(long sec)
{
	tm t;
	t.tm_hour = sec / 3600;
	long remains;
	remains = sec % 3600;
	t.tm_min = remains / 60;
	remains = remains % 60;
	t.tm_sec = remains;
	return t;
}


/*
std::string split implementation by using delimeter as a character.
*/
vector<double> splitToDoubleVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	double v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<double> splittedValues;
	while (getline(ss, item, delimeter))
	{
		string sv = trim(item);
		if (removeEmptyEntry == true)
		{
			if (sv != "")
			{
				v = stod(sv);
				splittedValues.push_back(v);
			}
		}
		else
		{
			v = stod(sv);
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

/*
std::string split implementation by using delimeter as an another string
*/
vector<double> splitToDoubleVector(string stringToBeSplitted, string delimeter, bool removeEmptyEntry )
{
	vector<double> splittedValues;
	int startIndex = 0;
	int  endIndex = 0;
	while ((endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size())
	{
		string item = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
		string sv = trim(item);
		double val;
		if (removeEmptyEntry == true)
		{
			if (sv != "")
			{
				val = stod(sv);
				splittedValues.push_back(val);
			}
		}
		else
		{
			val = stod(sv);
			splittedValues.push_back(val);
		}
		startIndex = endIndex + delimeter.size();
	}
	if (startIndex < stringToBeSplitted.size())
	{
		string item = stringToBeSplitted.substr(startIndex);
		string sv = trim(item);
		double val;
		if (removeEmptyEntry == true)
		{
			if (sv != "")
			{
				val = stod(sv);
				splittedValues.push_back(val);
			}
		}
		else
		{
			val = stod(sv);
			splittedValues.push_back(val);
		}
	}
	return splittedValues;
}


vector<int> splitToIntVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	int v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<int> splittedValues;
	while (getline(ss, item, delimeter))
	{
		string sv = trim(item);
		if (removeEmptyEntry == true)
		{
			if (sv != "")
			{
				v = stod(sv);
				splittedValues.push_back(v);
			}
		}
		else
		{
			v = stod(sv);
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

vector<string> splitToStringVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	string v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<string> splittedValues;
	while (getline(ss, item, delimeter))
	{
		v = trim(item);
		if (removeEmptyEntry == true )			
		{
			if (v != "")
			{
				splittedValues.push_back(v);
			}
		}
		else
		{
			splittedValues.push_back(v);
		}

	}
	return splittedValues;
}

char* stringToCharP(string genericString)
{
	std::vector<char> writable(genericString.begin(), genericString.end());
	writable.push_back('\0');
	return &writable[0];
}

tm stringToDateTime(string yyyymmddHHMM) // 201711282310
{
	tm t;
	t.tm_year = stoi(yyyymmddHHMM.substr(0, 4));
	t.tm_mon = stoi(yyyymmddHHMM.substr(4, 2));
	t.tm_mday = stoi(yyyymmddHHMM.substr(6, 2));
	t.tm_hour = stoi(yyyymmddHHMM.substr(8, 2));
	t.tm_min = stoi(yyyymmddHHMM.substr(10, 2));
	return t;
}

tm stringToDateTime2(string yyyy_mm_dd__HHcolonMM) // 2017-11-28 23:10, 0123-56-89 12:45
{
	tm t;
	t.tm_year = stoi(yyyy_mm_dd__HHcolonMM.substr(0, 4));
	t.tm_mon = stoi(yyyy_mm_dd__HHcolonMM.substr(5, 2));
	t.tm_mday = stoi(yyyy_mm_dd__HHcolonMM.substr(8, 2));
	t.tm_hour = stoi(yyyy_mm_dd__HHcolonMM.substr(11, 2));
	t.tm_min = stoi(yyyy_mm_dd__HHcolonMM.substr(14, 2));
	return t;
}






char* timeToString(struct tm* t, int includeSEC) 
{
	static char s[20];
	if (includeSEC < 0)
	{
		sprintf(s, "%04d-%02d-%02d %02d:%02d",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min);// , t->tm_sec);
	}
	else
	{
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);
	}
	return s;
}

string timeToString(COleDateTime t, int includeSEC)
{
	string s;
	if (includeSEC < 0)
	{
		s = t.Format(_T("%Y-%m-%d %H:%M"));
	}
	else
	{
		s = t.Format(_T("%Y-%m-%d %H:%M:%S"));
	}
	return s;
}


string toLower(string instring)
{
	std::transform(instring.begin(), instring.end(), instring.begin(), tolower);
	return instring;
}

string toUpper(string instring)
{
	std::transform(instring.begin(), instring.end(), instring.begin(), toupper);
	return instring;
}


bool writeNewLog(const char* fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		printf(printText);
	}
	if (bprintFile > 0)
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows + " " + printText;
		outfile.close();

		//FILE* outFile;
		//outFile = fopen(fpn, "w");
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeNewLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		printf(printText);
	}
	if (bprintFile > 0)
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows + " " + printText;
		outfile.close();

		//FILE* outFile;
		//string pstr = fpn.string();
		//const char* fpn_cchar = pstr.c_str();
		//outFile = fopen(fpn_cchar, "w");
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeNewLog(fs::path fpn, string printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		cout << printText;
	}
	if (bprintFile > 0)
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows+" "+printText;
		outfile.close();
	}
	return true;
}

bool writeLog(const char* fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		printf(printText);
	}
	if (bprintFile > 0)
	{
		std::ofstream outfile;
		if (fs::exists(fpn) == false)
		{
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true)
		{
			outfile.open(fpn, ios::app);
		}
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		outfile << nows + " " + printText;
		outfile.close();

		//FILE* outFile;
		//int nResult = access(fpn, 0);
		//if (nResult == -1)
		//{
		//	outFile = fopen(fpn, "w");
		//}
		//else if (nResult == 0)
		//{
		//	outFile = fopen(fpn, "a");
		//}
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		printf(printText);
	}

	if (bprintFile > 0)
	{
		std::ofstream outfile;
		if (fs::exists(fpn) == false)
		{
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true)
		{
			outfile.open(fpn, ios::app);
		}
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		outfile << nows + " " + printText;
		outfile.close();

		//FILE* outFile;
		//string pstr = fpn.string();
		//const char* fpn_cchar = pstr.c_str();

		//if (fs::exists(fpn) == false)
		//{
		//	outFile = fopen(fpn_cchar, "w");
		//}
		//else if (fs::exists(fpn) == true)
		//{
		//	outFile = fopen(fpn_cchar, "a");
		//}
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeLog(fs::path fpn, string printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)
	{
		cout << printText;
	}
	if (bprintFile > 0)
	{
		std::ofstream outfile;
		if (fs::exists(fpn) == false)
		{
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true)
		{
			outfile.open(fpn, ios::app);
		}
		time_t now = time(0);
		tm *ltm = localtime(&now);
		string nows = timeToString(ltm);
		outfile << nows + " " + printText;
		outfile.close();
	}
	return true;
}






