using System;
using System.IO;
using System.Text.Json;

namespace BiliAnalyzer
{
    public static class DataInitializer
    {
        public static void EnsureDataFiles()
        {
            string dataDir = Path.Combine(AppContext.BaseDirectory, "data");
            string historyFile = Path.Combine(dataDir, "biliHistory");
            string configFile = Path.Combine(dataDir, "config.json");

            if (!Directory.Exists(dataDir)) Directory.CreateDirectory(dataDir);
            if (!File.Exists(historyFile)) File.WriteAllText(historyFile, "");
            if (!File.Exists(configFile))
            {
                var config = new
                {
                    api_url = "https://api.bilibili.com/x/web-interface/history/cursor?max=0&view_at=0&business=&ps=20&type=all&web_location=333.1391",
                    cookie = "",
                    timeout = 15,
                    verify_ssl = false
                };
                
                string json = JsonSerializer.Serialize(config, new JsonSerializerOptions{WriteIndented = true});
                File.WriteAllText(configFile, json);

            }

        }
    }
    
}
