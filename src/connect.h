#ifndef CONNECT_H
#define CONNECT_H

#include <iostream>
#include <cstddef>
#include <vector>
#include <unordered_set>

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

int getBiliHistory(std::vector<BiliRecord>& records);

void loadRecordsFromFile(const std::string& filename, std::vector<BiliRecord>& records);

#endif