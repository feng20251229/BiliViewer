#include "viewer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cjson/cjson.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>

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
int getBiliHistory()
{
    // 初始化变量
    CURL *curl_handle = nullptr;
    CURLcode res;
    struct MemoryStruct chunk = {nullptr, 0};
    int ret_val = -1;

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

    // 配置crul请求参数
    curl_easy_setopt(curl_handle, CURLOPT_URL, bili_history_url.c_str()); // 设置URL
    curl_easy_setopt(curl_handle, CURLOPT_COOKIE, bili_cookie.c_str()); // 设置Cookie
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // 执行请求
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        std::cout << "获取成功，数据长度：" << chunk.size << " 字节" << std::endl;
        std::cout << chunk.memory << std::endl;
    } else {
        std::cerr << "请求失败：" << curl_easy_strerror(res) << std::endl;
    }

    ret_val = 0;

CLEANUP:
    // 清理所有资源
    free(chunk.memory);
    if (curl_handle) {
        curl_easy_cleanup(curl_handle);
    }
    curl_global_cleanup();

    return ret_val;
}