using System.IO;

namespace G2DCore
{
    public  class cProject
    {
        private static cProject mProject;
        public Dataset.projectds prjds;
        public string fpnDEM;
        public string fpnLC;
        public string fpnLCVAT;
        public string prjFileName;
        public string prjFilePath;
        public double  simDur_hr;
        public int isRainfallApplied;
        public int isBCApplied;
        public int mBCCount;
        public int bcCellCountAll;
        public int BConditionInterval_min;

        public cVars.LandCoverType LCType;

        public cOutput output;
        public cHydro hydro;
        public cDomain domain;
        public cConditionData[] bcData;
        public G2DCore.Dataset.projectds.BoundaryConditionDataDataTable dtBCData;
        public cRainfall rainfall;

        //삭제 대상
        public string fpnTest_willbeDeleted;
        public string fpnInterAcell_willbeDeleted;
        public string hvalues_Acell_willbeDeleted;
        //

        public cProject()
        {
            InitG2D();
        }

        public void InitG2D()
        {
            domain = new cDomain();
            hydro = new cHydro();
            prjds = new G2DCore.Dataset.projectds();
            fpnLC = "";
            fpnDEM = "";
        }

        public static bool OpenProject(string prjFPN)
        {
            mProject = new cProject();
            if (File.Exists(prjFPN) == false)
            {
                cGenEnv.writelogNconsole(string.Format("{0} is not exist. ", prjFPN), true);
                return false;
            }
            mProject.prjds.ReadXml(prjFPN);
            Dataset.projectds.ProjectSettingsRow row =
                 (G2DCore.Dataset.projectds.ProjectSettingsRow)
                    mProject.prjds.ProjectSettings.Rows[0];
            mProject.prjFilePath = Path.GetDirectoryName(prjFPN);
            mProject.prjFileName = Path.GetFileName(prjFPN);
            mProject.fpnDEM = row.DEMFile;
            if (row.LandCoverFile != null && System.IO.File.Exists(row.LandCoverFile))
            {
                string lcvatfpn = row.LandCoverVatFile;
                if (System.IO.File.Exists(lcvatfpn) == false)
                {
                    cGenEnv.writelogNconsole(string.Format("Landcover VAT ({0}) file is not exist!!  ", lcvatfpn), true);
                    return false;
                }
                mProject.fpnLCVAT = lcvatfpn;
                mProject.fpnLC = row.LandCoverFile;
                mProject.LCType = cVars.LandCoverType.File;
            }
            else
            {
                mProject.fpnLC = "";
                mProject.LCType = cVars.LandCoverType.Constant;
            }
            mProject.isRainfallApplied = -1;
            if (row.IsRainfallFileNull()==false && File.Exists (row.RainfallFile .ToString () )== true )
            {
                mProject.isRainfallApplied = 1 ;
            }
            mProject.BConditionInterval_min = 0;
            if (row.IsBCDataInterval_minNull ()== false)
            {
                if (int.TryParse(row.BCDataInterval_min, out int iv))
                {
                    mProject.BConditionInterval_min = iv;
                }
            }
            mProject.simDur_hr = row.SimulationDuration_hr;
            cGenEnv.initializecGenEnv();
            cGenEnv.setValues(row);
            if (mProject.SetValuesFromPrjFile() == false) { return false; }
            mProject.output = new cOutput(mProject);

            // 삭제 대상
            mProject.fpnTest_willbeDeleted
                = Path.Combine(mProject.prjFilePath, "00_Summary_test.out");
            if (File.Exists(mProject.fpnTest_willbeDeleted) == true)
            { File.Delete(mProject.fpnTest_willbeDeleted); }
            mProject.fpnInterAcell_willbeDeleted = Path.Combine(mProject.prjFilePath, "00_Summary_Acell.out");
            if (File.Exists(mProject.fpnInterAcell_willbeDeleted) == true)
            { File.Delete(mProject.fpnInterAcell_willbeDeleted); }
            mProject.hvalues_Acell_willbeDeleted = "";
            // 여기까지 삭제 대상
            return true;
        }

        private bool SetValuesFromPrjFile()
        {
            hydro.SetValues(mProject);
            if (domain.SetValues(mProject) == false) { return false; } // 여기서 domain 설정

            mProject.rainfall = new cRainfall();
            if (mProject.isRainfallApplied == 1)
            {
                rainfall.setValues(mProject);
            }
            else
            {
                rainfall.rainfallinterval_min = 0;
            }

            isBCApplied = -1;
            mBCCount = 0;
            if (prjds.BoundaryConditionData.Rows.Count > 0)
            {
                for (int nr = 0; nr < prjds.BoundaryConditionData.Rows.Count ; nr++)
                {
                    Dataset.projectds.BoundaryConditionDataRow row =
                                (G2DCore.Dataset.projectds.BoundaryConditionDataRow)
                              prjds.BoundaryConditionData.Rows[nr];
                    if (row.IsDataFileNull() == false && File.Exists(row.DataFile) == true
                        && row.IsCellXYNull() == false && row.CellXY.ToString().Trim() != ""
                        && row.IsDataTypeNull() == false && row.DataType.ToString().Trim() != "")
                    {
                        isBCApplied = 1;
                        mProject.mBCCount += 1;
                    }
                }
                if (isBCApplied == -1)
                {
                    cGenEnv.writelogNconsole(string.Format("Boundary condition file is invalid or empty."), true);
                }
            }

            bcCellCountAll = 0;
            if (isBCApplied == 1)
            {
                mProject.bcData = new cConditionData[mProject.mBCCount];
                int nb = 0;
                for (int nr = 0; nr < prjds.BoundaryConditionData.Rows.Count; nr++)
                {
                    Dataset.projectds.BoundaryConditionDataRow row =
                           (G2DCore.Dataset.projectds.BoundaryConditionDataRow)
                              prjds.BoundaryConditionData.Rows[nr];
                    if (row.IsDataFileNull() == false && File.Exists(row.DataFile) == true
                        && row.IsCellXYNull() == false && row.CellXY.ToString().Trim() != ""
                        && row.IsDataTypeNull() == false && row.DataType.ToString().Trim() != "")
                    {
                        bcData[nb] = new cConditionData();
                        bcData[nb].SetValues(row);

                        for (int ibc = 0; ibc < bcData[nb].bcCells.Count; ibc++)
                        {
                            if (bcData[nb].bcCells[ibc].x > (domain.nCols - 1) || bcData[nb].bcCells[ibc].x < 0)
                            {
                                cGenEnv.writelogNconsole(string.Format("Boundary condition cell x poistion is invalid!! (0 ~ {0})", domain.nCols - 1), cGenEnv.bwritelog_process);
                                return false;
                            }
                            if (bcData[nb].bcCells[ibc].y > (domain.nRows - 1) || bcData[nb].bcCells[ibc].y < 0)
                            {
                                cGenEnv.writelogNconsole(string.Format("Boundary condition cell y poistion is invalid!! (1 ~ {0})", domain.nRows - 1), cGenEnv.bwritelog_process);
                                return false;
                            }
                        }
                        bcCellCountAll = bcCellCountAll + bcData[nb].bcCells.Count;
                        nb++;
                    }
                }
            }
            return true;
        }
        
        public string prjFileNameWithoutExt
        {
            get
            {
                return Path.GetFileNameWithoutExtension(prjFileName);
            }
        }


        public int rainfallInterval_min
        {
            get
            {
                return (int)rainfall.rainfallinterval_min;
                //return mrainfallDataInterval_min;
            }
            set
            {
                rainfall.rainfallinterval_min = value;
            }
        }

        public static cProject Current 
        {
            get
            {
                return mProject;
            }
        }
    }
}
