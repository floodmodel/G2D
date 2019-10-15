using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Collections.Concurrent;
using gentle;

namespace G2DCore
{
    public class cRainfall
    {
        public  int rainfallDataType;
        public int rainfallinterval_min;
        private string mfpn_rf;
        private G2DCore.Dataset.projectds.RainfallDataTable mdtrainfallData;

        public cRainfall()
        {
            mdtrainfallData = new G2DCore.Dataset.projectds.RainfallDataTable() ;
        }

        public void setValues(cProject prj)
        {
            G2DCore.Dataset.projectds.ProjectSettingsRow row = (G2DCore.Dataset.projectds.ProjectSettingsRow)prj.prjds.ProjectSettings.Rows[0];

            //cVars.RainfallDataType rtype = (cVars.RainfallDataType)Enum.Parse(typeof(cVars.RainfallDataType), row.RainfallDataType);
            if (!row.IsRainfallDataTypeNull())
            {
                if (row.RainfallDataType.ToLower()== cVars.RainfallDataType.TextFileASCgrid.ToString().ToLower())
                {
                    rainfallDataType = 2;// cVars.RainfallDataType.TextFileASCgrid;
                }
                else if(row.RainfallDataType.ToLower() == cVars.RainfallDataType.TextFileMAP.ToString().ToLower())
                {
                    rainfallDataType = 1;// cVars.RainfallDataType.TextFileMAP;
                }
            }

            rainfallinterval_min = 0;
            if (int.TryParse(row.RainfallDataInterval_min, out int iv))
            {
                rainfallinterval_min = iv;
            }
            mfpn_rf = row.RainfallFile;
            int rf_order = 0;
            if (File.Exists(mfpn_rf) == true)
            {
                String[] Lines = System.IO.File.ReadAllLines(mfpn_rf);
                for (int nl = 0; nl < Lines.Length; nl++)
                {
                    if (string.IsNullOrEmpty(Lines[nl].Trim()))
                    {
                        return;
                    }
                    G2DCore.Dataset.projectds.RainfallRow ar = mdtrainfallData.NewRainfallRow();
                    rf_order++;
                    ar.Order = rf_order;
                    switch (rainfallDataType)
                    {
                        case 1:
                            ar.Rainfall = Path.GetFileName(Lines[nl]);
                            ar.DataFile = ar.Rainfall;
                            break;
                        case 2:
                            ar.Rainfall = Lines[nl].ToString();
                            ar.DataFile = mfpn_rf;
                            break;
                    }
                    if (cGenEnv.isDateTimeFormat == true)
                    {
                        ar.DataTime = gentle.cComTools.GetTimeToPrintOut(true, cGenEnv.eventStartTime, (int)(rainfallinterval_min * nl));
                    }
                    else
                    {
                        ar.DataTime = (rainfallinterval_min * nl).ToString();
                    }
                    mdtrainfallData.Rows.Add(ar);
                }
            }
            else
            {
                Console.WriteLine(string.Format("Rainfall file ({0}) is not exist.", mfpn_rf));
                return;
            }
        }

        public static int ReadRainfallAndGetIntensityUsingArray(cProject prj, int order, stCVAtt[] cvs, stCVAtt_add[] cvs_add)
        {
            if ((order - 1) < prj.rainfall.mdtrainfallData.Rows.Count)//강우자료 있으면, 읽어서 세팅
            {
                double inRF_mm = 0;
                int rfIntervalSEC = prj.rainfallInterval_min * 60;
                int rftype = prj.rainfall.rainfallDataType;
                G2DCore.Dataset.projectds.RainfallRow rfRow = (G2DCore.Dataset.projectds.RainfallRow)prj.rainfall.dtrainfallData.Rows[order - 1];
                string rfFpn = Path.Combine(rfRow.DataFile);
                try
                {
                    switch (rftype)
                    {
                        case 2:// cVars.RainfallDataType.TextFileASCgrid:
                            gentle.cAscRasterReader ascReader = new gentle.cAscRasterReader(rfFpn);
                            var po = new ParallelOptions { MaxDegreeOfParallelism = cGenEnv.maxDegreeParallelism };
                            Parallel.For(0, prj.domain.nRows, po, delegate (int nr)
                            {
                                for (int nc = 0; nc < prj.domain.nCols; nc++)
                                {
                                    if (prj.domain.dminfo[nc, nr].isInDomain == 1)
                                    {
                                        int idx = prj.domain.dminfo[nc, nr].cvid;
                                        inRF_mm = ascReader.ValueFromTL (nc, nr);
                                        if (inRF_mm <= 0)
                                        {
                                            cvs_add[idx].rfReadintensity_mPsec = 0;
                                        }
                                        else
                                        {
                                            cvs_add[idx].rfReadintensity_mPsec = inRF_mm / 1000 / rfIntervalSEC;
                                            cThisProcess.rfisGreaterThanZero = 1;
                                        }
                                    }
                                }
                            });
                            break;
                        case 1:// cVars.RainfallDataType.TextFileMAP:
                            if (gentle.cComTools.IsNumeric(rfRow.Rainfall) == true)
                            {
                                inRF_mm = double.Parse(rfRow.Rainfall);
                            }
                            else
                            {
                                cGenEnv.writelogNconsole(string.Format("Can not read rainfall value!   " + System.Environment.NewLine + "Order = " + order.ToString()),
                                    cGenEnv.bwritelog_process);
                            }
                            if (inRF_mm > 0)
                            { cThisProcess.rfisGreaterThanZero = 1; }
                            else 
                            { inRF_mm = 0; }

                            cThisProcess.rfReadintensityForMAP_mPsec = inRF_mm / 1000 / rfIntervalSEC;
                            break;
                    }
                    return -1;
                }
                catch (Exception e)
                {
                    cGenEnv.writelogNconsole(string.Format("{0} Exception was occured.", e), true);
                    return 1;
                }
            }
            return 1;
        }

        public G2DCore.Dataset.projectds.RainfallDataTable  dtrainfallData
        {
        get
            {
                return mdtrainfallData;
            }
            set
            {
                mdtrainfallData = value;
            }
        }
    }    
}
