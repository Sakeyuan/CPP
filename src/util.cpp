#include <execinfo.h>
#include "util.h"
#include "log.h"
#include "fiber.h"

namespace sake
{
    sake::Logger::ptr g_logger = SAKE_LOG_NAME("system");
    pid_t
    util::GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t util::GetFiberId()
    {
        return sake::Fiber::GetFiberId();
    }

    void util::Backtrace(std::vector<std::string> &bt, int size, int skip = 1)
    {
        void **array = (void **)malloc(sizeof(void *) * size);
        if (!array)
        {
            SAKE_LOG_ERROR(g_logger) << "malloc failed";
            return;
        }

        size_t s = ::backtrace(array, size);
        char **strings = backtrace_symbols(array, s);
        if (strings == NULL)
        {
            SAKE_LOG_ERROR(g_logger) << "backtrace_symbols failed";
            free(array);
            return;
        }

        for (size_t i = skip; i < s; ++i) // 注意 size 和 s 的区别
        {
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);
    }

    std::string util::BacktraceToString(int size, int skip = 2, const std::string &prefix)
    {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i)
        {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
}
