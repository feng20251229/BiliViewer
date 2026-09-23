#pragma once

#include <iostream>
#include <cstddef>
#include <vector>
#include <unordered_set>
#include <curl/curl.h>

typedef struct {
    std::string title;          // 视频标题
    std::string bvid;           // 视频ID
    std::string business;       // 模式（archive视频、live直播、pgc电影）
    long view_timestamp;        // 观看时间戳
    int progress;               // 停留时长
    int duration;               // 总时长
    int live_status;            // 直播状态（默认0）
    std::string tag;            // 分区标签
} BiliRecord;

// 用于存储API响应数据的结构
struct MemoryStruct {
    char *memory;
    size_t size;
};

enum readWay{
    Web,
    Data
};

class Gethist
{
public:
    Gethist(readWay way);
    ~Gethist();

    // 外部调用接口
    const std::vector<BiliRecord>& loadrecords() const {return records;}

private:
    std::string readFileToString(const std::string& filename);
    void saveRecordsToFile();
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
    std::string getURL(long max, long view_at);
    int getBiliRecord(long &max, long &view_at, const std::string& jsonData);
    int fetchBiliPage(long max, long view_at, const std::string& cookie, std::string& jsonData, CURL *curl_handle);

    int getBiliHistory();
    void loadRecordsFromFile();

    std::vector<BiliRecord> records;
    std::string filename = ".gitignore/biliHistory";
    std::unordered_set<std::string> seenKeys;
};
