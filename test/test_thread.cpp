#include "sake.h"
#include <vector>
#include <unistd.h>
int count = 0;
sake::Logger::ptr g_logger = SAKE_LOG_ROOT();
// sake::RWMutex s_mutex;
sake::Mutex s_mutex;
void fun1()
{
    SAKE_LOG_INFO(g_logger) << "name: " << sake::Thread::GetName()
                            << " this.name: "
                            << sake::Thread::GetThis()->getName()
                            << " id: "
                            << sake::util::GetThreadId()
                            << " this.id: " << sake::Thread::GetThis()->getId();
    for (int i = 0; i < 100000; i++)
    {
        sake::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2()
{
    while (true)
    {
        SAKE_LOG_INFO(g_logger) << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    }
}

void fun3()
{
    while (true)
    {
        SAKE_LOG_INFO(g_logger) << "=================================";
    }
}

int main(int argc, char **argv)
{
    SAKE_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node node = YAML::LoadFile("/home/yjf/LunixCPP/Config/logs2.yml");
    sake::Config::LoadFromYaml(node);
    std::vector<sake::Thread::ptr> threads;
    for (int i = 0; i < 2; i++)
    {
        sake::Thread::ptr thread1(new sake::Thread(&fun2, "name_" + std::to_string(i * 2)));
        sake::Thread::ptr thread2(new sake::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        threads.push_back(thread1);
        threads.push_back(thread2);
    }

    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i]->join();
    }
    SAKE_LOG_INFO(g_logger) << "thread test end";
    SAKE_LOG_INFO(g_logger) << "count : " << count;
}