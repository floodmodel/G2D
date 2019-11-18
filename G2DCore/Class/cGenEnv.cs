using System;
using System.IO;
using System.Text;
using Alea;
using System.Linq;
using System.Management;
using System.Collections.Generic;


namespace G2DCore
{
    public class cGenEnv
    {
        public static bool modelSetupIsNormal = true;
        public static string fpnlog = "";
        public static bool isDateTimeFormat = false;
        public static string eventStartTime = "";
        public static bool bMakeASCFile = true;
        public static bool bMakeImgFile = false;
        public static bool bwritelog_process = false;
        public static bool bwritelog_error = true;

        public static double dt_sec;
        public static int isfixeddt;// -1 : false, 1: true
        public static int isparallel;// -1 : false, 1: true
        public static int tsec_targetToprint;
        public static double tnow_min;
        public static double tnow_sec;
        public static DateTime simulationStartTime;
        public static DateTime thisPrintStepStartTime;
        public static double dt_printout_min;
        public static  int dt_printout_sec;

        public static double gravity;
        public static double dMinLimitforWet; // 이거보다 같거나 작으면 마른 것이다.
        public static double dMinLimitforWet_ori;
        public static double slpMinLimitforFlow; //이거보다 작으면 경사가 없는 것이다.
        public static double dtMaxLimit_sec;
        public static double dtMinLimit_sec;
        public static double dtStart_sec;
        public static double dflowmaxInThisStep; // courant number 계산용
        public static double vmaxInThisStep;
        public static double VNConMinInThisStep;
        public static double convergenceConditionh;
        public static double convergenceConditionhr;
        public static double convergenceConditionq;
        public static int iNRmax_forCE;
        public static int iNRmax_forME ;
        public static int iGSmax;
        public static int maxDegreeParallelism;
        public static bool isAnalyticSolution = false;
        public static bool isDWE = false ;
        public static bool vdtest = false;
        public static bool movingDomain = true;
        public static int iGS = 0;
        public static int iNR = 0;
        public static int iGSmax_GPU = 0;
        public static int iNRmax_GPU = 0;
        public static int EffCellThresholdForGPU = 0; //usingGPU==true 일경우, eff cell 개수가 이 숫자보다 크면 gpu를 이용한다.
        public static List<double> floodingCellDepthThresholds_m ;// 수렴 조건 적용
        public static int usingGPU = 1;// -1 : false, 1: true
        public static void initializecGenEnv()
        {
            gravity = 9.81; // 1;
            dMinLimitforWet_ori = 0.000001;// 0.00001;// 이게 0이면, 유량 계산시 수심으로 나누는 부분에서 발산. 유속이 크게 계산된다..
                                            // 이 값은 1. 주변셀과의 흐름 계산을 할 셀(effective 셀) 결정시 사용되고,
                                            //            2. 이 값보다 작은 셀은 이 셀에서 외부로의 유출은 없게 된다. 외부에서 이 셀로의 유입은 가능
                                            //            3. 생성항(강우, 유량 등)에 의한 유량 추가는 가능하다.
            dMinLimitforWet = dMinLimitforWet_ori;
            //slpMinLimitforFlow = 0.0001; //음해
            slpMinLimitforFlow = 0;// 양해

            if (isAnalyticSolution == true)
            {
                dtMaxLimit_sec = 2;// 600; //해석해 하고 비교할때는 1 이 아주 잘 맞는다..
                dtMinLimit_sec = 1;// 0.1; 
                dtStart_sec = dtMinLimit_sec;// 0.1;//1 ;
            }
            else
            {
                dtMaxLimit_sec = 300;// 600;
                dtMinLimit_sec = 0.01; 
                dtStart_sec = dtMinLimit_sec;
            }
            convergenceConditionh = 0.00001;// 양해 0.00001;//0.00001; // 0.00000001; //
            convergenceConditionhr = 0.001;// 양해 0.00001;//0.00001; // 0.00000001; //
            convergenceConditionq = 0.0001;//0.0000001;//0.00001; //0.1% 
            iNRmax_forCE = 6;
            iGSmax = 6;
            iGSmax_GPU = 8;
            iNRmax_GPU = 8;

            iNRmax_forME = 6;
            //maxDegreeParallelism = Environment.ProcessorCount * 2;
            dflowmaxInThisStep = -9999;
            vmaxInThisStep = -9999;
            VNConMinInThisStep = double.MaxValue;
        }

        public static void setValues(Dataset.projectds.ProjectSettingsRow row)
        {
            if (row.IsIsFixedDTNull() == false && row.IsFixedDT.ToLower() == "true")
            {
                isfixeddt = 1;
            }
            else
            {
                isfixeddt = -1;
            }

            if (row.IsCalculationTimeStep_secNull() == false && isfixeddt == 1)
            {
                dt_sec = row.CalculationTimeStep_sec;
            }
            else
            {
                dt_sec = dtStart_sec;
            }

            if (row.IsIsParallelNull() == false && row.IsParallel.ToLower() == "true")
            {
                isparallel = 1;
            }
            else
            {
                isparallel = -1;
            }
            maxDegreeParallelism = row.MaxDegreeOfParallelism;
            if (isparallel == 1 && row.IsUsingGPUNull() == false && row.UsingGPU.ToLower() == "true")
            { usingGPU = 1; }
            else
            { usingGPU = -1; }

            if (row.IsMaxIterationAllCellsOnCPUNull() == false)
            {
                iGSmax = row.MaxIterationAllCellsOnCPU;
            }

            if (row.IsMaxIterationACellOnCPUNull() == false)
            {
                iNRmax_forCE = row.MaxIterationACellOnCPU;
            }

            bool consoleWritten = false;
            if (row.IsMaxIterationAllCellsOnGPUNull() == false)
            {
                iGSmax_GPU = row.MaxIterationAllCellsOnGPU;
                if (iGSmax_GPU > 10)
                {
                    Console.WriteLine();
                    writelogNconsole("Maximum iteration value on GPU is 10.", true);
                    iGSmax_GPU = 10;
                    consoleWritten = true;
                }
            }

            if (row.IsMaxIterationACellOnGPUNull() == false)
            {
                iNRmax_GPU = row.MaxIterationACellOnGPU;
                if (iNRmax_GPU > 10)
                {
                    if (consoleWritten == false) { Console.WriteLine(); }
                    writelogNconsole("Maximum iteration value on GPU is 10.", true);
                    iNRmax_GPU = 10;
                }
            }

            if (usingGPU == 1)
            {
                iGSmax = iGSmax_GPU;
                iNRmax_forCE = iNRmax_GPU;
                if (row.IsEffCellThresholdForGPUNull() == false)
                {
                    EffCellThresholdForGPU = row.EffCellThresholdForGPU;
                }
            }

            dt_printout_min = row.PrintoutInterval_min;
            dt_printout_sec = (int)(dt_printout_min * 60);

            if (row.IsStartDateTimeNull() == false)
            {
                eventStartTime = row.StartDateTime;
            }
            else
            {
                eventStartTime = "0";
            }
            isDateTimeFormat = !float.TryParse(eventStartTime, out float output);

            floodingCellDepthThresholds_m = new List<double>();
            if (row.IsFloodingCellDepthThresholds_cmNull() == false && row.FloodingCellDepthThresholds_cm.Trim() != "")
            {
                string[] sepArray = new string[] { "," };
                string[] depths = row.FloodingCellDepthThresholds_cm.Split(sepArray, StringSplitOptions.RemoveEmptyEntries);

                for (int n = 0; n < depths.Length; n++)
                {
                    double v;
                    if (double.TryParse(depths[n], out v) == true)
                    {
                        v = v / 100;//m로 바꾼다.
                        floodingCellDepthThresholds_m.Add(v);
                    }
                }
            }
            else
            {
                floodingCellDepthThresholds_m.Add(0.00001);// 수렴 조건 적용
            }
            cThisProcess.FloodingCellCounts = new List<int>();
            for (int n = 0; n < floodingCellDepthThresholds_m.Count; n++)
            {
                cThisProcess.FloodingCellCounts.Add(0);//0개씩으로 초기화 한다.
            }
            if (row.IsMakeASCFileNull() == false && row.MakeASCFile.ToLower() == "true")
            {
                bMakeASCFile = true;
            }
            else
            {
                bMakeASCFile = false;
            }
            if (row.IsMakeImgFileNull() == false && row.MakeImgFile.ToLower() == "true")
            {
                bMakeImgFile = true;
            }
            else
            {
                bMakeImgFile = false;
            }
            if (row.IsWriteLogNull() == false && row.WriteLog.ToLower() == "true")
            {
                bwritelog_process = true;
            }
            else
            {
                bwritelog_process = false;
            }
        }

        [GpuManaged]
        public static void getGPUDeviceInfo()
        {
            var devices = Device.Devices;
            var numGpus = devices.Length;
            string v = "";
            StringBuilder sbALL = new StringBuilder();
            foreach (var device in devices)
            {
                v = string.Format("  GPU device number (name) : {0} ({1}){2}", numGpus, device.Name, "\r\n");
                sbALL.Append(v);
                v = string.Format("  CUDA core number : {0}{1}", device.Cores, "\r\n");
                sbALL.Append(v);
                v = string.Format("  CUDA capability version : {0}.{1}{2}", device.Arch.Major, device.Arch.Minor, "\r\n");
                sbALL.Append(v);
                v = string.Format("  Max. block dim : ({0}, {1}, {2}){3}", device.Attributes.MaxBlockDimX, device.Attributes.MaxBlockDimY, device.Attributes.MaxBlockDimZ, "\r\n");
                sbALL.Append(v);
                v = string.Format("  Max. grid dim : ({0}, {1}, {2})", device.Attributes.MaxGridDimX, device.Attributes.MaxGridDimY,
                                            device.Attributes.MaxGridDimZ);
                sbALL.Append(v);
                cGenEnv.writelogNconsole(sbALL.ToString(), true);
                //var id = device.Id;
                //var arch = device.Arch;
                //var numMultiProc = device.Attributes.MultiprocessorCount;
            }
        }

        public static void getCPUInfo()
        {
            string v = "";
            StringBuilder sbALL = new StringBuilder();

            foreach (var item in new System.Management.ManagementObjectSearcher("Select * from Win32_Processor").Get())
            {
                v = string.Format("  CPU name : {0} ", item["Name"].ToString());
            }
            sbALL.Append(v + "\r\n");

            foreach (var item in new System.Management.ManagementObjectSearcher("Select * from Win32_ComputerSystem").Get())
            {
                v = string.Format("  Number of physical processors : {0} ", item["NumberOfProcessors"]);
            }
            sbALL.Append(v + "\r\n");

            int coreCount = 0;
            foreach (var item in new System.Management.ManagementObjectSearcher("Select * from Win32_Processor").Get())
            {
                coreCount += int.Parse(item["NumberOfCores"].ToString());
            }
            v= string.Format("  Number of cores : {0}{1}", coreCount, "\r\n");
            sbALL.Append(v);

            v =string.Format("  Number of logical processors : {0}", Environment.ProcessorCount);
            sbALL.Append(v);
            cGenEnv.writelogNconsole(sbALL.ToString(), true);
        }

        public static void writelog(string logtxt, bool writeLog)
        {
            if (writeLog == true)
            {
                File.AppendAllText(cGenEnv.fpnlog,
                string.Format("{0:yyyy-MM-dd HH:mm}, ", DateTime.Now)
                + logtxt + "\r\n");
            }
        }

        public static void writelogNconsole(string logtxt, bool writeLog)
        {
            Console.WriteLine(logtxt);
            if (writeLog == true)
            {
                File.AppendAllText(cGenEnv.fpnlog,
                string.Format("{0:yyyy-MM-dd HH:mm}, ", DateTime.Now)
                + logtxt + "\r\n");
            }
        }
    }
}
