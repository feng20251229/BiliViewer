#include "viewer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cjson/cjson.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>

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

/**
 * 获取B站历史记录
 * @return 0 成功, -1 失败
 */
int getBiliHistory(std::vector<BiliRecord>& records)
{
    // 1.初始化变量
    CURL *curl_handle = nullptr;
    CURLcode res;
    struct MemoryStruct chunk = {nullptr, 0};
    int ret_val = -1;
    cJSON *root = nullptr;

    std::string configContent = readFileToString(".gitignore/config.json");
    cJSON *configRoot = cJSON_Parse(configContent.c_str());
    if (configRoot == nullptr) {
        std::cerr << "解析配置文件失败" << std::endl;
        return 0;
    }
    std::string bili_history_url = cJSON_GetObjectItem(configRoot, "api_url")->valuestring;
    std::string bili_cookie = cJSON_GetObjectItem(configRoot, "cookie")->valuestring;
    int timeout = cJSON_GetObjectItem(configRoot, "timeout")->valueint;

    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (curl_handle == nullptr) {
        std::cerr << "libcurl句柄初始化失败" << std::endl;
        goto CLEANUP;
    }

    // 2.配置crul请求参数
    curl_easy_setopt(curl_handle, CURLOPT_URL, bili_history_url.c_str()); // 设置URL
    curl_easy_setopt(curl_handle, CURLOPT_COOKIE, bili_cookie.c_str()); // 设置Cookie
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // 3.执行请求
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        std::cout << "获取成功，数据长度：" << chunk.size << " 字节" << std::endl;
    } else {
        std::cerr << "请求失败：" << curl_easy_strerror(res) << std::endl;
        goto CLEANUP;
    }

    root = cJSON_Parse(chunk.memory);
    if (root == nullptr) {
        std::cerr << "JSON解析失败" << std::endl;
        
    } else {
        cJSON *data = cJSON_GetObjectItem(root, "data");
        cJSON *list = cJSON_GetObjectItem(data, "list");
        int count = cJSON_GetArraySize(list);
        std::cout << "共获取到 " << count << "条历史记录" << std::endl;

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
    }

    cJSON_Delete(root);
    ret_val = 0;

    

CLEANUP:
    // 4.清理所有资源
    free(chunk.memory);
    if (curl_handle) {
        curl_easy_cleanup(curl_handle);
    }
    curl_global_cleanup();

    return ret_val;
}