using System;
using System.Drawing;
using System.Windows.Forms;

namespace BiliAnalyzer
{
    public class MainForm : Form
    {
        // ---- 控件 ----
        private Button btnLoadWeb = null!;
        private Button btnLoadFile = null!;
        private DateTimePicker dtStart = null!;
        private DateTimePicker dtEnd = null!;
        private Button btnFilter = null!;
        private Button btnTime = null!;
        private Button btnRate = null!;
        private Button btnWeek = null!;
        private DataGridView grid = null!;
        private TextBox txtOutput = null!;

        public MainForm()
        {
            this.Text = "B 站历史分析";
            this.Size = new Size(960, 640);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.Font = new Font("微软雅黑", 9F);

            BuildUI();
        }

        // ============================================================
        // 界面布局
        // ============================================================
        private void BuildUI()
        {
            // ---- 顶部按钮 ----
            btnLoadWeb = MakeButton("抓取网络", 10, 12, 100, BtnLoadWeb_Click);
            btnLoadFile = MakeButton("读取本地", 118, 12, 100, BtnLoadFile_Click);

            var lblStart = new Label { Text = "开始:", Left = 240, Top = 18, AutoSize = true };
            dtStart = new DateTimePicker
            {
                Left = 280,
                Top = 14,
                Width = 130,
                Format = DateTimePickerFormat.Short,
                Value = DateTime.Today.AddDays(-6)
            };

            var lblEnd = new Label { Text = "结束:", Left = 420, Top = 18, AutoSize = true };
            dtEnd = new DateTimePicker
            {
                Left = 460,
                Top = 14,
                Width = 130,
                Format = DateTimePickerFormat.Short,
                Value = DateTime.Today
            };

            btnFilter = MakeButton("按日期筛选", 610, 12, 110, BtnFilter_Click);
            btnTime = MakeButton("算时长", 730, 12, 90, BtnTime_Click);
            btnRate = MakeButton("完成率", 830, 12, 90, BtnRate_Click);

            // ---- 数据表格 ----
            grid = new DataGridView
            {
                Left = 10,
                Top = 55,
                Width = 925,
                Height = 400,
                ReadOnly = true,
                AllowUserToAddRows = false,
                AllowUserToResizeRows = false,
                AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill,
                SelectionMode = DataGridViewSelectionMode.FullRowSelect,
                RowHeadersVisible = false,
                BackgroundColor = Color.White
            };
            grid.Columns.Add("time", "观看时间");
            grid.Columns.Add("title", "标题");
            grid.Columns.Add("tag", "分区");
            grid.Columns.Add("business", "类型");
            grid.Columns.Add("bvid", "BVID");
            grid.Columns.Add("progress", "已看(秒)");
            grid.Columns.Add("duration", "总长(秒)");
            grid.Columns["time"]!.Width = 130;
            grid.Columns["title"]!.Width = 320;
            grid.Columns["tag"]!.Width = 100;
            grid.Columns["business"]!.Width = 70;
            grid.Columns["bvid"]!.Width = 110;
            grid.Columns["progress"]!.Width = 70;
            grid.Columns["duration"]!.Width = 70;

            // ---- 底部输出 ----
            txtOutput = new TextBox
            {
                Left = 10,
                Top = 465,
                Width = 925,
                Height = 120,
                Multiline = true,
                ReadOnly = true,
                ScrollBars = ScrollBars.Vertical,
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.LightGreen,
                Font = new Font("Consolas", 9.5F)
            };

            this.Controls.AddRange(new Control[] {
                btnLoadWeb, btnLoadFile, lblStart, dtStart, lblEnd, dtEnd,
                btnFilter, btnTime, btnRate, grid, txtOutput
            });

            // 关闭时释放 C++ 资源
            this.FormClosing += (s, e) =>
            {
                try { BiliNative.Bili_Shutdown(); } catch { }
            };


        }

        private Button MakeButton(string text, int left, int top, int width, EventHandler handler)
        {
            var b = new Button
            {
                Text = text,
                Left = left,
                Top = top,
                Width = width,
                Height = 30
            };
            b.Click += handler;
            return b;
        }

        // ============================================================
        // 按钮事件
        // ============================================================

        // 从网络抓取
        private void BtnLoadWeb_Click(object? sender, EventArgs e)
        {
            // 1. 读已保存的 Cookie
            string? cookie = CookieManager.LoadCookie();
            bool remembered = CookieManager.IsRemembered();

            // 2. 没有 Cookie，或用户没勾"记住我"，弹登录
            if (string.IsNullOrEmpty(cookie) || !remembered)
            {
                using var loginForm = new LoginForm();
                if (loginForm.ShowDialog() != DialogResult.OK)
                {
                    AppendOutput("用户取消登录");
                    return;
                }

                cookie = loginForm.ResultCookie;
                if (string.IsNullOrEmpty(cookie))
                {
                    AppendOutput("登录失败，没拿到 Cookie");
                    return;
                }

                // 3. 保存 Cookie 和"记住我"
                CookieManager.SaveCookie(cookie);
                CookieManager.SetRemembered(loginForm.RememberMe);

                AppendOutput("登录成功，Cookie 已保存");
            }
            else
            {
                AppendOutput("使用已保存的 Cookie");
            }

            // 4. 开始抓取
            LoadData(1);
        }

        // 从本地文件读
        private void BtnLoadFile_Click(object? sender, EventArgs e)
        {
            LoadData(0);
        }

        private void LoadData(int fromWeb)
        {
            SetBusy(true, fromWeb == 1 ? "正在抓取网络数据..." : "正在读取本地文件...");

            System.Threading.Tasks.Task.Run(() =>
            {
                try
                {
                    Log("1. 准备调用 Bili_Init");
                    int count = BiliNative.Bili_Init(fromWeb);
                    Log($"2. Bili_Init 返回，共 {count} 条");

                    this.Invoke(new Action(() =>
                    {
                        Log("3. 开始 ShowRecords");
                        ShowRecords();
                        Log("4. ShowRecords 完成");
                        SetBusy(false, null);
                        Log("5. 完成");
                    }));
                }
                catch (Exception ex)
                {
                    this.Invoke(new Action(() =>
                    {
                        Log("错误: " + ex.Message);
                        SetBusy(false, null);
                    }));
                }
            });
        }

        // 线程安全的日志
        private void Log(string msg)
        {
            if (txtOutput.InvokeRequired)
            {
                txtOutput.Invoke(new Action(() => Log(msg)));
                return;
            }
            AppendOutput(msg);
        }

        // 按日期筛选
        private void BtnFilter_Click(object? sender, EventArgs e)
        {
            if (BiliNative.Bili_Count() == 0)
            {
                MessageBox.Show("请先加载数据");
                return;
            }

            string s = dtStart.Value.ToString("yyyy-MM-dd");
            string t = dtEnd.Value.ToString("yyyy-MM-dd");

            grid.Rows.Clear();
            int count = BiliNative.Bili_Count();
            int shown = 0;

            for (int i = 0; i < count; i++)
            {
                long ts = BiliNative.Bili_ViewAt(i);
                string date = BiliNative.FormatTime(ts).Substring(0, 10);
                if (string.Compare(date, s) >= 0 && string.Compare(date, t) <= 0)
                {
                    AddRow(i);
                    shown++;
                }
            }

            AppendOutput($"筛选 {s} ~ {t}，共 {shown} 条");
        }

        // 算时长
        private void BtnTime_Click(object? sender, EventArgs e)
        {
            if (BiliNative.Bili_Count() == 0) { MessageBox.Show("请先加载数据"); return; }

            string s = dtStart.Value.ToString("yyyy-MM-dd");
            string t = dtEnd.Value.ToString("yyyy-MM-dd");

            int sec = BiliNative.Bili_TimeSeconds(s, t);
            AppendOutput($"{s} ~ {t} 观看时长: {sec / 3600}小时 {sec % 3600 / 60}分钟 {sec % 60}秒");
        }

        // 完成率
        private void BtnRate_Click(object? sender, EventArgs e)
        {
            if (BiliNative.Bili_Count() == 0) { MessageBox.Show("请先加载数据"); return; }

            double rate = BiliNative.Bili_CompletionRate();
            AppendOutput($"整体平均完成率: {rate:F2}%");
        }

        // ============================================================
        // 辅助
        // ============================================================

        private void ShowRecords()
        {
            grid.SuspendLayout();
            try
            {
                grid.Rows.Clear();
                int count = BiliNative.Bili_Count();
                for (int i = 0; i < count; i++)
                    AddRow(i);
            }
            finally
            {
                grid.ResumeLayout();
            }
        }

        private void AddRow(int i)
        {
            grid.Rows.Add(
                BiliNative.FormatTime(BiliNative.Bili_ViewAt(i)),
                BiliNative.GetTitle(i),
                BiliNative.GetTag(i),
                BiliNative.GetBusiness(i),
                BiliNative.GetBvid(i),
                BiliNative.Bili_Progress(i),
                BiliNative.Bili_Duration(i)
            );
        }

        private void AppendOutput(string line)
        {
            txtOutput.AppendText($"[{DateTime.Now:HH:mm:ss}] {line}\r\n");
        }

        private void SetBusy(bool busy, string? msg)
        {
            btnLoadWeb.Enabled = !busy;
            btnLoadFile.Enabled = !busy;
            btnFilter.Enabled = !busy;
            btnTime.Enabled = !busy;
            btnRate.Enabled = !busy;
            if (msg != null) AppendOutput(msg);
        }

        private void TestLogin()
        {
            using var loginForm = new LoginForm();
            if (loginForm.ShowDialog() == DialogResult.OK)
            {
                MessageBox.Show($"Cookie: {loginForm.ResultCookie}");
                MessageBox.Show($"记住我: {loginForm.RememberMe}");
            }
            else
            {
                MessageBox.Show("用户取消");
            }
        }
    }
}