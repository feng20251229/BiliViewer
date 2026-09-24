using System;
using System.IO; 
using System.Windows.Forms;

namespace BiliAnalyzer
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            // 把工作目录设为 exe 所在目录
            Directory.SetCurrentDirectory(AppContext.BaseDirectory);
            
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}