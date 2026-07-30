#include "connect.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cjson/cjson.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

std::unordered_set<std::string> seenKeys;

// 用于存储API响应数据的结构
struct MemoryStruct {
    char *memory;
    size_t size;
};

// 读取整个文件
static std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static void saveRecordsToFile(const std::vector<BiliRecord> &records, const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (const auto& rec : records) {
        std::string safeTitle = rec.title;
        std::replace(safeTitle.begin(), safeTitle.end(), '|', ';');
        std::string safeTag = rec.tag;
        std::replace(safeTag.begin(), safeTag.end(), '|', ';');    
        file << rec.view_timestamp << "|"
             << safeTitle << "|"
             << (rec.tag.empty() ? "未分类" : safeTag) << "|"
             << (rec.bvid.empty() ? "N/A" : rec.bvid) << "|"
             << rec.business << "|"
             << rec.progress << "|"
             << rec.duration << "\n";
    }
}

// 从API获取数据的回调函数, 追加到MemoryStruct缓冲区
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    auto *mem = static_cast<MemoryStruct*>(userp); // C++风格转换

    char *ptr = static_cast<char*>(realloc(mem->memory, mem->size + realsize+ 1));
    
    if (ptr == nullptr) {
        std::cerr << "Not enough memory (realloc returned NULL)" << std::endl;
        return 0;
    }
    
    mem->memory = ptr;
    std::memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

// 获取网页地址
static std::string getURL(long max, long view_at) {
    std::string base_url = "https://api.bilibili.com/x/web-interface/history/cursor";
    return base_url + "?max=" + std::to_string(max) + "&view_at=" + std::to_string(view_at) + "&ps=20";
}

static int getBiliRecord(long &max, long &view_at, const std::string& jsonData, std::vector<BiliRecord>& records)
{
    cJSON *root = cJSON_Parse(jsonData.c_str());
    if (root == nullptr) {
        std::cerr << "JSON解析失败" << std::endl;
        return -1;
    } 

    cJSON *data = cJSON_GetObjectItem(root, "data");
    // 获取max, view_at
    cJSON *cursor = cJSON_GetObjectItem(data, "cursor");
    cJSON *bmax = cJSON_GetObjectItem(cursor, "max");
    if (cJSON_IsNumber(bmax)) {
        max = bmax->valueint;
    }
    cJSON *bview_at = cJSON_GetObjectItem(cursor, "view_at");
    if (cJSON_IsNumber(bview_at)) {
        view_at = bview_at->valueint;
    }
    // 获取数据
    cJSON *list = cJSON_GetObjectItem(data, "list");
    int count = cJSON_GetArraySize(list);
    if (count == 0) return 0;

    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(list, i);
        BiliRecord record;
        // 提取标题
        cJSON *title = cJSON_GetObjectItem(item, "title");
        if (cJSON_IsString(title) && title->valuestring  != nullptr) {
                record.title = title->valuestring;
        }
        
        // 提取嵌套history
        cJSON *history = cJSON_GetObjectItem(item, "history");
        if (history) {
            // 提取视频ID
            cJSON *bvid = cJSON_GetObjectItem(history, "bvid");
            if (cJSON_IsString(bvid) && bvid->valuestring != nullptr) {
                record.bvid = bvid->valuestring;
            }
            // 提取分区
            cJSON *business = cJSON_GetObjectItem(history, "business");
            if (cJSON_IsString(business) && business->valuestring != nullptr) {
                record.business = business->valuestring;
            }
        }
            
        // 提取观看时间戳
        cJSON *view_at = cJSON_GetObjectItem(item, "view_at");
        if (cJSON_IsNumber(view_at)) {
            record.view_timestamp = view_at->valueint;
        }
        
        std::string key = record.bvid + "|" + std::to_string(record.view_timestamp);
        if (seenKeys.find(key) != seenKeys.end()) {
            cJSON_Delete(root);
            return 1;
        }
        seenKeys.insert(key);
        

        // 提取停留时长
        cJSON *progress = cJSON_GetObjectItem(item, "progress");
        if (cJSON_IsNumber(progress)) {
            record.progress = progress->valueint;
        }

        // 提取总时长
        cJSON *duration = cJSON_GetObjectItem(item, "duration");
        if (cJSON_IsNumber(duration)) {
            record.duration = duration->valueint;
        }

        // 提取分区标签
        cJSON *tag = cJSON_GetObjectItem(item, "tag_name");
        if (cJSON_IsString(tag) && tag->valuestring != nullptr) {
            record.tag = tag->valuestring;
        }

        records.push_back(record);
    }
    
    return 0;
}

// 网络请求
static int fetchBiliPage(long max, long view_at, const std::string& cookie, std::string& jsonData, CURL *curl_handle)
{
    struct MemoryStruct chunk = {nullptr, 0};
    int ret = -1;

    if (curl_handle) {
        std::string url = getURL(max, view_at);

        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl_handle);
        if (res == CURLE_OK && chunk.memory != nullptr) {
            jsonData = chunk.memory;
            ret = 0;
        } else {
            std::cerr << "请求失败：" << curl_easy_strerror(res) << std::endl;
        }
    }
    free(chunk.memory);
    return ret;
}

/**
 * 获取B站历史记录
 * @return 0 成功, -1 失败
 */
int getBiliHistory(std::vector<BiliRecord>& records)
{
    long max = 0, view_at = 0;
    long prev_max = -1, prev_view_at = -1;
    int pageCount = 0;
    std::string jsonData;
    CURL *curl_handle = nullptr;
    curl_handle = curl_easy_init();
    std::string filename = ".gitignore/biliHistory";

    std::string configContent = readFileToString(".gitignore/config.json");
    cJSON *configRoot = cJSON_Parse(configContent.c_str());
    if (configRoot == nullptr) {
        std::cerr << "解析配置文件失败" << std::endl;
        return -1;
    }
    std::string bili_cookie = cJSON_GetObjectItem(configRoot, "cookie")->valuestring;
    cJSON_Delete(configRoot); 

    // 初始化libcurl全局环境
    curl_global_init(CURL_GLOBAL_ALL);

    while (true)
    {
        if (fetchBiliPage(max, view_at, bili_cookie, jsonData, curl_handle) != 0) {
            std::cerr << "获取数据错误" << std::endl;
        }

        int rec = getBiliRecord(max, view_at, jsonData, records);
        if ( rec == -1) {
            std::cerr << "解析数据错误" << std::endl;
        } else if (rec == 1) {
            break;
        }

        if (max == prev_max && view_at == prev_view_at) {
            std::cout << "已获取全部历史记录" << std::endl;
            break;
        }

        pageCount++;
        int pos = (pageCount % 1000) * 50 / 1000;
        std::cout << "\r["; // 覆盖
        for (int i = 0; i < 50; i++) {
            std::cout << (i < pos ? "=" : (i == pos ? ">" : " "));
        }
        std::cout << "] " << pageCount << " 页" << std::flush; // 强制输出缓冲区

        prev_max = max;
        prev_view_at = view_at;

        if (pageCount == 1000) break;

    }

    std::cout << "\r" << "====== complete ======" << " " << pageCount << "页";

    saveRecordsToFile(records, filename);
    curl_easy_cleanup(curl_handle);
    // 清理 libcurl 全局环境
    curl_global_cleanup();

    return 0;
}


void loadRecordsFromFile(const std::string& filename, std::vector<BiliRecord>& records) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    records.clear();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        BiliRecord rec;
        size_t pos = 0, next;

        next = line.find('|'); rec.view_timestamp = std::stol(line.substr(pos, next - pos));
        pos = next + 1;
        next = line.find('|', pos); rec.title = line.substr(pos, next - pos);
        pos = next + 1;

        next = line.find('|', pos); rec.tag = line.substr(pos, next - pos);
        pos = next + 1;
        next = line.find('|', pos); rec.bvid = line.substr(pos, next - pos);
        pos = next + 1;
        next = line.find('|', pos); rec.business = line.substr(pos, next - pos);
        pos = next + 1;
        next = line.find('|', pos); rec.progress = std::stoi(line.substr(pos, next - pos));
        pos = next + 1;
        rec.duration = std::stoi(line.substr(pos));


        records.push_back(rec);
    }
}
