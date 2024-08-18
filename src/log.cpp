#include "log.h"
#include "config.h"

namespace sake
{
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H-%M-%S") : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M%S";
            }
        }

        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FileNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str)
        {
        }
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "")
        {
        }
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }

    private:
        std::string m_string;
    };

    class FiberFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    Logger::Logger(const std::string name) : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T[%p]%T[%c]%T%t%T%F%T%f:%l%T%m%n"));
    }

    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint64_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time)
        : m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_logger(logger), m_level(level)
    {
    }

    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX

        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }
    LogLevel::Level LogLevel::FromString(const std::string &str)
    {
#define XX(level, v)            \
    if (str == #v)              \
    {                           \
        return LogLevel::level; \
    }
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKOWN;
#undef XX
    }
    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            // 获取类的智能指针
            auto self = shared_from_this();
            MutexType::Lock lock(m_mutex);
            if (!m_appenders.empty())
            {
                for (auto &it : m_appenders)
                {
                    it->log(self, level, event);
                }
            }
            else if (m_root)
            {
                m_root->log(level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        if (!appender->getFormatter())
        {
            MutexType::Lock ll(appender->m_mutex);
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders()
    {
        m_appenders.clear();
    }

    void Logger::setLogFormatter(LogFormatter::ptr val)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        for (auto &i : m_appenders)
        {
            MutexType::Lock ll(i->m_mutex);
            if (!i->m_hasFormatter)
            {
                i->m_formatter = m_formatter;
            }
        }
    }

    void Logger::setLogFormatter(const std::string &val)
    {
        sake::LogFormatter::ptr new_val(new sake::LogFormatter(val));
        if (new_val->isError())
        {
            std::cout << "Logger setLogFormatter name = " << m_name
                      << " value = " << val << " invalid formatter" << std::endl;
            return;
        }
        setLogFormatter(new_val);
    }

    LogFormatter::ptr Logger::getLogFormatter()
    {
        return m_formatter;
    }

    std::string Logger::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevel::ToString(m_level);
        if (m_level != LogLevel::UNKOWN)
        {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void LogAppender::setFormater(LogFormatter::ptr formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = formatter;
        if (m_formatter)
        {
            m_hasFormatter = true;
        }
        else
        {
            m_hasFormatter = false;
        }
    }

    LogFormatter::ptr LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        reopen();
    }

    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios::app);
        return !m_filestream;
    }

    std::string FileLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKOWN)
        {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_hasFormatter && m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            uint64_t now = time(0);
            if (now != m_lastTime)
            {
                reopen();
                m_lastTime = now;
            }
            MutexType::Lock lock(m_mutex);
            if (!(m_filestream << m_formatter->format(logger, level, event)))
            {
                std::cout << "FileLogAppender log error: " << m_filename << std::endl;
            }
        }
    }

    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string StdoutLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKOWN)
        {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_hasFormatter && m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        init();
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &it : m_items)
        {
            it->format(ss, logger, level, event);
        }
        return ss.str();
    }

    //%X %X{} %X[X]
    /**
     * %m -- 消息体
     * %p -- level
     * %r -- 启动后的时间
     * %c -- 日志名称
     * %t -- 线程Id
     * %n -- 回车
     * %d -- 时间
     * %f -- 文件名
     * %l -- 行号
     * %T -- Tab
     * */

    void LogFormatter::init()
    {
        // str format type
        // type == 0 特殊符号 type == 1 m、p、r、c等标识符号
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            // 遇到[ 或者 ]
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            //%%
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, m_pattern[i]);
                    continue;
                }
            }
            size_t n = i + 1;
            // fmt_status == 0 解析str，fmt_status == 1 解析fmt
            int fmt_status = 0;
            size_t fmt_begin;

            std::string str;
            std::string fmt;
            while (n < m_pattern.size())
            {
                // 状态0，也不是字母，也不是{ 和 },即m_pattern[n]是[ 或者 ] 或者 %
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        // 截取 %Y-%m-%d %H-%M-%S
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_pattern.size())
                {
                    if (str.empty())
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_isError = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }
        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
        // 映射，不同标识符->不同FormatItem
        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
        #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, DateTimeFormatItem),
            XX(f, FileNameFormatItem),
            XX(l, LineFormatItem),
            XX(T, TabFormatItem),
            XX(F, FiberFormatItem)

#undef XX
        };

        for (auto &i : vec)
        {
            // type
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_isError = true;
                }
                else
                {
                    // fmt
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
        // std::cout << m_items.size() << std::endl;
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e)
    {
    }

    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getSS();
    }

    void LogEvent::format(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        format(fmt, args);
        va_end(args);
    }

    void LogEvent::format(const char *fmt, va_list args)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, args);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
        m_loggers[m_root->m_name] = m_root;
        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return it->second;
        }
        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    struct LogAppenderDefine
    {
        int type; // 1 Fiel 2 Std
        LogLevel::Level level = LogLevel::UNKOWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &oth) const
        {
            return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == oth.appenders;
        }
        bool operator<(const LogDefine &oth) const
        {
            return name < oth.name;
        }
    };

    template <>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> s;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                auto n = node[i];
                if (!n["name"].IsDefined())
                {
                    std::cout << "log conf error : name is null" << n << std::endl;
                    continue;
                }
                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
                if (n["formatter"].IsDefined())
                {
                    ld.formatter = n["formatter"].as<std::string>();
                }
                if (n["appenders"].IsDefined())
                {
                    for (size_t j = 0; j < n["appenders"].size(); ++j)
                    {
                        auto a = n["appenders"][j];

                        if (!a["type"].IsDefined())
                        {
                            std::cout << "log conf error : appeners type is null" << n << std::endl;
                            continue;
                        }
                        std::string type = a["type"].as<std::string>();
                        LogAppenderDefine lad;
                        if (type == "FileLogAppender")
                        {
                            lad.type = 1;
                            if (!a["file"].IsDefined())
                            {
                                std::cout << "log conf error : fileappener file is invalid" << a << std::endl;
                                continue;
                            }
                            lad.file = a["file"].as<std::string>();
                            if (a["formatter"].IsDefined())
                            {
                                lad.formatter = a["formatter"].as<std::string>();
                            }
                        }
                        else if ("StdoutLogAppender")
                        {
                            lad.type = 2;
                        }
                        else
                        {
                            std::cout << "log conf error : appeners type is invalid" << n << std::endl;
                            continue;
                        }
                        ld.appenders.push_back(lad);
                    }
                }
                s.insert(ld);
            }
            return s;
        }
    };

    template <>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                YAML::Node n;
                n["name"] = i.name;
                if (i.level != LogLevel::UNKOWN)
                {
                    n["level"] = LogLevel::ToString(i.level);
                }
                if (i.formatter.empty())
                {
                    n["formatter"] = i.formatter;
                }
                for (auto &a : i.appenders)
                {
                    YAML::Node na;
                    if (a.type == 1)
                    {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    }
                    else if (a.type == 2)
                    {
                        na["type"] = "StdoutLogAppender";
                    }
                    if (a.level != LogLevel::UNKOWN)
                    {
                        na["level"] = LogLevel::ToString(a.level);
                    }

                    if (!a.formatter.empty())
                    {
                        na["formatter"] = a.formatter;
                    }
                    n["appenders"].push_back(na);
                }
                node.push_back(n);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    sake::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = sake::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
    struct LogIniter
    {
        LogIniter()
        {
            g_log_defines->addListener([](const std::set<LogDefine> &old_value, const std::set<LogDefine> &new_value)
                                       {
                std::cout << "on_logger_conf_changed" << std::endl;
                //新增
                for (auto &i : new_value)
                {
                    auto it = old_value.find(i);
                    sake::Logger::ptr logger;
                    if (it == old_value.end())
                    {
                        //新增logger
                        logger = SAKE_LOG_NAME(i.name);
                    }
                    else{
                        //新的和旧的logger都有，判断是否变化
                        if(!(i == *it) ){
                            //修改的logger
                            logger = SAKE_LOG_NAME(i.name);
                        }
                    }
                    logger->setLevel(i.level);
                    if(!i.formatter.empty()){
                        logger->setLogFormatter(i.formatter);
                    }
                    logger->clearAppenders();
                    for (auto &j : i.appenders)
                    {
                        sake::LogAppender::ptr ap;
                        if (j.type == 1)
                        {
                            ap.reset(new FileLogAppender(j.file));
                        }
                        else if(j.type == 2){
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(j.level);
                        if(!j.formatter.empty()){
                            LogFormatter::ptr fmt(new LogFormatter(j.formatter));
                            if(!fmt->isError()){
                                ap->setFormater(fmt);
                            }
                            else{
                                std::cout <<"log.name = " << i.name << "appender formatter type = " << j.type
                                          << " formatter = " << j.formatter << " is invalid" << std::endl;
                            }
                        }
                        logger->addAppender(ap);
                    }
                }

                //删除
                for (auto &i : old_value){
                    auto it = new_value.find(i);
                    if(it == new_value.end()){
                        //删除logger(不是真的删除，因为有static初始化,所以设置日志级别高)
                        auto logger = SAKE_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)100);
                        logger->clearAppenders();
                    }
                } });
        }
    };
    static LogIniter __log__init;
    std::string LoggerManager::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for (auto &i : m_loggers)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    void LoggerManager::init()
    {
    }

}
