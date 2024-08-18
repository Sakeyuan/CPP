#include "thread.h"
#include "log.h"
#include "util.h"

namespace sake
{
    Semaphore::Semaphore(uint32_t count)
    {
        if (sem_init(&m_semaphore, 0, count))
        {
            throw std::logic_error("sem_init failed");
        }
    }

    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait()
    {

        if (sem_wait(&m_semaphore))
        {
            throw std::logic_error("sem_wait failed");
        }
    }

    void Semaphore::notify()
    {
        if (sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post failed");
        }
    }

    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";
    static sake::Logger::ptr g_logger = SAKE_LOG_NAME("system");
    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }

    void Thread::setName(const std::string &name)
    {
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(std::function<void()> cb, const std::string &name)
        : m_cb(cb), m_name(name) // 初始化列表
    {
        if (name.empty())
        {
            m_name = "UNKNOW";
        }
        int ret = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (ret)
        {
            SAKE_LOG_ERROR(g_logger) << "pthread_create failed, ret=" << ret << " name = " << name;
            throw std::logic_error("pthread_create failed");
        }
        m_semaphore.wait(); // 等待线程启动完成
    }

    Thread::~Thread()
    {
        if (m_thread)
        {
            pthread_detach(m_thread);
        }
    }

    void *Thread::run(void *arg)
    {
        Thread *thread = (Thread *)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = sake::util::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        std::function<void()> cb;
        cb.swap(thread->m_cb); // 交换 m_cb 的内容到局部变量 cb
        thread->m_semaphore.notify();
        if (cb) // 检查 cb 是否为空
        {
            cb(); // 调用回调函数
        }
        return 0;
    }

    void Thread::join()
    {
        if (m_thread)
        {
            int ret = pthread_join(m_thread, nullptr);
            if (ret)
            {
                SAKE_LOG_ERROR(g_logger) << "pthread_join failed, ret=" << ret << "name = " << m_name;
                throw std::logic_error("pthread_join failed");
            }
            m_thread = 0;
        }
    }

}
