#pragma once
#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"
#include "log.h"

namespace sake
{
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum State
        {
            INIT,   // 初始化状态
            HOLD,   // 等待状态
            EXEC,   // 执行状态
            TERM,   // 终止状态
            READY,  // 就绪状态
            EXCEPT, // 异常状态
        };

    private:
        Fiber();

    public:
        Fiber(std::function<void()> cb, size_t stacksize = 0);
        ~Fiber();

        // 重置协程函数，并重置状态
        // INIT , TERM
        void reset(std::function<void()> cb);
        void swapIn();  // 切换到当前协程执行
        void swapOut(); // 把当前协程放到后台
        uint64_t getId() const { return m_id; }

    public:
        static void SetThis(Fiber *f); // 设置当前协程
        static Fiber::ptr GetThis();   // 返回当前执行点协程
        static void YieldToReady();    // 协程切换到后台，切换到就绪状态
        static void YieldToHold();     // 协程切换到后台，切换到等待状态
        static uint64_t TotalFibers(); // 总的Fiber数量

        static void MainFunc(); // 主函数入口
        static uint64_t GetFiberId();

    private:
        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;
        State m_state = INIT;

        ucontext_t m_ctx;
        void *m_stack = nullptr;

        std::function<void()> m_cb;
    };
}
