#include "connect.h"
#include "data.h"

std::vector<BiliRecord> myRecords;

void getRecordsFromWeb() {
    int result = getBiliHistory(myRecords);
    if (result != 0) {
        std::cerr << "获取数据失败" << std::endl;
    }
}

void getRecordsFromData() {
    std::string filename = ".gitignore/biliHistory";
    loadRecordsFromFile(filename, myRecords);
}

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
    std::cout << "从 " << startDate << " 到" << endDate << " 共看时长为" << timeSum % 60 << std::endl;
}


std::vector<BiliRecord> selectRecordsFromDate(std::string &startDate, std::string &endDate) {
    auto records = filterRecordsByDate(myRecords, startDate, endDate);
    std::cout << "观看量：" << records.size() << " 条" << std::endl;
    return records;
}

int main()
{
    std::vector<BiliRecord> selectRecords;
    // 通过网站访问获取历史记录
    //getRecordsFromWeb();
    // 通过储存数据获取历史记录
    getRecordsFromData();
    
    /*std::string startDate = "2026-08-12";
    std::string endDate = "2026-08-12";
    // 根据时间段返回在内的历史记录
    selectRecords = selectRecordsFromDate(startDate, endDate);*/
    
    // 返回观看率
    //analyzeCompletionRate(myRecords);
    /// 返回所有出现的tag
    //getTag();

    std::string startDate = "2026-08-10";
    displayWeekRecords(myRecords, startDate, 7);

    return 0;
} 

// 界面设计
// 观看量趋势图
// 兴趣分析