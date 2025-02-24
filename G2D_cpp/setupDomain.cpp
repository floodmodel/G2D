#include "stdafx.h"
#include "gentle.h"
#include "g2d.h"


#include <time.h>

//using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern projectFile prj;
extern generalEnv ge;
extern domaininfo di;
extern domainCell **dmcells;
extern globalVinner gvi;
extern thisProcess ps;

extern cvatt * cvs;
extern cvattAddAtt * cvsAA;
extern cvattMaxValueAndFD* cvsMVnFD;
extern double* cvsele;
extern double* rfi_read_mPs;

int setupDomainAndCVinfo()
{
	if (prj.fpnDEM == "" || _access(prj.fpnDEM.c_str(), 0) == -1)	{
		string outstr = "ERROR : DEM file (" + prj.fpnDEM + ") in " + fpn_prj.string() + " is invalid.\n";
		writeLogString(fpn_log, outstr, 1, 1);
		return -1;
	}
	writeLogString(fpn_log, "Reading DEM file... ", 1, 1);
	ascRasterFile demfile = ascRasterFile(prj.fpnDEM);
	writeLogString(fpn_log, "completed.\n", 1, 1);
	string outstr_size = "";
	if (demfile.header.cellsize < 0.0 || demfile.header.dx <0.0 || demfile.header.dy<0.0) {
		outstr_size = "ERROR : Cell size (or dx, dy) is smaller than 0 m. \n";
		outstr_size = outstr_size + "ERROR : Please check the DEM ASCII file and cell size. \n";
		writeLogString(fpn_log, outstr_size, 1, 1);
		return -1;

	}
	bool sizeLog = false;
	if (demfile.header.cellsize <1.0 || demfile.header.dx < 1.0 || demfile.header.dy < 1.0) {
		outstr_size = "WARNNING : Cell size is smaller than 0m. \n";
		sizeLog = true;
	}
	if (demfile.header.dx != demfile.header.dy) {
		outstr_size = "WARNNING : The dx and dy values of the DEM file are different with each other. \n";
		sizeLog = true;
	}
	if (sizeLog == true) {
		outstr_size = outstr_size + "           Only TM coordinate system is available. \n";
		outstr_size = outstr_size + "           Please check the cell size. \n";
		outstr_size = outstr_size + "WARNNING : Simulation is proceeding...  ";
		outstr_size = outstr_size + "If you want to stop simulation, press Ctrl + C. \n";
		writeLogString(fpn_log, outstr_size, 1, 1);
	}
	
	ascRasterFile* lcfile = NULL;
	map <int, LCInfo> vatLC;
	ascRasterFile* icfile = NULL;
	if (prj.usingLCFile == 1) {
		if (prj.fpnLandCover != "" && _access(prj.fpnLandCover.c_str(), 0) == 0) {
			writeLogString(fpn_log, "Reading land cover file... ", 1, 1);
			lcfile = new ascRasterFile(prj.fpnLandCover);
			if (!lcfile) {
				writeLogString(fpn_log, "\nERROR : Dynamic allocation of land cover file was failed.\n", 1, 1);
				return -1;
			}
			if (lcfile->header.nCols != demfile.header.nCols ||
				lcfile->header.nRows != demfile.header.nRows ||
				lcfile->header.cellsize != demfile.header.cellsize)
			{
				writeLogString(fpn_log, "\nERROR : Land cover file region or cell size are not equal to the dem file.\n", 1, 1);
				return -1;
			}
			if (prj.fpnLandCoverVat != "" && _access(prj.fpnLandCoverVat.c_str(), 0) == 0) {
				vatLC = setLCvalueUsingVATfile(prj.fpnLandCoverVat);
			}
			else {
				string outstr = "\nERROR : Land cover vat file (" + prj.fpnLandCoverVat + ") in " +
					fpn_prj.string() + " is invalid.\n";
				writeLogString(fpn_log, outstr, 1, 1);
				return -1;
			}
			writeLogString(fpn_log, "completed.\n", 1, 1);
			//writeLog(fpn_log, "\nReading land cover file... completed.\n", 1, 0);
		}
		else {
			string outstr = "ERROR : Land cover file (" + prj.fpnLandCover + ") in "
				+ fpn_prj.string() + " is invalid.\n";
			writeLogString(fpn_log, outstr, 1, 1);
			return -1;
		}
	}

	if (prj.usingicFile == 1)	{
		if (prj.icFPN != "" && _access(prj.icFPN.c_str(), 0) == 0)		{
			writeLogString(fpn_log, "Reading initial condition raster file... ", 1, 1);
			icfile = new ascRasterFile(prj.icFPN);
			if (icfile->header.nCols != demfile.header.nCols ||
				icfile->header.nRows != demfile.header.nRows ||
				icfile->header.cellsize != demfile.header.cellsize)
			{
				writeLogString(fpn_log, "\nERROR : Initial condition file region or cell size are not equal to the dem file.\n", 1, 1);
				return -1;
			}
		}
		else		{
			string outstr = "\nERROR : Initial condition file (" + prj.icFPN + ") in "
				+ fpn_prj.string() + " is invalid.\n";
			writeLogString(fpn_log, outstr, 1, 1);
			return -1;
		}
		writeLogString(fpn_log, "completed.\n", 1, 1);
		//writeLog(fpn_log, "\nReading initial condition raster file... completed.\n", 1, 0);
	}
	di.cellSize = demfile.header.cellsize;
	if (di.cellSize > 0.0) {
		di.dx = di.cellSize;
		di.dy = di.cellSize;
	}
	else {
		di.dx = demfile.header.dx;
		di.dy = demfile.header.dy;
	}
	di.nRows = demfile.header.nRows;
	di.nCols = demfile.header.nCols;
	di.xll = demfile.header.xllcorner;
	di.yll = demfile.header.yllcorner;
	di.nodata_value = demfile.header.nodataValue;
	di.headerStringAll = demfile.headerStringAll;
	dmcells = new domainCell * [di.nCols];
	for (int i = 0; i < di.nCols; ++i)	{
		dmcells[i] = new domainCell[di.nRows];
	}
	vector<cvatt> cvsv;
	int idx = 0;
	vector <double> elezv;
	writeLogString(fpn_log, "Setting up domain data... ", 1, 1);
	for (int nr = 0; nr < di.nRows; ++nr) {
		int lcValue_bak = 0;
		if (prj.usingLCFile == 1) { lcValue_bak = vatLC.begin()->first; }
		for (int nc = 0; nc < di.nCols; ++nc) {
			if (demfile.valuesFromTL[nc][nr] == demfile.header.nodataValue) {
				dmcells[nc][nr].isInDomain = -1;
				dmcells[nc][nr].cvidx = -1;
			}
			else {
				cvatt cv;
				double elez;
				dmcells[nc][nr].isInDomain = 1;
				dmcells[nc][nr].cvidx = idx; //이 id는 cvsv의 배열 인덱스 와 같다. 0부터 시작
				cv.colx = nc;
				cv.rowy = nr;
				elez = demfile.valuesFromTL[nc][nr];
				cv.isBCcell = -1;
				//여기는 land cover 정보
				if (prj.usingLCFile == 1) {
					int lcvalue = (int)lcfile->valuesFromTL[nc][nr];
					if (lcvalue == lcfile->header.nodataValue) {
						string outstr = "\nWARNNING : Land cover value at [" + to_string(nc) + ", "
							+ to_string(nr) + "] has null value "
							+ to_string(lcfile->header.nodataValue) + ". "
							+ to_string(lcValue_bak) + " will be applied.\n";
						writeLogString(fpn_log, outstr, false, 1);
						cv.rc = vatLC[lcValue_bak].roughnessCoeff;
						cv.impervR = vatLC[lcValue_bak].imperviousRatio;
					}
					else {						
						if (vatLC.find(lcvalue) == vatLC.end()) {
							writeLogString(fpn_log, "\nERROR : Land cover data value ["+to_string(lcvalue)
								+ "] is not defined in VAT file.\n", 1, 1);
							return -1;
						}
						cv.rc = vatLC[lcvalue].roughnessCoeff;
						cv.impervR = vatLC[lcvalue].imperviousRatio;
						lcValue_bak = lcvalue;
					}
				}
				else {
					cv.rc = prj.roughnessCoeff;
					cv.impervR = prj.imperviousR;
				}
				cvsv.push_back(cv);//여기서는 모의 대상 셀(domain 내부의 셀)만 담는다. cvid는 cvsv의 index와 같다..
				elezv.push_back(elez);
				idx++;
			}
		}
	}
	writeLogString(fpn_log, "completed.\n", 1, 1);
	//writeLog(fpn_log, "\nSetting up domain data... completed.\n", 1, 0);
	cvs = new cvatt[cvsv.size()];
	cvsele = new double[cvsv.size()];
	std::copy(cvsv.begin(), cvsv.end(), cvs);
	std::copy(elezv.begin(), elezv.end(), cvsele);
	di.cellNnotNull = (int)cvsv.size();
	cvsAA = new cvattAddAtt[cvsv.size()];
	cvsMVnFD = new cvattMaxValueAndFD[cvsv.size()];
	rfi_read_mPs = new double[cvsv.size()](); // 이렇게 하면 0으로 초기화됨

	writeLogString(fpn_log, "Setting up control volume... ", 1, 1);
	for (int ncv = 0; ncv < cvsv.size(); ++ncv) {
		//여기서 좌우측 cv 값 부터 arrynum 정보를 업데이트. 
		//x, y 값을 이용해서 cvs, cvsAA 정보 설정
		int cx = cvs[ncv].colx;
		int ry = cvs[ncv].rowy;
		if (cx > 0 && cx < di.nCols - 1) {
			if (dmcells[cx - 1][ry].isInDomain == 1) {
				cvs[ncv].cvidx_atW = dmcells[cx - 1][ry].cvidx;
			}
			else {
				cvs[ncv].cvidx_atW = -1;
			}
			if (dmcells[cx + 1][ry].isInDomain == 1) {
				cvs[ncv].cvdix_atE = dmcells[cx + 1][ry].cvidx;
			}
			else {
				cvs[ncv].cvdix_atE = -1;
			}
		}
		if (cx == di.nCols - 1 && di.nCols > 1) {
			if (dmcells[cx - 1][ry].isInDomain == 1) {
				cvs[ncv].cvidx_atW = dmcells[cx - 1][ry].cvidx;
			}
			else {
				cvs[ncv].cvidx_atW = -1;
			}
			cvs[ncv].cvdix_atE = -1;
		}
		if (cx == 0 && di.nCols > 1) {
			if (dmcells[cx + 1][ry].isInDomain == 1) {

				cvs[ncv].cvdix_atE = dmcells[cx + 1][ry].cvidx;
			}
			else {
				cvs[ncv].cvdix_atE = -1;
			}
			cvs[ncv].cvidx_atW = -1;
		}
		if (ry > 0 && ry < di.nRows - 1) {
			if (dmcells[cx][ry - 1].isInDomain == 1) {
				cvs[ncv].cvidx_atN = dmcells[cx][ry - 1].cvidx;
			}
			else {
				cvs[ncv].cvidx_atN = -1;
			}
			if (dmcells[cx][ry + 1].isInDomain == 1) {
				cvs[ncv].cvidx_atS = dmcells[cx][ry + 1].cvidx;
			}
			else {
				cvs[ncv].cvidx_atS = -1;
			}
		}
		if (ry == di.nRows - 1 && di.nRows > 1) {
			if (dmcells[cx][ry - 1].isInDomain == 1) {
				cvs[ncv].cvidx_atN = dmcells[cx][ry - 1].cvidx;
			}
			else {
				cvs[ncv].cvidx_atN = -1;
			}
			cvs[ncv].cvidx_atS = -1;
		}
		if (ry == 0 && di.nRows > 1) {
			if (dmcells[cx][ry + 1].isInDomain == 1) {
				cvs[ncv].cvidx_atS = dmcells[cx][ry + 1].cvidx;
			}
			else {
				cvs[ncv].cvidx_atS = -1;
			}
			cvs[ncv].cvidx_atN = -1;
		}

		double icValue = 0.0;
		if (prj.isicApplied == 1 && icfile != NULL && prj.icDataType == fileOrConstant::File) {
			icValue = icfile->valuesFromTL[cx][ry];
		}
		else {
			icValue = prj.icValue_m;
		}

		cvsAA[ncv].initialConditionDepth_m = 0.0;
		if (prj.isicApplied == 1 && prj.icType == conditionDataType::Depth)
		{
			if (icValue < 0.0) { icValue = 0.0; }
			cvsAA[ncv].initialConditionDepth_m = icValue;
		}
		if (prj.isicApplied == 1 && prj.icType == conditionDataType::WaterLevel)
		{
			double icV = icValue - cvsele[ncv];// cvs[ncv].elez;
			if (icV < 0.0) { icV = 0.0; }
			cvsAA[ncv].initialConditionDepth_m = icV;
		}
	}
	writeLogString(fpn_log, "completed.\n", 1, 1);
	//writeLog(fpn_log, "\nSetting up control volume... completed.\n", 1, 0);

	if (prj.usingLCFile == 1 && lcfile->disposed == false) {
		delete lcfile;
	}
	if (prj.usingicFile == 1 && icfile->disposed == false) {
		delete icfile;
	}
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
			ainfo.imperviousRatio = 1;// 불투수율을 입력하지 않으면, 기본값으로 1을 적용한다.
		}
		vat.insert(make_pair(key, ainfo));
	}
	return vat;
}

int changeDomainElevWithDEMFile(double tnow_min, double tbefore_min)
{
	int isnormal = 1;
	int demEnded = 0;
	for (int i = 0; i < prj.DEMtoChangeCount; ++i) {
		double t_toChange_min = prj.dcs[i].timeToChangeDEM_min;
		if (tbefore_min < t_toChange_min && tnow_min >= t_toChange_min) {
			string demfpn = prj.dcs[i].fpnDEMtoChange;
			ascRasterFile demfile = ascRasterFile(demfpn);
			if (di.cellSize != demfile.header.cellsize) { isnormal = 0; break; }
			if (di.nRows != demfile.header.nRows) { isnormal = 0; break; }
			if (di.nCols != demfile.header.nCols) { isnormal = 0; break; }
			if (i == prj.DEMtoChangeCount - 1) { demEnded = 1; }
			omp_set_num_threads(ps.mdp);
#pragma omp parallel for schedule(guided)
			for (int i = 0; i < gvi.nCellsInnerDomain; ++i) {
				int nr = cvs[i].rowy;
				int nc = cvs[i].colx;
				cvsele[i] = demfile.valuesFromTL[nc][nr];
			}
			if (isnormal == 0) {
				writeLogString(fpn_log, "WARNNING : An error was occurred while changing dem file. Simulation continues... \n", 1, 1);
				demEnded = 1; // 한번 애러가 발생하면, 그 후의 DEM은 더이상 사용하지 않는다.
			}
			else if (isnormal == 1) {
				writeLogString(fpn_log, "DEM file was changed. \n", 1, 1);
			}
			break;
		}
	}
	return demEnded;
}





