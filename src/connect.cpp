#include "connect.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cjson/cjson.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_set>

Gethist::Gethist(readWay a){
    if (a == Web) {
        int getInt = Gethist::getBiliHistory();
        if (getInt == -1) {
            std::cout << "构造函数获取网络数据失败" << std::endl;
        }
    } else if (a == Data) {
        loadRecordsFromFile();
    } else {
        std::cout << "构造输入错误" << std::endl;
    }
}

Gethist::~Gethist() { 
    if (!saveOnDestruct) return;
    // 排序历史记录
    std::sort(this->records.begin(), this->records.end(),
        [](const BiliRecord& a, const BiliRecord& b) {
            return a.view_timestamp > b.view_timestamp;  
    });

    // 覆盖保存（用 std::ios::trunc 覆盖原文件）
    std::ofstream refile(filename, std::ios::trunc);
    for (const auto& rec : this->records) {
        // 关键：替换掉 |
        std::string safeTitle = rec.title;
        std::replace(safeTitle.begin(), safeTitle.end(), '|', ';');
        std::string safeTag = rec.tag;
        std::replace(safeTag.begin(), safeTag.end(), '|', ';');

        refile << rec.view_timestamp << "|"
             << safeTitle << "|"
             << safeTag << "|"
             << rec.bvid << "|"
             << rec.business << "|"
             << rec.progress << "|"
             << rec.duration << "\n";
    }
}

// 读取整个文件
std::string Gethist::readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Gethist::saveRecordsToFile() {
    // 1. 加载已有记录的键值集合
    std::unordered_set<std::string> existingKeys;
    std::ifstream readFile(this->filename);
    if (readFile.is_open()) {
        std::string line;
        while (std::getline(readFile, line)) {
            if (line.empty()) continue;
            // 提取前三个字段作为键值（时间戳|标题|tag）
            size_t first = line.find('|');
            if (first == std::string::npos) continue;
            size_t second = line.find('|', first + 1);
            if (second == std::string::npos) continue;
            size_t third = line.find('|', second + 1);
            if (third == std::string::npos) continue;
            // 用前三个字段组合成键值
            std::string key = line.substr(0, third);
            existingKeys.insert(key);
        }
        readFile.close();
    }

    // 2. 打开文件以追加模式写入
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) return;

    // 3. 遍历新记录，只写入键值不存在的记录
    for (const auto& rec : records) {
        std::string safeTitle = rec.title;
        std::replace(safeTitle.begin(), safeTitle.end(), '|', ';');
        std::string safeTag = rec.tag;
        std::replace(safeTag.begin(), safeTag.end(), '|', ';');    
        std::string finalTag = rec.tag.empty() ? "未分类" : safeTag;

        // 生成键值（与写入格式完全一致）
        std::string key = std::to_string(rec.view_timestamp) + "|" + safeTitle + "|" + finalTag;
        // 如果已存在，跳过
        if (existingKeys.find(key) != existingKeys.end()) {
            continue;
        }

        // 写入新记录
        file << rec.view_timestamp << "|"
             << safeTitle << "|"
             << (rec.tag.empty() ? "未分类" : safeTag) << "|"
             << (rec.bvid.empty() ? "N/A" : rec.bvid) << "|"
             << rec.business << "|"
             << rec.progress << "|"
             << rec.duration << "\n";

        // 将新键值加入集合，避免同一批次内重复写入
        existingKeys.insert(key);
    }
    saveOnDestruct = true;
}

// 从API获取数据的回调函数, 追加到MemoryStruct缓冲区
size_t Gethist:: WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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
std::string Gethist::getURL(long max, long view_at) {
    std::string base_url = "https://api.bilibili.com/x/web-interface/history/cursor";
    return base_url + "?max=" + std::to_string(max) + "&view_at=" + std::to_string(view_at) + "&ps=20";
}

int Gethist::getBiliRecord(long &max, long &view_at, const std::string& jsonData)
{
    // 采用RAII管理cJSON数据
    CJsonManager root(cJSON_Parse(jsonData.c_str()));
    if (!root) {
        std::cerr << "JSON解析失败" << std::endl;
        return -1;
    } 
    // 使用匿名函数解决返回错误
    auto getItem = [](cJSON* obj, const char* key) -> cJSON* {
        return obj ? cJSON_GetObjectItem(obj, key) : nullptr;
    };

    cJSON *data = cJSON_GetObjectItem(root.get(), "data");
    // 获取max, view_at
    cJSON *cursor = getItem(data, "cursor");
    cJSON *bmax = getItem(cursor, "max");
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
int Gethist::fetchBiliPage(long max, long view_at, const std::string& cookie, std::string& jsonData, CURL *curl_handle)
{
    struct MemoryStruct chunk = {nullptr, 0};
    int ret = -1;

    if (curl_handle) {
        std::string url = getURL(max, view_at);

        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &Gethist::WriteMemoryCallback);
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
int Gethist::getBiliHistory()
{
    long max = 0, view_at = 0;
    long prev_max = -1, prev_view_at = -1;
    int pageCount = 0;
    std::string jsonData;
    CURL *curl_handle = nullptr;
    

    std::string configContent = readFileToString("data/config.json");
    cJSON *configRoot = cJSON_Parse(configContent.c_str());
    if (configRoot == nullptr) {
        std::cerr << "解析配置文件失败" << std::endl;
        return -1;
    }
    std::string bili_cookie = cJSON_GetObjectItem(configRoot, "cookie")->valuestring;
    cJSON_Delete(configRoot); 

    // 初始化libcurl全局环境
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    while (true)
    {
        if (fetchBiliPage(max, view_at, bili_cookie, jsonData, curl_handle) != 0) {
            std::cerr << "获取数据错误" << std::endl;
        }

        int rec = getBiliRecord(max, view_at, jsonData);
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

    saveRecordsToFile();
    curl_easy_cleanup(curl_handle);
    // 清理 libcurl 全局环境
    curl_global_cleanup();

    return 0;
}


void Gethist::loadRecordsFromFile() {
    std::ifstream file(this->filename);
    if (!file.is_open()) {
        std::cout << ">>> 文件打开失败: " << this->filename << std::endl;
        return;
    }

    this->records.clear();
    std::string line;
    int lineNo = 0;
    int good = 0;
    int bad = 0;

    while (std::getline(file, line)) {
        lineNo++;
        if (line.empty()) continue;

        try {
            BiliRecord rec;
            size_t pos = 0, next;

            next = line.find('|');
            if (next == std::string::npos) { bad++; continue; }
            rec.view_timestamp = std::stol(line.substr(pos, next - pos));
            pos = next + 1;

            next = line.find('|', pos);
            if (next == std::string::npos) { bad++; continue; }
            rec.title = line.substr(pos, next - pos);
            pos = next + 1;

            next = line.find('|', pos);
            if (next == std::string::npos) { bad++; continue; }
            rec.tag = line.substr(pos, next - pos);
            pos = next + 1;

            next = line.find('|', pos);
            if (next == std::string::npos) { bad++; continue; }
            rec.bvid = line.substr(pos, next - pos);
            pos = next + 1;

            next = line.find('|', pos);
            if (next == std::string::npos) { bad++; continue; }
            rec.business = line.substr(pos, next - pos);
            pos = next + 1;

            next = line.find('|', pos);
            if (next == std::string::npos) { bad++; continue; }
            rec.progress = std::stoi(line.substr(pos, next - pos));
            pos = next + 1;

            rec.duration = std::stoi(line.substr(pos));

            this->records.push_back(rec);
            good++;
        }
        catch (const std::exception& e) {
            std::cout << ">>> 第 " << lineNo << " 行坏: " << e.what() << std::endl;
            bad++;
            continue;
        }
    }

    file.close();
    std::cout << ">>> 读入 " << good << " 条，跳过 " << bad
              << " 条，共 " << lineNo << " 行" << std::endl;
}
// 先curl接受数据
// 然后解析数据，重组织数据
// 最后应用数据
// 那么该怎么组织函数？