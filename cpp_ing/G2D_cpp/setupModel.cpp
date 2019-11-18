#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <io.h>
#include <cctype>

#include "gentle.h"
#include "g2dLib.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern projectFile prj;
extern domaininfo di;
extern domainAtt cvdatt;
extern vector<cvinfo> cvsv;
extern cvinfo * cvs;

// set_values_using_rasterFile(project prj) 이부분 구현할 차례
int setupDomainAndCVinfo()
{
	ascRasterFile demfile = ascRasterFile(prj.fpnDEM);

	if (prj.usingLCFile == 1)
	{
		ascRasterFile lcfile = ascRasterFile(prj.fpnLandCover);
		map <int, LCInfo> vatLC;

	if (lcfile.header.nCols != demfile.header.nCols ||
		lcfile.header.nRows != demfile.header.nRows ||
		lcfile.header.cellsize != demfile.header.cellsize)
	{
		appendTextAndCloseFile(fpn_log, "Land cover file region or cell size are not equal to the dem file.", 1, 1);
		return -1;
	}

	vatLC = setLCvalueUsingVATfile(prj.fpnLandCoverVat);
	}


	//if (prj.LCType == cVars.LandCoverType.File)
	//{
	//	lcfile = new gentle.cAscRasterReader(prj.fpnLC);
	//	if (lcfile.Header.numberCols != demfile.Header.numberCols ||
	//		lcfile.Header.numberRows != demfile.Header.numberRows ||
	//		lcfile.Header.cellsize != demfile.Header.cellsize)
	//	{
	//		cGenEnv.writelogNconsole(string.Format("Land cover file region or cell size are not equal to the dem file."), true);
	//		return false;
	//	}

	//	vatLC = new SortedList<int, cVars.LCInfo>();
	//	vatLC = setLCvalueUsingVATfile(prj.fpnLCVAT);

	//}

	return 1;
}


map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat)
{
	map<int, LCInfo> vat ;
	map<int, vector<string>> values ;
	values = gentle.cTextFile.ReadVatFile(fpnLCvat, gentle.cTextFile.ValueSeparator.COMMA);
	for (int n = 0; n < values.Count; n++)
	{
		int tmpK = values.Keys[n];
		if (vat.Keys.Contains(tmpK) == false)
		{
			cVars.LCInfo lcinfo;
			lcinfo.LCCode = tmpK;
			lcinfo.LCname = values[tmpK][0];
			lcinfo.roughnessCoeff = double.Parse(values[tmpK][1]);
			if ((values[tmpK] != null) && (values[tmpK].Length > 2))
			{
				lcinfo.imperviousRatio = double.Parse(values[tmpK][2]);
			}
			else { lcinfo.imperviousRatio = 1; }// 불투수율을 입력하지 않으면, 기본값으로 1을 적용한다.
			vat.Add(tmpK, lcinfo);
		}
	}
	return vat;
}