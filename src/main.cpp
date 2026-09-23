#include "connect.h"
#include "data.h"
#include <string.h>

std::vector<BiliRecord> myRecords;


void getTag() {
    std::unordered_set<std::string> uniqueTags;

    for (const auto &rec : myRecords) {
        uniqueTags.insert(rec.tag);
    }

    for (const auto &tag : uniqueTags) {
        std::cout << " " << tag;
    }
}

void getTime(std::string &startDate, std::string &endDate) {
    int timeSum = getTimeFromRecords(myRecords, startDate, endDate);
    std::cout << "从 " << startDate << " 到" << endDate << " 共看时长为" << timeSum / 60 << std::endl;
}

// 返回选定时间段内历史记录
std::vector<BiliRecord> selectRecordsFromDate(std::string &startDate, std::string &endDate) {
    auto records = filterRecordsByDate(myRecords, startDate, endDate);
    std::cout << "观看量：" << records.size() << " 条" << std::endl;
    return records;
}

int main()
{
    Gethist A = Gethist(Web);
    myRecords = A.loadrecords();
    
    /*
    std::string startDate = "2026-08-12";
    std::string endDate = "2026-08-12";
    
    if (strcmp(argv[2], "tag") == 0) {
        /// 返回所有出现的tag
        getTag();
    } else if (strcmp(argv[2], "time") == 0) {
        // 根据时间段返回在内的历史记录
        getTime(startDate, endDate);
    } else if (strcmp(argv[2], "rate") == 0) {
        // 返回观看率
        analyzeCompletionRate(myRecords);
    } else if (strcmp(argv[2], "week") == 0) {
        // 返回一周内近似观看条数和时长
        displayWeekRecords(myRecords, startDate, 7);
    } else {
        std::cout << "指令错误" << std::endl;
        return -1;
    }*/

    return 0;
} 

//文化趋势追踪
//自动获取cookie