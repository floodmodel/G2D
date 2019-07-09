using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace G2DCore
{
    
    public struct stCVAtt
    {// -1 : false, 1: true
        public int isSimulatingCell;  // -1 : false, 1: true
        public int colxary;
        public int rowyary;
        public double elez;
        public int cvaryNum_atW;
        public int cvaryNum_atE;
        public int cvaryNum_atN;
        public int cvaryNum_atS;

        public double rc;
        public double impervR;

        public double dp_tp1;  // 새로 계산될 수심 t+dt
        public double dp_t; //현재 기존 수심
        public double hp_tp1;//z+d

        public double dfe; //e로의 흐름수심
        public double dfs; //s로의 흐름수심

        public double ve_tp1;
        public double vs_tp1;
        /// <summary>
        /// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
        /// </summary>
        public double slpe;

        /// <summary>
        /// water surface slope. dh/dx에서 기준은 i+1. 그러므로, +면 i 셀의 수위가 더 낮다는 의미
        /// </summary>
        public double slps;

        public double qe_tp1;
        public double qw_tp1;
        public double qs_tp1;
        public double qn_tp1;

        public double qe_t;
        public double qw_t;
        public double qs_t;
        public double qn_t;

        public double resd; //residual
    }


}
