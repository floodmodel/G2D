using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace G2DCore
{
    public class cVars
    {
        public const string CONST_FILENAME_TAG_DISCHARGE = "_Discharge";
        public const string CONST_FILENAME_TAG_DEPTH = "_Depth";
        public const string CONST_FILENAME_TAG_HEIGHT = "_Height";
        public const string CONST_FILENAME_TAG_VELOCITY = "_Velocity";
        public const string CONST_FILENAME_TAG_FLOWDIRECTION = "_FDirection";
        public const string CONST_FILENAME_TAG_RFGRID = "_RFGrid";
        public const string CONST_FILENAME_TAG_BCDATA = "_BC";
        public const string CONST_FILENAME_TAG_SOURCEALL = "_SourceAll";
        public const string CONST_FILENAME_TAG_SINKDATA = "_Sink";

        public const string CONST_OUTPUT_ASCFILE_EXTENSION = ".out";
        public const string CONST_OUTPUT_IMGFILE_EXTENSION = ".png";
        public const string CONST_OUTPUT_PROJECTIONFILE_EXTENSION = ".prj";

        public const string CONST_TIME_FIELD_NAME = "DataTime";

        public const int CONST_IMG_WIDTH = 600;
        public const int CONST_IMG_HEIGHT = 600;

        public enum FlowDirection4
        {
            //N=1, E = 4, S = 16, W=64, NONE=0
            E = 1, S = 3, W = 5, N = 7, NONE = 0
        };

        public enum FlowDirection8
        {
            //N=1, NE=2, E=4, SE=8, S=16,  SW=32, W=64, NW=128, NONE=0
            E = 1, SE = 2, S = 3, SW = 4, W = 5, NW = 6, N = 7, NE = 8, NONE = 0
        };


        public enum RainfallDataType
        {
            TextFileMAP, //1
            TextFileASCgrid, //2
            None //0
        };

        // 1:Discharge, 2:Depth, 3:Height, 4:None
        public enum ConditionDataType
        {
            ///<summary>Discharge : cms</summary> 
            Discharge, // 1
            /// <summary>Depth : m</summary>
            Depth,      //2
            Height,    //3
            None     //4
        };


        //public enum LandCoverCode
        //{
        //    WATR,
        //    URBN,
        //    BARE,
        //    WTLD,
        //    GRSS,
        //    FRST,
        //    AGRL,
        //    CONST_VALUE
        //}

        public enum LandCoverType
        {
            File,
            Constant
        }

        public enum InitialConditionType
        {
            File,
            Constant
        }

        public struct CellPosition
        {
            public int x;
            public int y;
        }

        public struct LCInfo
        {
            public int LCCode;
            public string LCname;
            public double roughnessCoeff;
            public double imperviousRatio;
        }

        public struct DEMFileInfo
        {
            public double timeMinutes;
            public string demFPN;
        };
    }

    public struct stFluxData
    {
        public double v;
        public double slp;
        public double dflow;
        public double q;
        public int fd; //E = 1, S = 3, W = 5, N = 7, NONE = 0
    }

    public struct stGlobalValues
    {// -1 : false, 1: true
        public double dx;
        public int nCols;
        public int nRows;
        public double dMinLimitforWet;
        public double dMinLimitforWet_ori;
        public double slpMinLimitforFlow;
        public double domainOutBedSlope;
        public double ConvgC_h;
        public double froudNCriteria;
        public int iNRmax_forCE;
        public int iGSmax;
        public int iNR;
        public int iGS;
        public double gravity;
        public int isDWE;
        public int isAnalyticSolution;
        public int isApplyVNC;
        public double dt_sec;

        public int bAllConvergedInThisGSiteration;
    }

    public struct stBCinfo
    {
        public int cvid;
        public double bcDepth_dt_m_tp1;
        public int bctype;
    }

    //GPU parameter 로 넘기는 매개변수를 최소화 하기 위해서 이것을 추가로 사용한다. 여기에 포함된 값은 gpu로 안넘긴다.
    public struct stCVAtt_add
    {// -1 : false, 1: true
        public int cvid;
        public double rfReadintensity_mPsec;
        public double sourceRFapp_dt_meter;
        public double bcData_curOrder;
        public double bcData_nextOrder;
        public double bcData_curOrderStartedTime_sec;

        public double initialConditionDepth_meter;

        /// <summary>
        /// cms
        /// </summary>
        public double Qmax_cms;
        public double vmax;
        public int fdmax; // N = 1, E = 4, S = 16, W = 64, NONE = 0
    }

    public struct domainAtt
    {
        public int isInDomain;
        public int cvid;
        public double elez;
    }
}
