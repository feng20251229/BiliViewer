using System;
using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace BiliAnalyzer
{
    public static class CookieManager
    {
        private static string ConfigPath =>
            Path.Combine(AppContext.BaseDirectory, "data", "config.json");

        private static string RememberPath =>
            Path.Combine(AppContext.BaseDirectory, "data", "remember.json");

        public static string? LoadCookie()
        {
            try
            {
                if (!File.Exists(ConfigPath)) return null;

                string json = File.ReadAllText(ConfigPath);
                using var doc = JsonDocument.Parse(json);

                if (doc.RootElement.TryGetProperty("cookie", out var c))
                {
                    string? cookie = c.GetString();
                    if (!string.IsNullOrWhiteSpace(cookie))
                        return cookie;
                }
            }
            catch { }
            return null;
        }

        /// <summary>
        /// 只更新 config.json 里的 cookie 字段，其它字段保留
        /// </summary>
        public static void SaveCookie(string cookie)
        {
            try
            {
                if (!File.Exists(ConfigPath))
                {
                    // 文件不存在，创建默认的
                    var defaultConfig = new
                    {
                        api_url = "https://api.bilibili.com/x/web-interface/history/cursor?max=0&view_at=0&business=&ps=20&type=all&web_location=333.1391",
                        cookie = cookie,
                        timeout = 15,
                        verify_ssl = false
                    };
                    string json = JsonSerializer.Serialize(defaultConfig, new JsonSerializerOptions
                    {
                        WriteIndented = true
                    });
                    File.WriteAllText(ConfigPath, json);
                    return;
                }

                // 文件存在，只改 cookie 字段
                string oldJson = File.ReadAllText(ConfigPath);
                var node = JsonNode.Parse(oldJson);
                if (node == null) return;

                node["cookie"] = cookie;

                string newJson = node.ToJsonString(new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                File.WriteAllText(ConfigPath, newJson);
            }
            catch { }
        }

        public static bool IsRemembered()
        {
            try
            {
                if (!File.Exists(RememberPath)) return false;
                string json = File.ReadAllText(RememberPath);
                using var doc = JsonDocument.Parse(json);

                if (doc.RootElement.TryGetProperty("remember", out var r))
                    return r.GetBoolean();
            }
            catch { }
            return false;
        }

        public static void SetRemembered(bool remember)
        {
            var obj = new { remember = remember };
            string json = JsonSerializer.Serialize(obj);
            File.WriteAllText(RememberPath, json);
        }

        public static void ClearCookie()
        {
            try
            {
                if (File.Exists(ConfigPath))
                {
                    // 只把 cookie 字段清空，保留其它字段
                    string json = File.ReadAllText(ConfigPath);
                    var node = JsonNode.Parse(json);
                    if (node != null)
                    {
                        node["cookie"] = "";
                        File.WriteAllText(ConfigPath, node.ToJsonString(new JsonSerializerOptions
                        {
                            WriteIndented = true
                        }));
                    }
                }

                if (File.Exists(RememberPath)) File.Delete(RememberPath);
            }
            catch { }
        }
    }
}