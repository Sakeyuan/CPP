#include "config.h"
#include "log.h"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <iostream>
// 约定配置参数
#if 0
sake::ConfigVar<int>::ptr g_int_value_config = sake::Config::Lookup("system.port", (int)8080, "system port");
sake::ConfigVar<float>::ptr g_int_valuex_config = sake::Config::Lookup("system.port", (float)8080, "system port");
sake::ConfigVar<float>::ptr g_float_value_config = sake::Config::Lookup("system.value", (float)10.2f, "system value");
sake::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = sake::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");
sake::ConfigVar<std::list<int>>::ptr g_int_list_value_config = sake::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");
sake::ConfigVar<std::set<int>>::ptr g_int_set_value_config = sake::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");
sake::ConfigVar<std::unordered_set<int>>::ptr g_int_unordered_set_value_config = sake::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{1, 2}, "system int unordered_set");
sake::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config = sake::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map");
sake::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_unordered_map_value_config = sake::Config::Lookup("system.str_int_unordered_map", std::unordered_map<std::string, int>{{"k", 2}}, "system str int unordered_map");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar()
                             << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar()
                             << "NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
                                 << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
                                 << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node node = YAML::LoadFile("/home/yjf/CPP/Config/test.yml");
    print_yaml(node, 0);
}

void test_conifg()
{
    LOG_INFO(LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    LOG_INFO(LOG_ROOT()) << "before: " << g_float_value_config->toString();
#define XX(g_var, name, prefix)                                                      \
    {                                                                                \
        auto &vec = g_var->getValue();                                               \
        for (auto &i : vec)                                                          \
        {                                                                            \
            LOG_INFO(LOG_ROOT()) << #prefix << ": " #name << ": " << i;              \
        }                                                                            \
        LOG_INFO(LOG_ROOT()) << #prefix << ": " #name "yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix)                                                             \
    {                                                                                         \
        auto &vec = g_var->getValue();                                                        \
        for (auto &i : vec)                                                                   \
        {                                                                                     \
            LOG_INFO(LOG_ROOT()) << #prefix << ": " #name ": {" << i.first << "-" << i.second \
                                 << "}";                                                      \
        }                                                                                     \
        LOG_INFO(LOG_ROOT()) << #prefix << ": " #name "  yaml: " << g_var->toString();        \
    }
    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_unordered_set_value_config, int_unordered_set, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_unordered_map_value_config, str_int_unordered_map, before);

    YAML::Node node = YAML::LoadFile("/home/yjf/CPP/Config/test.yml");
    sake::Config::LoadFromYaml(node);

    LOG_INFO(LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    LOG_INFO(LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_unordered_set_value_config, int_unordered_set, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_unordered_map_value_config, str_int_unordered_map, after);
}
#endif

class Persion
{
public:
    Persion() {}

public:
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "{ Persion name = " << m_name
           << " age = " << m_age
           << " sex = " << m_age
           << " }";
        return ss.str();
    }
    bool operator==(const Persion &oth) const
    {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
    }
};

namespace sake
{
    // string to persion
    template <>
    class LexicalCast<std::string, Persion>
    {
    public:
        Persion operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            Persion p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            return p;
        }
    };

    // persion to string
    template <>
    class LexicalCast<Persion, std::string>
    {
    public:
        std::string operator()(const Persion &p)
        {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

sake::ConfigVar<Persion>::ptr g_persion = sake::Config::Lookup("class.persion", Persion(), "class persion");
sake::ConfigVar<std::map<std::string, Persion>>::ptr g_persion_map = sake::Config::Lookup("class.map", std::map<std::string, Persion>(), "class map");
sake::ConfigVar<std::map<std::string, std::vector<Persion>>>::ptr g_person_vec_map =
    sake::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Persion>>(), "system person");
void test_class()
{
#define XX_PM(g_var, prefix)                                                                          \
    {                                                                                                 \
        auto m = g_persion_map->getValue();                                                           \
        for (auto &i : m)                                                                             \
        {                                                                                             \
            SAKE_LOG_INFO(SAKE_LOG_ROOT()) << prefix ": " << i.first << " - " << i.second.toString(); \
        }                                                                                             \
        SAKE_LOG_INFO(SAKE_LOG_ROOT()) << prefix ": "                                                 \
                                       << "size = " << m.size();                                      \
    }
    g_persion->addListener([](const Persion &old_value, const Persion &new_value)
                           { SAKE_LOG_INFO(SAKE_LOG_ROOT()) << "old_value = " << old_value.toString() << " new_value = " << new_value.toString(); });

    SAKE_LOG_INFO(SAKE_LOG_ROOT()) << "before: " << g_persion->getValue().toString();

    YAML::Node node = YAML::LoadFile("/home/yjf/LunixCPP/Config/logs.yml");
    sake::Config::LoadFromYaml(node);

    SAKE_LOG_INFO(SAKE_LOG_ROOT()) << "after: " << g_persion->getValue().toString();
}

void test_log()
{
    sake::Logger::ptr system_logger = SAKE_LOG_NAME("system");
    SAKE_LOG_INFO(system_logger) << "hello system" << std::endl;
    std::cout << sake::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node node = YAML::LoadFile("/home/yjf/LunixCPP/Config/logs.yml");
    sake::Config::LoadFromYaml(node);
    std::cout << "=========================" << std::endl;
    std::cout << sake::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    SAKE_LOG_INFO(system_logger) << "hello system" << std::endl;
    system_logger->setLogFormatter("%d - %m%n");
    SAKE_LOG_INFO(system_logger) << "hello system" << std::endl;
}

int main(int argc, char **argv)
{
    test_log();
    sake::Config::Visit([](sake::ConfigVarBase::ptr var)
                        { SAKE_LOG_INFO(SAKE_LOG_ROOT()) << " name = " << var->getName()
                                                         << " description = " << var->getDescription()
                                                         << " typename = " << var->getTypeName()
                                                         << " value = " << var->toString(); });
    return 0;
}