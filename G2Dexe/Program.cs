using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using G2DCore;

namespace G2Dexe
{
   public class Program
    {
       public cSimulatorCPUnGPU simulatorCPUnGPU = new cSimulatorCPUnGPU();
       public double simDur_min = 0;
       static void Main(string[] args)
        {
            Program MainRun = new Program();
            MainRun.RunG2D(args);
        }

        public void RunG2D(string[] args)
        {
            System.Reflection.Assembly aExe = System.Reflection.Assembly.GetExecutingAssembly();
            string fp = Path.GetDirectoryName(aExe.Location);
            string fpnCore = Path.Combine(fp, "G2DCore.dll");
            string fpnGentle = Path.Combine(fp, "gentle.dll");
            System.Reflection.Assembly aCore = System.Reflection.Assembly.LoadFrom(fpnCore) ;
            FileVersionInfo fviGentle = FileVersionInfo.GetVersionInfo(fpnGentle);
            System.Reflection.AssemblyName anExe = aExe.GetName();
            System.Reflection.AssemblyName anCore = aCore.GetName();
            string fvExe = anExe.Version.Major.ToString() + "."
                + anExe.Version.Minor.ToString() + "." + anExe.Version.Build.ToString();
            string fvCore = anCore.Version.Major.ToString() + "."
                + anCore.Version.Minor.ToString() + "." + anCore.Version.Build.ToString();
            string fvGentle = fviGentle.FileVersion.ToString();
            FileInfo fiExe = new FileInfo(aExe.Location);
            FileInfo fiCore = new FileInfo(fpnCore);
            FileInfo fiGentle = new FileInfo(fpnGentle);
            string fileInfoLogExe = string.Format("{0} v{1}. Built in {2}", anExe.Name, fvExe, fiExe.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
            string fileInfoLogCore = string.Format("{0} v{1}. Built in {2}", anCore.Name, fvCore, fiCore.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
            string fileInfoLogGentle = string.Format("{0} v{1}. Built in {2}", fviGentle.ProductName.ToString(), fvGentle, fiGentle.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
            Console.Write(fileInfoLogExe+"\r\n");
            Console.Write(fileInfoLogCore + "\r\n");
            Console.Write(fileInfoLogGentle + "\r\n");

            if (args.Length == 0)
            {
                Console.Write("G2D project file was not entered.");
                g2dHelp();
                return;
            }
            if (args[0] == "/?")
            {
                g2dHelp();
                return;
            }
            string prjfpn = args[0];
            if (Path.GetDirectoryName(prjfpn) == "")
            {
                prjfpn = Path.Combine(Path.GetDirectoryName(aExe.Location), prjfpn);
            }
            G2DCore.cGenEnv.fpnlog = prjfpn.Replace(".g2p", ".log");
          
            if (File.Exists(G2DCore.cGenEnv.fpnlog) == true) { File.Delete(G2DCore.cGenEnv.fpnlog); }
            cGenEnv.writelog(fileInfoLogExe, true);
            cGenEnv.writelog(fileInfoLogCore, true);
            cGenEnv.writelog(fileInfoLogGentle, true);

            Stopwatch sw = new Stopwatch();
            sw.Start();
            G2DCore.cGenEnv.simulationStartTime = DateTime.Now;
            cGenEnv.writelog("G2D was started.", true);
            if (cProject.OpenProject(prjfpn) == false)
            {
                cGenEnv.writelog(string.Format("An error was occurred while opening project file."), G2DCore.cGenEnv.bwritelog_error);
                Console.Write("Press any key...");
                Console.ReadKey();
                goto theEND;
            }
            cGenEnv.writelog(prjfpn + " project was opened.", true);
            cProject prj = cProject.Current;
            simDur_min = prj.simDur_hr * 60;
            simulatorCPUnGPU.SimulationStep += new cSimulatorCPUnGPU.SimulationStepEventHandler(simulationStep);
            simulatorCPUnGPU.SimulationComplete += new cSimulatorCPUnGPU.SimulationCompleteEventHandler(simulationCompleted);
            cGenEnv.writelogNconsole(string.Format("{0} -> Model setup was completed", prjfpn), true);
            if (G2DCore.cGenEnv.isparallel == 1)
            {
                string usingGPU = "false";
                if (G2DCore.cGenEnv.usingGPU == 1) { usingGPU = "true"; }
                cGenEnv.writelogNconsole(string.Format("Parallel : true. Max. degree of parallelism : {0}. Using GPU : {1}",
                    G2DCore.cGenEnv.maxDegreeParallelism.ToString(), usingGPU), true);
            }
            else
            {
                cGenEnv.writelogNconsole(string.Format("Parallel : false. Using GPU : false"), true);
            }
            if (G2DCore.cGenEnv.isparallel == 1)
            {
                G2DCore.cGenEnv.getCPUInfo();
            }
            if (cGenEnv.usingGPU == 1)
            {
                G2DCore.cGenEnv.getGPUDeviceInfo();
                cGenEnv.writelog(string.Format("Effective cells number threshold to convert to GPU calculation : {0}", G2DCore.cGenEnv.EffCellThresholdForGPU.ToString()), true);
            }
            cGenEnv.writelog(string.Format("iGS(all cells) max : {0}, iNR(a cell) max : {1}, tolerance : {2}",
                G2DCore.cGenEnv.iGSmax.ToString(), G2DCore.cGenEnv.iNRmax_forCE.ToString(), G2DCore.cGenEnv.convergenceConditionh.ToString()), true);
            cGenEnv.writelog(string.Format("Calculation using CPU was started."), true);

            prj.output.deleteAlloutputFilesExisted();
            if (simulatorCPUnGPU.simulationControlUsingCPUnGPU(prj) == false) // 이것을 이용하는 것으로 전환. 2018.08.10.
            {
                cGenEnv.writelog(string.Format("An error was occurred while simulation."), cGenEnv.bwritelog_error);
                goto theEND;
            }
            cGenEnv.bwritelog_process = true;
            TimeSpan ts = sw.Elapsed;
            sw.Stop();
            cGenEnv.writelog(string.Format("Simulation was completed. Run time : {0}hrs {1}min {2}sec.",
                ts.Hours, ts.Minutes, ts.Seconds), true);
            //Console.ReadKey();
            if (cGenEnv.vdtest == true)
            {
                System.Diagnostics.Process.Start("notepad.exe", Path.Combine(Path.GetDirectoryName(prjfpn), "00_Summary_test.out")); //'실행파일 뒤에 빈칸 하나 필수..
            }
        theEND:;
        }

        public static void g2dHelp()
        {
            Console.WriteLine();
            Console.WriteLine(" Usage : g2d.exe [The full path and name of the current project file to simulate]");
            Console.WriteLine();
            Console.WriteLine("** 사용법 (in Korean)");
            Console.WriteLine("  1. G2D 모형의 입력자료(지형공간자료, 경계조건, 강우 등)를 준비하고,");
            Console.WriteLine("     project 파일(.g2p)을 작성한다.");
            Console.WriteLine("  2. 모델링 대상 프로젝트 이름을 argument로 하여 실행한다.");
            Console.WriteLine("     - Console에서 [G2D.exe argument] 로 실행한다.");
            Console.WriteLine("     ** 주의사항 : 장기간 모의할 경우, 컴퓨터 업데이트로 인해, 종료될 수 있으니, ");
            Console.WriteLine("                       네트워크를 차단하고, 자동업데이트 하지 않음으로 설정한다.");
            Console.WriteLine("  3. argument");
            Console.WriteLine("      - /?");
            Console.WriteLine("          도움말");
            Console.WriteLine("      - 프로젝트 파일경로와 이름");
            Console.WriteLine("        G2D 모델을 파일단위로 실행시킨다.");
            Console.WriteLine("        이때 full path, name을 넣어야 하지만, ");
            Console.WriteLine("        대상 프로젝트 파일이 G2D.exe 파일과 동일한 폴더에 있을 경우에는,");
            Console.WriteLine("                 path는 입력하지 않아도 된다.");
            Console.WriteLine("         대상 프로젝트 이름과 경로에 공백이 포함될 경우 큰따옴표로 묶어서 입력한다.");
            Console.WriteLine("          ** 예문(G2D.exe가 d://G2Drun에 있을 경우)");
            Console.WriteLine("              - Case 1. G2D.exe와 다른 폴더에 프로젝트 파일이 있을 경우");
            Console.WriteLine("                d://G2Drun>G2D.exe D://G2DTest//TestProject//test.g2p");
            Console.WriteLine("              - Case 2. G2D.exe와 같은 폴더에 프로젝트 파일이 있을 경우");
            Console.WriteLine("                d://G2Drun>G2D.exe test.gmp");
            Console.WriteLine();
            Console.WriteLine("** land cover vat file " );
            Console.WriteLine("   - the first value is grid value, the second is land cover name,");
            Console.WriteLine("     and the third is roughness coefficient.");
            Console.WriteLine(" ");
            Console.WriteLine(" ");
            Console.WriteLine("** Usage (in English)");
            Console.WriteLine("  1. Prepare input data (geospatial data, boundary condition, rainfall, etc.) of G2D model");
            Console.WriteLine("     and create a project file (.g2p).");
            Console.WriteLine("  2. Run the G2D model using the project file (.g2p) as an argument.");
            Console.WriteLine("     - Run [G2D.exe  argument] in the console window.");
            Console.WriteLine("     ** NOTICE: If you simulate for a long time, it may be shut down due to computer update.");
            Console.WriteLine("           So, it is safe to disconnect the network and set the computer not to update automatically.");
            Console.WriteLine("  3. argument");
            Console.WriteLine("      - /?");
            Console.WriteLine("          Help");
            Console.WriteLine("      - Project file path and name");
            Console.WriteLine("        Run the G2D model on a file-by-file basis..");
            Console.WriteLine("        At this time, full path and name should be used, ");
            Console.WriteLine("            but if the target project file is in the same folder as G2D.exe file,");
            Console.WriteLine("            you do not need to input the file path.");
            Console.WriteLine("        If the target project name and path contain spaces, enclose them in double quotes.");
            Console.WriteLine("          ** Examples (when G2D.exe is in d://G2Drun)");
            Console.WriteLine("              - Case 1. The project file is in a folder other than G2D.exe");
            Console.WriteLine("                d://G2Drun>g2d.exe D://G2DTest//TestProject//test.g2p");
            Console.WriteLine("              - Case 2. The project file is in the same folder as G2D.exe");
            Console.WriteLine("                d://G2Drun>g2d.exe test.gmp");
            Console.WriteLine();
            Console.WriteLine("** land cover vat file ");
            Console.WriteLine("   - the first value is grid value, the second is land cover name,");
            Console.WriteLine("     and the third is roughness coefficient.");
        }

        public void simulationStep(double step_min)
        {
            int progressRatio = (int)(step_min / (simDur_min) * 100);
            Console.Write(string.Format("{3}Current progress[min]: {0}/{1}[{2}%]..", (int)step_min, this.simDur_min, progressRatio, "\r"));
        }

        public void simulationCompleted()
        {
            Console.WriteLine(string.Format("Simulation was completed."));
        }
    }
}
