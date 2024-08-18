#include "./util.h"
#include "util.h"
namespace sake
{
    pid_t util::GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t util::GetFiberId()
    {
        return 0;
    }
}
