#pragma once
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>
#include <string>

namespace sake
{
    class util
    {
    public:
        static pid_t GetThreadId();
        static uint32_t GetFiberId();
        static void Backtrace(std::vector<std::string> &bt, int size, int skip);
        static std::string BacktraceToString(int size, int skip, const std::string &prefix = "");
    };
};
