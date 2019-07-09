using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Threading;
using System.Collections.Concurrent;
using Alea;

namespace G2DCore
{
     public class cDomain
    {
        public double dx;
        public int nRows;
        public int nCols;
        public double xll;
        public double yll;
        public double cellSize;
        public int nodata_value;
        public string headerStringAll;
        public domainAtt[,] dminfo;
        public  List<stCVAtt> mCVs;
        public stCVAtt[] mCVsAry;
        public stCVAtt_add[] mCVsAddAary;
        public List<cVars.DEMFileInfo> demfileLstToChange;

        public cDomain()
        {
            demfileLstToChange = new List<cVars.DEMFileInfo>() ;
        }

        public bool SetValues(cProject prj)
        {
            if (set_values_using_rasterFile(prj) == false) { return false; }
            setDEMFileToChange(prj.prjds .DEMFileToChange);
            return true;
        }

        public bool setDEMFileToChange(Dataset.projectds.DEMFileToChangeDataTable dt)
        {
            try
            {
                for (int nr = 0; nr < dt.Rows.Count; nr++)
                {
                    Dataset.projectds.DEMFileToChangeRow row =
                        (Dataset.projectds.DEMFileToChangeRow)dt.Rows[nr];
                    double minuteout = 0;
                    if (double.TryParse(row.TimeMinute, out minuteout) == true)
                    {
                        if (System.IO.File.Exists(row.DEMFile) == true && minuteout > 0)
                        {
                            cVars.DEMFileInfo nf;
                            nf.timeMinutes = minuteout;
                            nf.demFPN = row.DEMFile;
                            demfileLstToChange.Add(nf);
                        }
                    }
                }
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} exception was occurred.", e);
                return false;
            }
        }

        private bool set_values_using_rasterFile(cProject prj)
        {
            gentle.cAscRasterReader demfile =
                         new gentle.cAscRasterReader(prj.fpnDEM);
            gentle.cAscRasterReader lcfile =null;
            SortedList<int, cVars.LCInfo> vatLC=null;
            if (prj.LCType == cVars.LandCoverType.File)
            {
                lcfile = new gentle.cAscRasterReader(prj.fpnLC);
                if (lcfile .Header .numberCols != demfile .Header .numberCols  ||
                    lcfile .Header .numberRows != demfile .Header .numberRows ||
                    lcfile .Header .cellsize != demfile .Header .cellsize )
                {
                    cGenEnv.writelogNconsole(string.Format("Land cover file region or cell size are not equal to the dem file."),true);
                    return false;
                }

                vatLC = new SortedList<int, cVars.LCInfo>();
                vatLC=setLCvalueUsingVATfile(prj.fpnLCVAT);
                
            }
            gentle.cAscRasterReader icDatafile = null;
            if (prj.hydro.icType==cVars.InitialConditionType.File )
            {
                icDatafile= new gentle.cAscRasterReader(prj.hydro.fpn_ic);
                if (icDatafile.Header.numberCols != demfile.Header.numberCols ||
                    icDatafile.Header.numberRows != demfile.Header.numberRows ||
                    icDatafile.Header.cellsize != demfile.Header.cellsize)
                {
                    cGenEnv.writelogNconsole(string.Format("Initial condition file region or cell size are not equal to the dem file."), true);
                    return false;
                }
            }
            dx = demfile.Header.cellsize;
            nRows = demfile.Header.numberRows;
            nCols = demfile.Header.numberCols;
            cellSize = demfile.Header.cellsize;
            if (cellSize<1 )
            {
                cGenEnv.writelogNconsole(string.Format("Cell size is smaller than 1m. Only TM coordinate system was available. Please check the cell size."), true);
            }
            xll = demfile.Header.xllcorner;
            yll = demfile.Header.yllcorner;
            nodata_value = demfile.Header.nodataValue;
            headerStringAll = demfile.HeaderStringAll;

            dminfo = new domainAtt[nCols, nRows];
            mCVs = new List<stCVAtt>();

            int id = 0;
            for (int nr = 0; nr < nRows; nr++)
            {
                int lcValue_bak = 0;
                if (prj.LCType == cVars.LandCoverType.File) { lcValue_bak = vatLC.Keys[0]; }
                for (int nc = 0; nc < nCols; nc++)
                {
                    G2DCore.stCVAtt cv = new stCVAtt();
                    if (demfile.ValueFromTL (nc,nr) == demfile.Header.nodataValue)
                    {
                        dminfo[nc, nr].isInDomain = -1;
                        dminfo[nc, nr].cvid = -1;
                        //cv.isSimulatingCell = -1;
                        //cv.cvid = -1;
                    }
                    else
                    {
                        dminfo[nc, nr].isInDomain = 1;
                        dminfo[nc, nr].cvid = id;
                        //cv.isSimulatingCell = 1;
                        //cv.cvid = id; //이 id는 mCVs의 인덱스와 같다. 0부터 시작
                        cv.colxary = nc;
                        cv.rowyary = nr;
                        cv.elez = demfile.ValueFromTL(nc, nr);// ademRow[nc];
                        //여기는 land cover 정보
                        if (prj.LCType == cVars.LandCoverType.File)
                        {
                            if ((int) lcfile.ValueFromTL(nc, nr)== lcfile .Header .nodataValue )
                            {
                                cGenEnv.writelogNconsole(string.Format("Land cover value at [{0}, {1}] has null value {2}. {3} will be applied", nc, nr,lcfile .Header .nodataValue , lcValue_bak ), false );
                            }
                            cv.rc = vatLC[(int)lcfile.ValueFromTL (nc, nr)].roughnessCoeff;
                            cv.impervR  = vatLC[(int)lcfile.ValueFromTL(nc, nr)].imperviousRatio;
                            lcValue_bak = (int)lcfile.ValueFromTL(nc, nr);
                        }
                        else
                        {
                            cv.rc= cHydro.roughness;
                            cv.impervR = cHydro.imperviousR;

                        }
                        mCVs.Add(cv); //여기서는 모의 대상 셀(domain 내부의 셀)만 담는다. cvid는 mcv의 list index와 같다..
                        id++;
                    }
                }
            }
            mCVsAry = mCVs.ToArray(); //구조체 리스트는 변수 수정이 안되므로, 여기서 1 차원 배열로 변환해서 모든 모의에 사용한다.
            mCVsAddAary = new stCVAtt_add[mCVsAry.Length];

            for (int ncv = 0; ncv< mCVsAry.Length ;ncv++)
            {
                //여기서 좌우측 cv 값의 arrynum 정보를 업데이트. 
                int cx = mCVsAry[ncv].colxary;
                int ry = mCVsAry[ncv].rowyary;
                if (cx > 0 && cx < nCols - 1)
                {
                    if (dminfo[cx - 1, ry].isInDomain== 1)
                    {
                        mCVsAry[ncv].cvaryNum_atW = dminfo[cx - 1, ry].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atW = -1;
                    }
                    if (dminfo[cx + 1, ry].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atE = dminfo[cx + 1, ry].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atE = -1;
                    }
                }
                if (cx == nCols - 1 && nCols > 1)
                {
                    if (dminfo[cx - 1, ry].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atW = dminfo[cx - 1, ry].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atW = -1;
                    }
                    mCVsAry[ncv].cvaryNum_atE = -1;
                }
                if (cx == 0 && nCols > 1)
                {
                    if (dminfo[cx + 1, ry].isInDomain == 1)
                    {
                        
                        mCVsAry[ncv].cvaryNum_atE = dminfo[cx + 1, ry].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atE = -1;
                    }
                    mCVsAry[ncv].cvaryNum_atW = -1;
                }
                if (ry > 0 && ry < nRows - 1)
                {
                    if (dminfo[cx, ry - 1].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atN = dminfo[cx, ry-1].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atN = -1;
                    }
                    if (dminfo[cx, ry + 1].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atS = dminfo[cx, ry + 1].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atS = -1;
                    }
                }
                if (ry == nRows - 1 && nRows > 1)
                {
                    if (dminfo[cx, ry - 1].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atN = dminfo[cx, ry - 1].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atN = -1;
                    }
                    mCVsAry[ncv].cvaryNum_atS = -1;
                }
                if (ry == 0 && nRows > 1)
                {
                    if (dminfo[cx, ry + 1].isInDomain == 1)
                    {
                        mCVsAry[ncv].cvaryNum_atS = dminfo[cx, ry + 1].cvid;
                    }
                    else
                    {
                        mCVsAry[ncv].cvaryNum_atS = -1;
                    }
                    mCVsAry[ncv].cvaryNum_atN = -1;
                }

                //여기서, mCVsAddAary에 cvid 정보, 초기조건 정보 설정
                mCVsAddAary[ncv].cvid = ncv; //배열번호와 cvid가 같다.
                double icValue = 0;
                if (prj.hydro.icType == cVars.InitialConditionType.File)
                {
                    icValue=icDatafile.ValueFromTL(cx, ry);
                }
                else
                {
                    icValue = prj.hydro.icValue_meter;
                }

                mCVsAddAary[ncv].initialConditionDepth_meter = 0;
                if (prj.hydro.icDataType == cVars.ConditionDataType.Depth)
                {
                    if (icValue < 0) { icValue = 0; }
                    mCVsAddAary[ncv].initialConditionDepth_meter = icValue;
                }
                if (prj.hydro.icDataType == cVars.ConditionDataType.Height)
                {
                    double icV = icValue - mCVs[ncv].elez;
                    if (icV < 0) { icV= 0; }
                    mCVsAddAary[ncv].initialConditionDepth_meter = icV;
                }
            }
            return true;
        }

        /// <summary>
        /// In land cover vat file, the first value is grid value, the second is land cover name, 
        /// the third is roughness coefficient, and the forth is impervious ratio.
        /// </summary>
        /// <param name="fpnLCvat"></param>
        /// <returns></returns>
        private SortedList<int, cVars.LCInfo> setLCvalueUsingVATfile(string fpnLCvat)
        {
            SortedList<int, cVars.LCInfo> vat = new SortedList<int, cVars.LCInfo>();
            SortedList<int, string []> values= new SortedList<int, string []>();
            values = gentle.cTextFile.ReadVatFile(fpnLCvat, gentle.cTextFile.ValueSeparator.CSV);
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
                    vat.Add(tmpK,lcinfo);
                }
            }
            return vat;
        }

        public static bool changeDomainElevWithDEMFileUsingArray(string demfpn, cDomain dm, stCVAtt[] cvs)
        {
            gentle.cAscRasterReader demfile =
                        new gentle.cAscRasterReader(demfpn);
            if (dm.dx != demfile.Header.cellsize) { return false; }
            if (dm.nRows != demfile.Header.numberRows) { return false; }
            if (dm.nCols != demfile.Header.numberCols) { return false; }
            for (int nr = 0; nr < dm.nRows; nr++)
            {
                for (int nc = 0; nc < dm.nCols; nc++)
                {
                    int idx = dm.dminfo[nc, nr].cvid;
                    if (idx >= 0) { cvs[idx].elez = demfile.ValueFromTL (nc, nr); }
                }
            }
            return true;
        }

        public static bool changeDomainElevWithDEMFile(string demfpn, cDomain dm)
        {
            gentle.cAscRasterReader demfile =
                        new gentle.cAscRasterReader(demfpn);
            if (dm.dx != demfile.Header.cellsize) { return false; }
            if (dm.nRows != demfile.Header.numberRows) { return false; }
            if (dm.nCols != demfile.Header.numberCols) { return false; }
            for (int nr = 0; nr < dm.nRows; nr++)
            {
                for (int nc = 0; nc < dm.nCols ; nc++)
                {
                    dm.dminfo[nc, nr].elez = demfile.ValueFromTL (nc,nr);
                }
            }
            return true;
        }

        public int cvCount
        {
           get
            {
                return mCVs.Count ;
            }
        }
    }
}
