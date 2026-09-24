#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include "connect.h"

class Danalysis{
public:
    Danalysis(const std::vector<BiliRecord> &inputRecords) : records{inputRecords} {}
    ~Danalysis();
    //外部接口
    // 获取规定时间内记录
    std::vector<BiliRecord> filterRecordsByDate(const std::string& startDate, const std::string& endDate);
    // 获取观看时间
    int getTimeFromRecords(const std::string& startDate, const std::string& endDate);
    // 分析观看/完成比
    void analyzeCompletionRate();
    // 输出一周观看数据
    void displayWeekRecords(const std::string &startDate, int day);
    static std::string timestampToDate(long timestamp);
private:
    long dateToTimestamp(const std::string& dateStr);
    std::string addDays(const std::string& dateStr, int days);

    const std::vector<BiliRecord> records;
};