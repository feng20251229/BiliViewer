using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.Web.WebView2.WinForms;

namespace BiliAnalyzer
{
    public class LoginForm : Form
    {
        private WebView2 webView;
        private CheckBox chkRemember;
        private Label lblStatus;
        private Button btnOk;

        /// <summary>
        /// 登录成功后拿到的 Cookie（没有则是 null）
        /// </summary>
        public string? ResultCookie { get; private set; }

        /// <summary>
        /// 是否勾选"记住我"
        /// </summary>
        public bool RememberMe => chkRemember.Checked;

        public LoginForm()
        {
            this.Text = "登录 B 站";
            this.Size = new Size(900, 700);
            this.StartPosition = FormStartPosition.CenterScreen;

            // 顶部提示
            var lblTip = new Label
            {
                Text = "请在下方登录 B 站，登录成功后会自动抓取 Cookie",
                Dock = DockStyle.Top,
                Height = 30,
                TextAlign = ContentAlignment.MiddleCenter,
                BackColor = Color.LightYellow
            };

            // 底部面板
            var panelBottom = new Panel
            {
                Dock = DockStyle.Bottom,
                Height = 50
            };

            chkRemember = new CheckBox
            {
                Text = "记住我（下次自动登录）",
                Left = 10,
                Top = 15,
                Width = 220,
                Checked = true
            };

            btnOk = new Button
            {
                Text = "完成登录",
                Left = 240,
                Top = 10,
                Width = 100,
                Height = 30
            };
            btnOk.Click += BtnOk_Click;

            lblStatus = new Label
            {
                Text = "等待登录...",
                Left = 360,
                Top = 18,
                Width = 500,
                AutoSize = false,
                ForeColor = Color.Gray
            };

            panelBottom.Controls.AddRange(new Control[] {
                chkRemember, btnOk, lblStatus
            });

            // WebView2
            webView = new WebView2
            {
                Dock = DockStyle.Fill
            };

            // 加控件的顺序很重要（Dock 布局）
            this.Controls.Add(webView);
            this.Controls.Add(panelBottom);
            this.Controls.Add(lblTip);

            this.Load += LoginForm_Load;
        }

        private async void LoginForm_Load(object? sender, EventArgs e)
{
    await webView.EnsureCoreWebView2Async();

    // 导航完成时检查 Cookie
    webView.CoreWebView2.NavigationCompleted += (s, args) =>
    {
        CheckCookie();
    };

    // 加载 B 站登录页
    webView.Source = new Uri("https://passport.bilibili.com/login");
}

        /// <summary>
        /// 检查有没有登录成功的 Cookie
        /// </summary>
        private async void CheckCookie()
        {
            try
            {
                var cookies = await webView.CoreWebView2.CookieManager
                    .GetCookiesAsync("https://www.bilibili.com");

                string? sessdata = null;
                string? biliJct = null;
                string? dedeUserId = null;

                foreach (var c in cookies)
                {
                    if (c.Name == "SESSDATA") sessdata = c.Value;
                    if (c.Name == "bili_jct") biliJct = c.Value;
                    if (c.Name == "DedeUserID") dedeUserId = c.Value;
                }

                if (!string.IsNullOrEmpty(sessdata))
                {
                    // 拼成 Cookie 字符串
                    string cookieStr = $"SESSDATA={sessdata}";
                    if (!string.IsNullOrEmpty(biliJct))
                        cookieStr += $"; bili_jct={biliJct}";
                    if (!string.IsNullOrEmpty(dedeUserId))
                        cookieStr += $"; DedeUserID={dedeUserId}";

                    ResultCookie = cookieStr;

                    this.Invoke(new Action(() =>
                    {
                        lblStatus.Text = "✅ 已检测到登录，点\"完成登录\"保存";
                        lblStatus.ForeColor = Color.Green;
                    }));
                }
            }
            catch { }
        }

        private void BtnOk_Click(object? sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(ResultCookie))
            {
                MessageBox.Show("还没检测到登录，请先在页面里登录 B 站");
                return;
            }

            this.DialogResult = DialogResult.OK;
            this.Close();
        }
    }
}