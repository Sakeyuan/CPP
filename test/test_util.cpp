#include "sake.h"
#include <assert.h>

sake::Logger::ptr g_logger = SAKE_LOG_NAME("root");

void test_assert()
{
    SAKE_LOG_INFO(g_logger) << sake::util::BacktraceToString(10, 2, "   ");
    SAKE_ASSERT2(0==1,"abcd  xxx");
}

int main()
{
    test_assert();
    return 0;
}
