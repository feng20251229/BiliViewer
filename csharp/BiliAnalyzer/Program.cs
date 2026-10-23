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
            // 自动生成存储历史记录和检索数据的文件夹
            DataInitializer.EnsureDataFiles();
        
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}