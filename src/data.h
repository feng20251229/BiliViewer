#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include "connect.h"

// 根据时间范围过滤记录
std::vector<BiliRecord> filterRecordsByDate(
    const std::vector<BiliRecord>& records,
    const std::string& startDate,
    const std::string& endDate
);

int getTimeFromRecords(
    const std::vector<BiliRecord> &records,
    const std::string& startDate,
    const std::string& endDate
);

void analyzeCompletionRate(const std::vector<BiliRecord> &records);

void displayWeekRecords(const std::vector<BiliRecord>& records, const std::string& startDate, int day);

