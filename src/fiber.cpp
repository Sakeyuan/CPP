#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>

namespace sake
{
    static std::atomic<uint64_t> s_fiber_id{0};
    static std::atomic<uint64_t> s_fiber_count{0};
    static Logger::ptr g_logger = SAKE_LOG_NAME("system");
    static thread_local Fiber *t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;
    static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "Fiber stack size in bytes");
    class MallocStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }

        static void Dealloc(void *vp, std::size_t size)
        {
            return free(vp);
        }
    };
    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber()
    {
        m_state = EXEC;
        SetThis(this);
        if (getcontext(&m_ctx))
        {
            SAKE_ASSERT2(false, "getcontext failed");
        }
        ++s_fiber_count;
    }

    Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        {
            SAKE_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stacksize);
            m_stack = nullptr;
        }
        // 主协程
        else
        {
            SAKE_ASSERT(!m_cb);
            SAKE_ASSERT(m_state == EXEC);
            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }
        SAKE_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id;
    }

    Fiber::Fiber(std::function<void()> cb, size_t stacksize)
        : m_id(++s_fiber_id),
          m_cb(cb)
    {
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
        m_stack = StackAllocator::Alloc(m_stacksize);
        if (getcontext(&m_ctx))
        {
            SAKE_ASSERT2(false, "getcontext failed");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        SAKE_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
    }

    // 重置协程函数，并重置状态
    // INIT , TERM
    void Fiber::reset(std::function<void()> cb)
    {
        SAKE_ASSERT(m_stack);
        SAKE_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
        m_cb = cb;
        if (getcontext(&m_ctx))
        {
            SAKE_ASSERT2(false, "getcontext failed");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    }

    void Fiber::swapIn() // 切换到当前协程执行,主协程转子协程
    {
        SetThis(this);
        SAKE_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
        {
            SAKE_ASSERT2(false, "swapcontext failed");
        }
    }

    void Fiber::swapOut() // 把当前协程放到后台，当前协程转主协程
    {
        SetThis(t_threadFiber.get());
        if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
        {
            SAKE_ASSERT2(false, "swapcontext failed");
        }
    }

    // 设置当前协程
    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }

    // 返回当前执行点协程
    Fiber::ptr Fiber::GetThis()
    {
        if (t_fiber)
        {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        SAKE_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    // 协程切换到后台，切换到就绪状态
    void Fiber::YieldToReady()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = READY;
        cur->swapOut();
    }

    // 协程切换到后台，切换到等待状态
    void Fiber::YieldToHold()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = HOLD;
        cur->swapOut();
    }

    // 总的Fiber数量
    uint64_t Fiber::TotalFibers()
    {
        return s_fiber_count;
    }

    // 主函数入口
    void Fiber::MainFunc()
    {
        Fiber::ptr cur = GetThis();
        SAKE_ASSERT(cur);
        try
        {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
        catch (const std::exception &e)
        {
            cur->m_state = EXCEPT;
            SAKE_LOG_ERROR(g_logger) << "Fiber Except: " << e.what();
        }
        catch (...)
        {
            cur->m_state = EXCEPT;
            SAKE_LOG_ERROR(g_logger) << "Fiber Except: unknown exception";
        }

        // 用裸指针，防止智能指针+1,调用协程析构
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();
        SAKE_ASSERT2(false, "never reach"); 
    }
    uint64_t Fiber::GetFiberId()
    {
        if (t_fiber)
        {
            return t_fiber->getId();
        }
        return 0;
    }
}