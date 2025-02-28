#pragma once

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <string>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "log.h"
#include "thread.h"

namespace sake
{
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string name, const std::string description = "")
            : m_name(name),
              m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}

        const std::string getName() const { return m_name; }
        const std::string getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string val) = 0;
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };
    // F from_type,T to_type
    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &from_type) { return boost::lexical_cast<T>(from_type); }
    };
    // string to vector
    template <class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    // vector to string
    template <class T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to list
    template <class T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> li;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                li.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return li;
        }
    };

    // list to string
    template <class T>
    class LexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                node.push_back(YAML::Node(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to set
    template <class T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> s;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                s.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return s;
        }
    };

    // set to string
    template <class T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                node.push_back(YAML::Node(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to unordered_set
    template <class T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> s;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                s.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return s;
        }
    };

    // unordered_set to string
    template <class T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                node.push_back(YAML::Node(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to map
    template <class T>
    class LexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> mp;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                mp.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    // map to string
    template <class T>
    class LexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T> &v)
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &i : v)
            {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to unordered_map
    template <class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> mp;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                mp.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    // unordered_map to string
    template <class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                node[i.first] = YAML::Node(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 复杂类型序列化和反序列化clas FromStr,class ToStr
    // FromStr T operator()(const string&)
    // ToStr string operator()(const T&)
    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef RWMutex RWMutexType;
        typedef std::shared_ptr<ConfigVar> ptr;
        // 当配置文件更改回调
        typedef std::function<void(const T &old_value, const T &new_value)> on_changed_cb;

        ConfigVar(const std::string &name, const T &default_val, const std::string description = "")
            : ConfigVarBase(name, description),
              m_val(default_val) {}

        std::string toString() override
        {
            try
            {
                // return boost::lexical_cast<std::string>(m_val);
                RWMutex::ReadLock lock(m_mutex);
                return ToStr()(m_val);
            }
            catch (const std::exception &e)
            {
                SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << "convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

        bool fromString(const std::string val) override
        {
            try
            {
                // m_val = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
            }
            catch (const std::exception &e)
            {
                SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "ConfigVar::fromString exception"
                                                << e.what() << "convert: string to "
                                                << typeid(m_val).name()
                                                << " name = " << m_name
                                                << " - " << val;
            }
            return false;
        }
        const T getValue()
        {
            RWMutex::ReadLock lock(m_mutex);
            return m_val;
        }

        void setValue(const T &v)
        {
            // 值没有更改
            {
                RWMutex::ReadLock lock(m_mutex);
                if (v == m_val)
                {
                    return;
                }
                for (auto &i : m_cbs)
                {
                    i.second(m_val, v);
                }
            }
            RWMutex::WriteLock lock(m_mutex);
            m_val = v;
        }

        std::string getTypeName() const override { return typeid(T).name(); }

        uint64_t addListener(on_changed_cb cb)
        {
            static uint64_t s_fun_id = 0;
            RWMutex::WriteLock lock(m_mutex);
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }

        void delListener(uint64_t key)
        {
            RWMutex::WriteLock lock(m_mutex);
            m_cbs.erase[key];
        }

        void clearListener()
        {
            RWMutex::WriteLock lock(m_mutex);
            m_cbs.clear();
        }

        on_changed_cb getListener(uint64_t key)
        {
            RWMutex::ReadLock lock(m_mutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

    private:
        RWMutexType m_mutex;
        T m_val;
        // 变更回调函数组，uint64_t key要求唯一，一般使用hash
        std::map<uint64_t, on_changed_cb> m_cbs;
    };

    class Config
    {
    public:
        typedef RWMutex RWMutexType;
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &default_val, const std::string &description = "")
        {
            RWMutex::WriteLock lock(GetMutex());
            auto it = GetDatas().find(name);
            if (it != GetDatas().end())
            {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if (tmp)
                {
                    SAKE_LOG_INFO(SAKE_LOG_ROOT()) << "Lookup name = " << name << " exists";
                    return tmp;
                }
                else
                {
                    SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "Lookup name = " << name << " exists but type not "
                                                    << typeid(T).name() << " real_type = " << it->second->getTypeName()
                                                    << " value = " << it->second->toString();
                    return nullptr;
                }
            }
            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "Lookup name is invalid: " << name;
                throw std::invalid_argument(name);
            }
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_val, description));
            GetDatas()[name] = v;
            return v;
        }

        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            RWMutex::ReadLock lock(GetMutex());
            auto it = GetDatas().find(name);
            if (it == GetDatas().end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        static void LoadFromYaml(const YAML::Node &root);
        static ConfigVarBase::ptr Lookup(const std::string &name);

        static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

    private:
        static ConfigVarMap &GetDatas()
        {
            static ConfigVarMap s_datas;
            return s_datas;
        }

        // 静态变量初始化顺序不一定，所以得先初始化锁吗，防止后面锁的变量先初始化了
        static RWMutexType &GetMutex()
        {
            static RWMutexType s_mutex;
            return s_mutex;
        }
    };
}
