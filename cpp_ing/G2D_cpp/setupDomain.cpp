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
extern generalEnv genEnv;
extern domaininfo di;
extern domainCell **dmcells;

extern cvatt * cvs;
extern cvattAdd * cvsAdd;

int setupDomainAndCVinfo()
{
	ascRasterFile demfile = ascRasterFile(prj.fpnDEM);
	ascRasterFile *lcfile;
	map <int, LCInfo> vatLC;
	ascRasterFile *icfile;

	if (prj.usingLCFile == 1)
	{
		lcfile = new ascRasterFile(prj.fpnLandCover);
		if (lcfile->header.nCols != demfile.header.nCols ||
			lcfile->header.nRows != demfile.header.nRows ||
			lcfile->header.cellsize != demfile.header.cellsize)
		{
			writeLog(fpn_log, "Land cover file region or cell size are not equal to the dem file.", 1, 1);
			return -1;
		}

		vatLC = setLCvalueUsingVATfile(prj.fpnLandCoverVat);
	}

	if (prj.icType == fileOrConstant::File)
	{
		icfile = new ascRasterFile(prj.icFPN);
		if (icfile->header.nCols != demfile.header.nCols ||
			icfile->header.nRows != demfile.header.nRows ||
			icfile->header.cellsize != demfile.header.cellsize)
		{
			writeLog(fpn_log, "Initial condition file region or cell size are not equal to the dem file.", 1, 1);
			return -1;
		}
	}
	di.dx = demfile.header.cellsize;
	di.nRows = demfile.header.nRows;
	di.nCols = demfile.header.nCols;
	di.cellSize = demfile.header.cellsize;
	if (di.cellSize < 1)
	{
		writeLog(fpn_log,"Cell size is smaller than 1m. Only TM coordinate system was available. Please check the cell size.", 1,1);
	}
	di.xll = demfile.header.xllcorner;
	di.yll = demfile.header.yllcorner;
	di.nodata_value = demfile.header.nodataValue;
	di.headerStringAll = demfile.headerStringAll;
	dmcells = new domainCell*[di.nCols];
	for (int i = 0; i < di.nCols; ++i)
	{
		dmcells[i] = new domainCell[di.nRows];
	}
	vector<cvatt> cvsv;	
	int id = 0;
	for (int nr = 0; nr <di.nRows; ++nr)
	{
		int lcValue_bak = 0;
		if (prj.usingLCFile == 1) { lcValue_bak = vatLC.begin()->first; }
		for (int nc = 0; nc <di.nCols; ++nc)
		{
			cvatt cv ;
			if (demfile.valuesFromTL[nc][nr] == demfile.header.nodataValue)
			{
				dmcells[nc][nr].isInDomain = -1;
				dmcells[nc][nr].cvid = -1;
			}
			else
			{
				dmcells[nc][nr].isInDomain = 1;
				dmcells[nc][nr].cvid = id; //이 id는 cvsv의 배열 인덱스 와 같다. 0부터 시작
				cv.colx = nc;
				cv.rowy = nr;
				cv.elez = demfile.valuesFromTL[nc][nr];
				//여기는 land cover 정보
				if (prj.usingLCFile == 1)
				{
					if ((int)lcfile->valuesFromTL[nc][nr] == lcfile->header.nodataValue)
					{
						string outstr = "Land cover value at [" + to_string(nc) + ", " + to_string(nr) + "] has null value "
							+ to_string(lcfile->header.nodataValue) + ". " + to_string(lcValue_bak) + " will be applied.";
						writeLog(fpn_log, outstr, false, 1);
						cv.rc = vatLC[lcValue_bak].roughnessCoeff;
						cv.impervR = vatLC[lcValue_bak].imperviousRatio;
					}
					else
					{
						cv.rc = vatLC[(int)lcfile->valuesFromTL[nc][nr]].roughnessCoeff;
						cv.impervR = vatLC[(int)lcfile->valuesFromTL[nc][nr]].imperviousRatio;
						lcValue_bak = (int)lcfile->valuesFromTL[nc][nr];
					}
				}
				else
				{
					cv.rc = prj.roughnessCoeff;
					cv.impervR = prj.imperviousR;
				}
				cvsv.push_back(cv);//여기서는 모의 대상 셀(domain 내부의 셀)만 담는다. cvid는 cvsv의 index와 같다..
				id++;
			}
		}
	}
	cvs = &cvsv[0];//구조체 리스트는 변수 수정이 안되므로, 여기서 1 차원 배열로 변환해서 모든 모의에 사용한다.
	genEnv.cellCountNotNull = cvsv.size();
	cvsAdd = new cvattAdd[cvsv.size()];
	
	for (int ncv = 0; ncv < cvsv.size(); ++ncv)
	{
		//여기서 좌우측 cv 값 부터 arrynum 정보를 업데이트. 
		//x, y 값을 이용해서 cvs 정보 설정
		int cx = cvs[ncv].colx;
		int ry = cvs[ncv].rowy;
		if (cx > 0 && cx < di.nCols - 1)
		{
			if (dmcells[cx - 1][ry].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atW = dmcells[cx - 1][ry].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atW = -1;
			}
			if (dmcells[cx + 1][ry].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atE = dmcells[cx + 1][ry].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atE = -1;
			}
		}
		if (cx == di.nCols - 1 && di.nCols > 1)
		{
			if (dmcells[cx - 1][ry].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atW = dmcells[cx - 1][ry].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atW = -1;
			}
			cvs[ncv].cvaryNum_atE = -1;
		}
		if (cx == 0 && di.nCols > 1)
		{
			if (dmcells[cx + 1][ry].isInDomain == 1)
			{

				cvs[ncv].cvaryNum_atE = dmcells[cx + 1][ry].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atE = -1;
			}
			cvs[ncv].cvaryNum_atW = -1;
		}
		if (ry > 0 && ry < di.nRows - 1)
		{
			if (dmcells[cx][ry - 1].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atN = dmcells[cx][ry - 1].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atN = -1;
			}
			if (dmcells[cx][ry + 1].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atS = dmcells[cx][ry + 1].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atS = -1;
			}
		}
		if (ry == di.nRows - 1 && di.nRows > 1)
		{
			if (dmcells[cx][ry - 1].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atN = dmcells[cx][ry - 1].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atN = -1;
			}
			cvs[ncv].cvaryNum_atS = -1;
		}
		if (ry == 0 && di.nRows > 1)
		{
			if (dmcells[cx][ry + 1].isInDomain == 1)
			{
				cvs[ncv].cvaryNum_atS = dmcells[cx][ry + 1].cvid;
			}
			else
			{
				cvs[ncv].cvaryNum_atS = -1;
			}
			cvs[ncv].cvaryNum_atN = -1;
		}

		//여기서, mCVsAddAary에 cvid 정보, 초기조건 정보 설정
		//cvsAdd[ncv].cvid = ncv; //배열번호와 cvid가 같다.
		double icValue = 0;
		if (prj.icType == fileOrConstant::File)
		{
			icValue = icfile->valuesFromTL[cx][ry];
		}
		else
		{
			icValue = prj.icValue_m;
		}

		cvsAdd[ncv].initialConditionDepth_m = 0;
		if (prj.icDataType == conditionDataType::Depth)
		{
			if (icValue < 0) { icValue = 0; }
			cvsAdd[ncv].initialConditionDepth_m = icValue;
		}
		if (prj.icDataType == conditionDataType::Height)
		{
			double icV = icValue - cvs[ncv].elez;
			if (icV < 0) { icV = 0; }
			cvsAdd[ncv].initialConditionDepth_m = icV;
		}
	}
	
	delete lcfile;
	delete icfile;
	return 1;
}


map<int, LCInfo> setLCvalueUsingVATfile(string fpnLCvat)
{
	map<int, LCInfo> vat;
	map<int, vector<string>> values;
	values = readVatFile(fpnLCvat, ',');
	map <int, vector<string>>::iterator itermap;
	for (itermap = values.begin(); itermap != values.end(); ++itermap)
	{
		int key = itermap->first;
		vector<string> lcatt = itermap->second;
		LCInfo ainfo;
		ainfo.LCCode = key;
		ainfo.LCname = lcatt[0];
		ainfo.roughnessCoeff = stod(lcatt[1]);
		if (lcatt.size() > 2)
		{
			ainfo.imperviousRatio = stod(lcatt[2]);
		}
		else
		{
			ainfo.imperviousRatio = 1;// 불투수율을 입력하지 않으면, 기본값으로 1을 적용한다.
		}
		vat.insert(make_pair(key, ainfo));
	}
	return vat;
}


int changeDomainElevWithDEMFile(string demfpn, domaininfo indm, domainCell **indmcells, cvatt *incvs)
{
	ascRasterFile demfile = ascRasterFile(demfpn);
	if (indm.dx != demfile.header.cellsize) { return false; }
	if (indm.nRows != demfile.header.nRows) { return false; }
	if (indm.nCols != demfile.header.nCols) { return false; }
	for (int nr = 0; nr < indm.nRows; ++nr)
	{
		for (int nc = 0; nc < indm.nCols; ++nc)
		{
			int idx = indmcells[nc][nr].cvid;
			incvs[idx].elez = demfile.valuesFromTL[nc][nr];
		}
	}
	return 1;
}