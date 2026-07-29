#include "viewer.h"
#include <map>

int main()
{
    std::vector<BiliRecord> myRecords;
    int result = getBiliHistory(myRecords);
    if (result != 0) {
        std::cerr << "获取数据失败" << std::endl;
    }

    auto records = filterRecordsByDate(myRecords, "2026-07-27", "2026-07-27");
    std::cout << "观看量：" << records.size() << " 条" << std::endl;

    return 0;
}