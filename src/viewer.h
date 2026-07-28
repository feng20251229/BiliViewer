#ifndef VIEWER_H
#define VIEWER_H

#include <iostream>
#include <cstddef>

typedef struct {
    std::string title;          // 视频标题
    std::string tag;            // 分区标签
    std::string bvid;           // 视频ID
    int duration;               // 观看停留时长
    long view_timestamp;        // 观看时间戳
} BiliRecord;

int getBiliHistory();

#endif