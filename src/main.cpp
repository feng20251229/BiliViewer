#include "viewer.h"

int main()
{
    std::vector<BiliRecord> myRecords;
    int result = getBiliHistory(myRecords);
    if (result != 0) {
        std::cerr << "获取数据失败" << std::endl;
    }
    
    return 0;
}