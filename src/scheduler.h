#pragma once

#include <memory>
#include "fiber.h"
#include "mu tex.h"
#include "thread.h"
namespace sake
{
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");
        virtual ~Scheduler();
        const std::string &getName() const { return m_name; }

        static Scheduler *GetThis();
        static Fiber *GetMainFiber();

        void start();
        void stop();
        template <class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = scheduleNoLock(fc, thread);
            }
            if (need_tickle)
            {
                tickle();
            }
        }
        template <class InputIterator>
        void schedule(InputIterator first, InputIterator last)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while (begin != end)
                {
                    need_tickle = scheduleNoLock(&*begin) || need_tickle;
                }
            }
            if (need_tickle)
            {
                tickle();
            }
        }

    protected:
        virtual void tickle();

    private:
        template <class FiberOrCb>
        bool scheduleNoLock(FiberOrCb fc, int thread)
        {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(fc, thread);
            if (ft.fiber || ft.cb)
            {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }

    private:
        struct FiberAndThread
        {
            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread;
            FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}
            FiberAndThread(Fiber::ptr f, int thr) : thread(thr)
            {
                fiber.swap(*f);
            }

            FiberAndThread(std::function<void()> f, int thr) : cb(f), thread(thr)
            {
            }

            FiberAndThread(std::function<void()> f, int thr) : thread(thr)
            {
                cb.swap(*f);
            }
            FiberAndThread() : thread(-1)
            {
            }

            void reset()
            {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        MutexType m_mutex;
        std::vector<Thread::ptr> m_threads;
        std::list<FiberAndThread> m_fibers;
        std::string m_name;
    };
}