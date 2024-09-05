#include "sake.h"
sake::Logger::ptr g_logger = SAKE_LOG_ROOT();

void run_in_fiber()
{
    SAKE_LOG_INFO(g_logger) << "run_in_fiber begin";
    sake::Fiber::YieldToHold();
    SAKE_LOG_INFO(g_logger) << "run_in_fiber end";
    sake::Fiber::YieldToHold();
}

void test_fiber()
{
    SAKE_LOG_INFO(g_logger) << "main begin -1";
    {
        sake::Fiber::GetThis();
        SAKE_LOG_INFO(g_logger) << "main begin";
        sake::Fiber::ptr fiber(new sake::Fiber(run_in_fiber));
        fiber->swapIn();
        SAKE_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SAKE_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SAKE_LOG_INFO(g_logger) << "main after end2";
}

int main()
{
    sake::Thread::setName("main");
    std::vector<sake::Thread::ptr> threads;
    for (int i = 0; i < 3; i++)
    {
        threads.push_back(sake::Thread::ptr(new sake::Thread(test_fiber, "name_" + std::to_string(i))));
    }

    for (auto i : threads)
    {
        i->join();
    }

    return 0;
}