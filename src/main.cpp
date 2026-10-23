#include "connect.h"
#include "data.h"

int main()
{
    Gethist A = Gethist(Web);
    Danalysis B = Danalysis(A.loadrecords());
    return 0;
} 

//TODO:文化趋势追踪
//TODO:自动获取cookie ✅️
//TODO:载入数据库
//TODO:载入ai帮我找视频？