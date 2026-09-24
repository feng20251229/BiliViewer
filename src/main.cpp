#include "connect.h"
#include "data.h"

int main()
{
    Gethist A = Gethist(Web);
    Danalysis B = Danalysis(A.loadrecords());
    
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