#pragma once
#include <string>
#include <stdint.h>
#include <memory>
#include <yaml-cpp/yaml.h>
#include <list>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <functional>
#include <tuple>
#include <time.h>
#include <string.h>
#include <map>
#include <thread>
#include <pthread.h>
#include <stdarg.h>
#include "singleton.h"
#include "util.h"
#include "thread.h"

#define SAKE_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level)  \
    sake::LogEventWrap(sake::LogEvent::ptr(new sake::LogEvent(logger, level, __FILE__, __LINE__, 0, sake::util::GetThreadId(), sake::util::GetFiberId(), time(0)))).getSS()

#define SAKE_LOG_DEBUG(logger) SAKE_LOG_LEVEL(logger, sake::LogLevel::Level::DEBUG)
#define SAKE_LOG_INFO(logger) SAKE_LOG_LEVEL(logger, sake::LogLevel::Level::INFO)
#define SAKE_LOG_WARN(logger) SAKE_LOG_LEVEL(logger, sake::LogLevel::Level::WARN)
#define SAKE_LOG_ERROR(logger) SAKE_LOG_LEVEL(logger, sake::LogLevel::Level::ERROR)
#define SAKE_LOG_FATAL(logger) SAKE_LOG_LEVEL(logger, sake::LogLevel::Level::FATAL)

#define SAKE_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                               \
    if (logger->getLevel() <= level)                                                                              \
    sake::LogEventWrap(sake::LogEvent::ptr(new sake::LogEvent(logger, level, __FILE__, __LINE__, 0,               \
                                                              sake::GetThreadId(), sake::GetFiberId(), time(0)))) \
        .getEvent()                                                                                               \
        ->format(fmt, __VA_ARGS__)

#define SAKE_LOG_FMT_DEBUG(logger, fmt, ...) SAKE_LOG_FMT_LEVEL(logger, sake::LogLevel::Level::DEBUG, fmt, __VA_ARGS__)
#define SAKE_LOG_FMT_INFO(logger, fmt, ...) SAKE_LOG_FMT_LEVEL(logger, sake::LogLevel::Level::INFO, fmt, __VA_ARGS__)
#define SAKE_LOG_FMT_WARN(logger, fmt, ...) SAKE_LOG_FMT_LEVEL(logger, sake::LogLevel::Level::WARN, fmt, __VA_ARGS__)
#define SAKE_LOG_FMT_ERROR(logger, fmt, ...) SAKE_LOG_FMT_LEVEL(logger, sake::LogLevel::Level::ERROR, fmt, __VA_ARGS__)
#define SAKE_LOG_FMT_FATAL(logger, fmt, ...) SAKE_LOG_FMT_LEVEL(logger, sake::LogLevel::Level::FATAL, fmt, __VA_ARGS__)

#define SAKE_LOG_ROOT() sake::LoggerMgr::GetInstance()->getRoot()
#define SAKE_LOG_NAME(name) sake::LoggerMgr::GetInstance()->getLogger(name)

namespace sake
{
    class Logger;
    class LogEvent;
    class LogAppender;
    class LogFormatter;
    class FormatItem;
    class StdoutLogAppender;
    class FileLogAppender;
    class LogLevel;
    // 日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
        // 将日志级别转换成为文本
        static const char *ToString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string &str);
    };

    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint64_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getElapse() const { return m_elapse; }
        uint64_t getTime() const { return m_time; }
        std::string getContent() const { return m_ss.str(); }
        std::stringstream &getSS() { return m_ss; }

        std::shared_ptr<Logger> getLogger() { return m_logger; }
        LogLevel::Level getLevel() { return m_level; }

        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);

    private:
        // 文件名称
        const char *m_file = nullptr;
        // 行号
        int32_t m_line = 0;
        // 程序启动到现在的毫秒数
        uint64_t m_elapse = 0;
        // 线程Id
        uint32_t m_threadId = 0;
        // 协程Id
        uint32_t m_fiberId = 0;
        // 时间
        uint64_t m_time = 0;
        // 消息
        std::stringstream m_ss;
        // 日志器
        std::shared_ptr<Logger> m_logger;
        // 日志级别
        LogLevel::Level m_level;
    };

    // logevent日志事件包装器
    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        std::stringstream &getSS();
        LogEvent::ptr getEvent() const { return m_event; }

    private:
        LogEvent::ptr m_event;
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        LogFormatter(const std::string &pattern);

    public:
        // 日志解析模块
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        // 解析pattern
        void init();

        bool isError() const { return m_isError; }

        const std::string getPattern() const { return m_pattern; }

    private:
        // 日志格式模板
        std::string m_pattern;

        // 日志解析后的格式
        std::vector<FormatItem::ptr> m_items;

        bool m_isError = false;
    };

    // 日志输出地
    class LogAppender
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<LogAppender> ptr;
        typedef SpinLock MutexType;
        virtual ~LogAppender() {}
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;

        void setFormater(LogFormatter::ptr formatter);
        LogFormatter::ptr getFormatter();
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;

        bool m_hasFormatter = false;

        LogFormatter::ptr m_formatter;
        MutexType m_mutex;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
        friend class LoggerManager;

    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef SpinLock MutexType;
        Logger(const std::string name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        // 添加日志落地目标
        void addAppender(LogAppender::ptr appender);

        // 删除日志落地目标
        void delAppender(LogAppender::ptr appender);

        void clearAppenders();

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }
        const std::string &getName() const { return m_name; }

        void setLogFormatter(LogFormatter::ptr val);
        void setLogFormatter(const std::string &val);

        LogFormatter::ptr getLogFormatter();

        std::string toYamlString();

    private:
        // 日志名称
        std::string m_name;

        // 日志级别
        LogLevel::Level m_level;

        // 输出队列
        std::list<LogAppender::ptr> m_appenders;

        // 日志格式器
        LogFormatter::ptr m_formatter;

        // 主日志器
        Logger::ptr m_root;

        MutexType m_mutex;
    };

    // 输出到控制台
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;
    };

    // 输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen();
        std::string toYamlString() override;

    private:
        std::string m_filename;
        std::ofstream m_filestream;
        uint64_t m_lastTime = 0;
    };

    class LoggerManager
    {
    public:
        typedef SpinLock MutexType;
        LoggerManager();
        Logger::ptr getLogger(const std::string &name);
        void init();
        Logger::ptr getRoot() const { return m_root; }

        std::string toYamlString();

    private:
        // 日志容器
        std::map<std::string, Logger::ptr> m_loggers;

        // 主日志器
        Logger::ptr m_root;

        MutexType m_mutex;
    };

    // 日志管理单例类
    typedef Singleton<LoggerManager> LoggerMgr;

}
