#pragma once
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace sake
{
    class util
    {
    public:
        static pid_t GetThreadId();
        static uint32_t GetFiberId();
    };
};
