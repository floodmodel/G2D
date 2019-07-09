using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Alea;

namespace G2DCore
{
    public class cConditionData
    {
        public List<cVars.CellPosition> bcCells = new List<cVars.CellPosition>();
        private  string mFpnBcData;
        public cVars.ConditionDataType conditionDataType;
        public  double[] bcValues;
        private  string[] mSeparatorXY = { " ", "," };
        private string[] mSeparatorCell = {"/" };

        public void SetValues(Dataset.projectds.BoundaryConditionDataRow row)
        {
            try
            {
                setBCCells(row);
                mFpnBcData = row.DataFile;
                conditionDataType = (cVars.ConditionDataType)Enum.Parse(typeof(cVars.ConditionDataType), row.DataType);
                if (File.Exists(mFpnBcData) == true)
                {
                    string[] lines = System.IO.File.ReadAllLines(mFpnBcData);

                    List<string> valueList = new List<string>();
                    valueList = lines.ToList<string>();
                    if (cGenEnv.isAnalyticSolution == false)// 해석해와 비교할때는 이거 주석처리. 
                    {
                        valueList.Insert(0, "0"); //항상 0에서 시작하게 한다. 급격한 수위변화를 막기 위해서,, 수문곡선은 완만하게 변한다. 
                    }
                    List<double> vlist = new List<double>();
                    for (int nr=0; nr<(valueList.Count-1);nr++)
                    {
                        if(valueList[nr].Trim()!="" )
                        {
                            double v = 0;
                            if (double.TryParse(valueList[nr].Trim(),out v) == true)
                            {
                                vlist.Add(v);
                            }
                        }
                    }
                    //valueList.ConvertAll(double.Parse);
                    bcValues = new double[vlist.Count];
                    bcValues = vlist.ToArray();
                }
                else
                {
                    cGenEnv.writelogNconsole(string.Format("Source data file ({0}) is not exist.", row.DataFile), cGenEnv.bwritelog_process);
                }
            }
            catch (Exception e)
            {
                cGenEnv.writelogNconsole(string.Format("{0} Exception was occured.", e), true);
                throw e;
            }
        }

        private void setBCCells(Dataset.projectds.BoundaryConditionDataRow row)
        {
            if (row.CellXY != "")
            {
                string[] cellString = row.CellXY.Split(mSeparatorCell, StringSplitOptions.RemoveEmptyEntries);
                for (int cellidx = 0; cellidx < cellString.Length; cellidx++)
                {
                    string[] cellPosXY = cellString[cellidx].Split(mSeparatorXY, StringSplitOptions.RemoveEmptyEntries);
                    cVars.CellPosition cp;
                    cp.x = int.Parse(cellPosXY[0]);// column index starts from 0
                    cp.y = int.Parse(cellPosXY[1]);// row index starts from 0
                    bcCells.Add(cp);
                }
            }
        }

        public static void getConditionDataUsingAry(cDomain dm, stCVAtt[] cvs, stCVAtt_add[] cvsadd, cConditionData[] cdInfo,
           int cdCount, int dataOrder, int dataInterval_min)
        {
            for (int sc = 0; sc < cdCount; sc++)
            {
                int ndiv;
                int cellCount = cdInfo[sc].bcCells.Count;
                if (cdInfo[sc].conditionDataType == cVars.ConditionDataType.Discharge)
                {
                    ndiv = cellCount;
                }
                else { ndiv = 1; }
                for (int nc = 0; nc < cellCount; nc++)
                {
                    int cx = cdInfo[sc].bcCells[nc].x;
                    int ry = cdInfo[sc].bcCells[nc].y;
                    int idx = dm.dminfo[cx, ry].cvid;
                    double vcurOrder = 0;
                    double vnextOrder = 0;
                    //이 조건은 데이터가 0.1~0.3까지 3개가 있을 경우, 모의는 0~0.3까지 4개의 자료를 이용한다.                        
                    //dataorder는 1부터 이고, 1번 데이터는 무조건 0이다, 
                    //dataorder 4의 vcurOrder= 0.3, vnextOrder=0 이다. 
                    if ((dataOrder) <= cdInfo[sc].bcValues.Length)
                    {
                        vcurOrder = cdInfo[sc].bcValues[dataOrder - 1] / ndiv;
                    }
                    if ((dataOrder) <= cdInfo[sc].bcValues.Length - 1)
                    {
                        vnextOrder = cdInfo[sc].bcValues[dataOrder] / ndiv;
                    }
                    //if (dataOrder == cdInfo[sc].bcValues.Length + 1)
                    //{
                    //    vcurOrder = cdInfo[sc].bcValues[dataOrder - 2]; //이건 마지막 자료를 한번 더 이용하는 것..
                    //}
                    cvsadd[idx].bcData_curOrder = vcurOrder;
                    cvsadd[idx].bcData_nextOrder = vnextOrder;
                    cvsadd[idx].bcData_curOrderStartedTime_sec = dataInterval_min * (dataOrder - 1) * 60;
                }
            }
        }

        public static int getbcArrayIndex(stBCinfo[] bcinfos, int cvid)
        {
            for (int i = 0; i < bcinfos.Length; i++)
            {
                if (bcinfos[i].cvid == cvid) { return i; }
            }
            return -1;
        }
    }
}
