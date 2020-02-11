#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <io.h>
#include <cctype>
#include <omp.h>

#include "gentle.h"
#include "g2d.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern projectFile prj;
extern generalEnv ge;
extern domaininfo di;
extern domainCell **dmcells;
extern globalVinner gvi[1];

extern cvatt * cvs;
extern cvattAdd * cvsAA;

int setupDomainAndCVinfo()
{
	if (prj.fpnDEM == "" || _access(prj.fpnDEM.c_str(), 0) == -1)	{
		string outstr = "DEM file (" + prj.fpnDEM + ") in " + fpn_prj.string() + " is invalid.\n";
		writeLog(fpn_log, outstr, 1, 1);
		return -1;
	}
	ascRasterFile demfile = ascRasterFile(prj.fpnDEM);
	ascRasterFile* lcfile = NULL;
	map <int, LCInfo> vatLC;
	ascRasterFile* icfile = NULL;
	if (prj.usingLCFile == 1) {
		if (prj.fpnLandCover != "" && _access(prj.fpnLandCover.c_str(), 0) == 0) {
			lcfile = new ascRasterFile(prj.fpnLandCover);
			if (!lcfile) {
				writeLog(fpn_log, "Land cover file ���� �Ҵ� ����.\n", 1, 1);
				return -1;
			}
			if (lcfile->header.nCols != demfile.header.nCols ||
				lcfile->header.nRows != demfile.header.nRows ||
				lcfile->header.cellsize != demfile.header.cellsize)
			{
				writeLog(fpn_log, "Land cover file region or cell size are not equal to the dem file.\n", 1, 1);
				return -1;
			}
			if (prj.fpnLandCoverVat != "" && _access(prj.fpnLandCoverVat.c_str(), 0) == 0) {
				vatLC = setLCvalueUsingVATfile(prj.fpnLandCoverVat);
			}
			else {
				string outstr = "Land cover vat file (" + prj.fpnLandCoverVat + ") in " +
					fpn_prj.string() + " is invalid.\n";
				writeLog(fpn_log, outstr, 1, 1);
				return -1;
			}

		}
		else {
			string outstr = "Land cover file (" + prj.fpnLandCover + ") in "
				+ fpn_prj.string() + " is invalid.\n";
			writeLog(fpn_log, outstr, 1, 1);
			return -1;
		}
	}

	if (prj.usingicFile == 1)	{
		if (prj.icFPN != "" && _access(prj.icFPN.c_str(), 0) == 0)		{
			icfile = new ascRasterFile(prj.icFPN);
			if (icfile->header.nCols != demfile.header.nCols ||
				icfile->header.nRows != demfile.header.nRows ||
				icfile->header.cellsize != demfile.header.cellsize)
			{
				writeLog(fpn_log, "Initial condition file region or cell size are not equal to the dem file.\n", 1, 1);
				return -1;
			}
		}
		else		{
			string outstr = "Initial condition file (" + prj.icFPN + ") in "
				+ fpn_prj.string() + " is invalid.\n";
			writeLog(fpn_log, outstr, 1, 1);
			return -1;
		}
	}
	di.dx = demfile.header.cellsize;
	di.nRows = demfile.header.nRows;
	di.nCols = demfile.header.nCols;
	di.cellSize = demfile.header.cellsize;
	if (di.cellSize < 1)	{
		string outstr = "Cell size is smaller than 1m. ";
		outstr = outstr + "Only TM coordinate system was available. ";
		outstr = outstr + "Please check the cell size. \n";
		outstr = outstr + "Simulation is proceeding. ";
		outstr = outstr + "If you want to stop simulation press Ctrl + C. \n";
		writeLog(fpn_log, outstr, 1, 1);
	}
	di.xll = demfile.header.xllcorner;
	di.yll = demfile.header.yllcorner;
	di.nodata_value = demfile.header.nodataValue;
	di.headerStringAll = demfile.headerStringAll;
	dmcells = new domainCell * [di.nCols];
	for (int i = 0; i < di.nCols; ++i)	{
		dmcells[i] = new domainCell[di.nRows];
	}
	vector<cvatt> cvsv;
	int id = 0;
	for (int nr = 0; nr < di.nRows; ++nr) {
		int lcValue_bak = 0;
		if (prj.usingLCFile == 1) { lcValue_bak = vatLC.begin()->first; }
		for (int nc = 0; nc < di.nCols; ++nc) {
			cvatt cv;
			if (demfile.valuesFromTL[nc][nr] == demfile.header.nodataValue) {
				dmcells[nc][nr].isInDomain = -1;
				dmcells[nc][nr].cvid = -1;
			}
			else {
				dmcells[nc][nr].isInDomain = 1;
				dmcells[nc][nr].cvid = id; //�� id�� cvsv�� �迭 �ε��� �� ����. 0���� ����
				cv.colx = nc;
				cv.rowy = nr;
				cv.elez = demfile.valuesFromTL[nc][nr];
				//����� land cover ����
				if (prj.usingLCFile == 1) {
					if ((int)lcfile->valuesFromTL[nc][nr] == lcfile->header.nodataValue) {
						string outstr = "Land cover value at [" + to_string(nc) + ", "
							+ to_string(nr) + "] has null value "
							+ to_string(lcfile->header.nodataValue) + ". "
							+ to_string(lcValue_bak) + " will be applied.";
						writeLog(fpn_log, outstr, false, 1);
						cv.rc = vatLC[lcValue_bak].roughnessCoeff;
						cv.impervR = vatLC[lcValue_bak].imperviousRatio;
					}
					else {
						cv.rc = vatLC[(int)lcfile->valuesFromTL[nc][nr]].roughnessCoeff;
						cv.impervR = vatLC[(int)lcfile->valuesFromTL[nc][nr]].imperviousRatio;
						lcValue_bak = (int)lcfile->valuesFromTL[nc][nr];
					}
				}
				else {
					cv.rc = prj.roughnessCoeff;
					cv.impervR = prj.imperviousR;
				}
				cvsv.push_back(cv);//���⼭�� ���� ��� ��(domain ������ ��)�� ��´�. cvid�� cvsv�� index�� ����..
				id++;
			}
		}
	}
	cvs = new cvatt[cvsv.size()];
	copy(cvsv.begin(), cvsv.end(), cvs);
	//cvs = &cvsv[0];//c#���� ����ü ����Ʈ�� ���� ������ �ȵǹǷ�, ���⼭ 1 ���� �迭�� ��ȯ�ؼ� ��� ���ǿ� ����Ѵ�.
	di.cellCountNotNull = (int)cvsv.size();
	cvsAA = new cvattAdd[cvsv.size()];

	for (int ncv = 0; ncv < cvsv.size(); ++ncv) {
		//���⼭ �¿��� cv �� ���� arrynum ������ ������Ʈ. 
		//x, y ���� �̿��ؼ� cvs ���� ����
		int cx = cvs[ncv].colx;
		int ry = cvs[ncv].rowy;
		if (cx > 0 && cx < di.nCols - 1) {
			if (dmcells[cx - 1][ry].isInDomain == 1) {
				cvs[ncv].cvaryNum_atW = dmcells[cx - 1][ry].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atW = -1;
			}
			if (dmcells[cx + 1][ry].isInDomain == 1) {
				cvs[ncv].cvaryNum_atE = dmcells[cx + 1][ry].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atE = -1;
			}
		}
		if (cx == di.nCols - 1 && di.nCols > 1) {
			if (dmcells[cx - 1][ry].isInDomain == 1) {
				cvs[ncv].cvaryNum_atW = dmcells[cx - 1][ry].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atW = -1;
			}
			cvs[ncv].cvaryNum_atE = -1;
		}
		if (cx == 0 && di.nCols > 1) {
			if (dmcells[cx + 1][ry].isInDomain == 1) {

				cvs[ncv].cvaryNum_atE = dmcells[cx + 1][ry].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atE = -1;
			}
			cvs[ncv].cvaryNum_atW = -1;
		}
		if (ry > 0 && ry < di.nRows - 1) {
			if (dmcells[cx][ry - 1].isInDomain == 1) {
				cvs[ncv].cvaryNum_atN = dmcells[cx][ry - 1].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atN = -1;
			}
			if (dmcells[cx][ry + 1].isInDomain == 1) {
				cvs[ncv].cvaryNum_atS = dmcells[cx][ry + 1].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atS = -1;
			}
		}
		if (ry == di.nRows - 1 && di.nRows > 1) {
			if (dmcells[cx][ry - 1].isInDomain == 1) {
				cvs[ncv].cvaryNum_atN = dmcells[cx][ry - 1].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atN = -1;
			}
			cvs[ncv].cvaryNum_atS = -1;
		}
		if (ry == 0 && di.nRows > 1) {
			if (dmcells[cx][ry + 1].isInDomain == 1) {
				cvs[ncv].cvaryNum_atS = dmcells[cx][ry + 1].cvid;
			}
			else {
				cvs[ncv].cvaryNum_atS = -1;
			}
			cvs[ncv].cvaryNum_atN = -1;
		}

		//���⼭, mCVsAddAary�� cvid ����, �ʱ����� ���� ����
		//cvsAdd[ncv].cvid = ncv; //�迭��ȣ�� cvid�� ����.
		double icValue = 0;
		if (prj.isicApplied == 1 && icfile != NULL && prj.icDataType == fileOrConstant::File) {
			icValue = icfile->valuesFromTL[cx][ry];
		}
		else {
			icValue = prj.icValue_m;
		}

		cvsAA[ncv].initialConditionDepth_m = 0.0;
		if (prj.isicApplied == 1 && prj.icType == conditionDataType::Depth)
		{
			if (icValue < 0) { icValue = 0; }
			cvsAA[ncv].initialConditionDepth_m = icValue;
		}
		if (prj.isicApplied == 1 && prj.icType == conditionDataType::Height)
		{
			double icV = icValue - cvs[ncv].elez;
			if (icV < 0) { icV = 0; }
			cvsAA[ncv].initialConditionDepth_m = icV;
		}
	}

	if (prj.usingLCFile == 1 && lcfile->disposed == false) {
		delete lcfile;
	}
	if (prj.usingicFile == 1 && icfile->disposed == false) {
		delete icfile;
	}

	writeLog(fpn_log, "Setting domain data and control volume information were completed.\n",
		prj.writeLog, prj.writeLog);
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
		ainfo.roughnessCoeff = stof(lcatt[1]);
		if (lcatt.size() > 2) {
			ainfo.imperviousRatio = stof(lcatt[2]);
		}
		else {
			ainfo.imperviousRatio = 1;// ���������� �Է����� ������, �⺻������ 1�� �����Ѵ�.
		}
		vat.insert(make_pair(key, ainfo));
	}
	return vat;
}


int changeDomainElevWithDEMFile(double tnow_min, double tbefore_min)
{
	int isnormal = 1;
	int demEnded = -1;
	int demFileWasChanged = -1;
	for (int i = 0; i < prj.DEMtoChangeCount; ++i) {
		double minuteout = prj.timeToChangeDEM_min[i];
		if (tbefore_min < minuteout && tnow_min >= minuteout) {
			string demfpn = prj.fpnDEMtoChange[i];
			ascRasterFile demfile = ascRasterFile(demfpn);
			if (di.dx != demfile.header.cellsize) { isnormal = -1; break; }
			if (di.nRows != demfile.header.nRows) { isnormal = -1; break; }
			if (di.nCols != demfile.header.nCols) { isnormal = -1; break; }
			if (i == prj.DEMtoChangeCount - 1) { demEnded = 1; }
			int nchunk;
			omp_set_num_threads(gvi[0].mdp);
			//prj.isParallel == 1 �� ��쿡�� gvi[0].mdp > 0 �� �����
			nchunk = gvi[0].nCellsInnerDomain / gvi[0].mdp;
#pragma omp parallel for schedule(guided, nchunk) 
			for (int i = 0; i < gvi[0].nCellsInnerDomain; ++i) {
				int nr = cvs[i].rowy;
				int nc = cvs[i].colx;
				cvs[i].elez = demfile.valuesFromTL[nc][nr];
			}
			//for (int nr = 0; nr < di.nRows; ++nr) {
			//	for (int nc = 0; nc < di.nCols; ++nc) {
			//		int idx = dmcells[nc][nr].cvid;
			//		if (idx >= 0) {
			//			cvs[idx].elez = (float)demfile.valuesFromTL[nc][nr];
			//		}					
			//	}
			//}
			demFileWasChanged = 1;
			break;
		}
	}
	if (isnormal == -1) {
		writeLog(fpn_log, "An error was occurred while changing dem file. Simulation continues... \n", 1, 1);
		demEnded = 1; // �ѹ� �ַ��� �߻��ϸ�, �� ���� DEM�� ���̻� ������� �ʴ´�.
	}
	if (demFileWasChanged == 1 && isnormal == 1) {
		writeLog(fpn_log, "DEM file was changed. \n", 1, 1);
	}
	return demEnded;
}

void setEffectiveCells(int idx)
{
	cvs[idx].isSimulatingCell = 1;
	if (cvs[idx].cvaryNum_atE >= 0) { cvs[cvs[idx].cvaryNum_atE].isSimulatingCell = 1; }
	if (cvs[idx].cvaryNum_atW >= 0) { cvs[cvs[idx].cvaryNum_atW].isSimulatingCell = 1; }
	if (cvs[idx].cvaryNum_atN >= 0) { cvs[cvs[idx].cvaryNum_atN].isSimulatingCell = 1; }
	if (cvs[idx].cvaryNum_atS >= 0) { cvs[cvs[idx].cvaryNum_atS].isSimulatingCell = 1; }
}



