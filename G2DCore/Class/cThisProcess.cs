using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace G2DCore
{
    public class cThisProcess
    {
        public static double[] subregionVmax;
        public static double[] subregionDflowmax;
        public static double[] subregionVNCmin;
        public static bool bAllConvergedInThisGSiteration;
        public static int maxNR_inME = 0;
        public static double maxResd = 0;
        public static string maxResdCell = "";
        public static double[] subregionMaxResd ;
        public static string[] subregionMaxResdCell;
        public static int effCellCount = 0;
        public static List<int> FloodingCellCounts; // the number of cells that have water depth.
        //public static List<double> FloodingCellMaxDepth; 
        public static List<double> FloodingCellMeanDepth;
        public static double FloodingCellMaxDepth;
        public static double rfReadintensityForMAP_mPsec = 0;
        public static int rfisGreaterThanZero = 1; // 1:true, -1: false
    }
}
