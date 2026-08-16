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
static long dateToTimestamp(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::time_t tt = std::mktime(&tm);
    return static_cast<long>(tt);
}

static std::string addDays(const std::string& dateStr, int days) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    std::time_t tt = std::mktime(&tm);
    tt += days * 24 * 60 * 60;

    std::tm* newTm = std::localtime(&tt);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", newTm);
    return std::string(buffer);
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

// 获取时长
int getTimeFromRecords(
    const std::vector<BiliRecord> &records,
    const std::string& startDate,
    const std::string& endDate) 
{
    long start = dateToTimestamp(startDate);
    long end = dateToTimestamp(endDate);
    
    if (start == end) {
        end += 24 * 60 * 60 - 1;
    }

    int timeSumSecond = 0;
    for (const auto& rec : records) {
        if (rec.view_timestamp >= start && rec.view_timestamp <= end) {
            if (rec.progress == -1) {
                timeSumSecond += rec.duration;
                // 看评论
                timeSumSecond += 10;
            }
            else {
                timeSumSecond += rec.progress;
                if (rec.progress > rec.duration / 2) 
                timeSumSecond += 5;
            }
        }
    }
    
    return timeSumSecond;
}

// 整体平均观看完成率
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

// 图形化展示一周观看时长+条数
void displayWeekRecords(const std::vector<BiliRecord> &records, const std::string &startDate, int day) {
    const int width = 50;
    
    for (int i = 0; i < day; i++) {
        std::string date = addDays(startDate, i);
        auto rec = filterRecordsByDate(records, date, date);

        int count = rec.size();
        int time = getTimeFromRecords(rec, date, date) / 60;

        int countLen = (count > 0) ? std::max(1, count * width / 500) : 0;
        int timeLen = (time > 0) ? std::max(1, time * width / 500) : 0;

        std::cout << date << ": ";

        std::cout << "[";
        for (int i = 0; i < width; i++) {
            std::cout << (i < countLen ? "=" : " ");
        }
        std::cout << "] " << count << "条" << std::endl;

        std:: cout << std::string (date.length() + 2, ' ') << "[";
        for (int i = 0; i < width; i++) {
            std::cout << (i < timeLen ? "#" : " ");
        }
        std::cout << "] " << time / 60 << "小时 " << time % 60 << "分钟" << std::endl;

    }
}