#include "data.h"

// 将Unix时间戳转换为 "YYYY-MM-DD" 格式的字符串
static std::string timestampToDate(long timestamp) {
    using namespace std::chrono;
    // 将时间戳转化为时间点
    system_clock::time_point tp = system_clock::from_time_t(timestamp);
    // 转换为本地时间
    std::time_t tt = system_clock::to_time_t(tp);
    std::tm local_tm = *std::localtime(&tt);
    // 格式化为字符串
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d");
    return oss.str();
}

// 将"YYYY-MM-DD" 格式的字符串转换为时间戳
static long dateToTimestamp(const std::string& dataStr) {
    std::tm tm = {};
    std::istringstream ss(dataStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::time_t tt = std::mktime(&tm);
    return static_cast<long>(tt);
}

// 时间筛选
std::vector<BiliRecord> filterRecordsByDate(
    const std::vector<BiliRecord>& records,
    const std::string& startDate,
    const std::string& endDate) 
{
    long start = dateToTimestamp(startDate);
    long end = dateToTimestamp(endDate);
    // 如果希望包含结束日期全天，取消下面这行的注释
    if (start == end) {
        end += 24 * 60 * 60 - 1; 
    }

    std::vector<BiliRecord> result;
    for (const auto& rec : records) {
        if (rec.view_timestamp >= start && rec.view_timestamp <= end) {
            result.push_back(rec);
        }
    }
    return result;
}

void analyzeCompletionRate(const std::vector<BiliRecord> &records) {
    double totalProgress = 0;
    int allCount = 0;
    
    for (const auto &rec : records) {
        if (rec.duration <= 0) continue;
        if (rec.progress == -1) {
            totalProgress += 1;
            allCount++;
            continue;
        }

        totalProgress += (double)rec.progress / rec.duration;
        allCount++;
    }
 
    std::cout << "整体平均观看完成率：" << (totalProgress / allCount* 100) << "%" << std::endl;
}