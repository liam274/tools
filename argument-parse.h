#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdlib>

inline std::vector<std::string> spesplit(const std::string &data, char deli)
{
    std::vector<std::string> result = {};
    std::vector<char> temp = {};
    for (char chr : data)
    {
        if (chr == deli)
        {
            std::string _(temp.begin(), temp.end());
            result.push_back(_);
            temp.clear();
        }
        else
        {
            temp.push_back(chr);
        }
    }
    if (temp.size())
    {
        std::string _(temp.begin(), temp.end());
        result.push_back(_);
    }
    return result;
}

template <typename type>
inline std::string join(const type &strings, const std::string &joiner)
{
    if (strings.empty())
    {
        return "";
    }
    std::size_t total_len = 0;
    for (const auto &s : strings)
    {
        total_len += s.size();
    }
    std::string result;
    result.reserve(total_len + joiner.size() * (strings.size() - 1));
    int i = 0;
    for (const auto &s : strings)
    {
        result.append(s);
        if (i != strings.size() - 1)
        {
            result.append(joiner);
        }
        i++;
    }
    return result;
}

namespace argument
{
    struct details
    {
        std::string help;
        std::string default_value;
        std::string alias;
        std::vector<std::string> collide;
    };
};

class argv_verify
{
private:
    std::unordered_map<std::string, argument::details> rules;
    std::string name;
    std::string description;
    bool _strict;
    std::vector<std::string> org;

public:
    argv_verify(const std::string &file_name, bool strict = true, const std::string &doc = "is a CPP application")
    {
        name = file_name;
        description = doc;
        _strict = strict;
    }
    void append(const std::string &flag_name, const std::string &alias, const std::string &help,
                const std::string &_default = "", const std::vector<std::string> &collide = {})
    {
        org.push_back(flag_name);
        argument::details details;
        details.help = help;
        details.default_value = _default;
        details.alias = alias;
        details.collide = collide;
        rules[flag_name] = details;
        details.alias = flag_name;
        rules[alias] = details;
    }
    void descriptionPrinter() const
    {
        std::cout << name << " " << description << std::endl;
    }
    /**
     * @brief
     * @param data
     * @param argc
     * @return
     */
    std::unordered_map<std::string, std::string> verify(char *data[], const int argc) const
    {
        std::unordered_map<std::string, std::string> flags = {};
        bool is_flag = true;
        std::string flag_name;
        for (int t = 1; t < argc; t++)
        {
            if (data[t][0] == '-')
            {
                is_flag = true;
            }
            if (is_flag)
            {
                flag_name = data[t];
                if (flags.find(flag_name) != flags.end())
                {
                    if (_strict)
                    {
                        std::cerr << "Error: Flag \"" << flag_name << "\" repeated" << std::endl;
                        exit(-2);
                    }
                    else
                    {
                        std::cerr << "Warning: Flag \"" << flag_name << "\" repeated. Abandon the value, keep the first value as default." << std::endl;
                    }
                }
                else if (flag_name.find("=") == std::string::npos)
                {
                    flags[flag_name] = "";
                }
                else
                {
                    std::vector<std::string> vec = spesplit(flag_name, '='),
                                             sub(vec.begin() + 1, vec.end());
                    flags[vec[0]] = join(sub, "=");
                    is_flag = false;
                }
            }
            else
            {
                flags[flag_name] = data[t];
            }
            is_flag = !is_flag;
        }
        std::vector<std::string> temp = {};
        if (flags.find("-h") != flags.end() || flags.find("--help") != flags.end())
        {
            descriptionPrinter();
            int maxl = 0;
            for (const auto &[key, value] : rules)
            {
                if (std::find(temp.begin(), temp.end(), key) != temp.end())
                {
                    continue;
                }
                if (key.size() + value.alias.size() > maxl)
                {
                    maxl = static_cast<int>(key.size());
                }
                temp.push_back(value.alias);
            }
            temp.clear();
            for (const auto &[key, value] : rules)
            {
                if (std::find(temp.begin(), temp.end(), key) != temp.end())
                {
                    continue;
                }
                std::cout << "         " << key << " " << value.alias;
                std::string filler(5 + maxl - (key.size() + value.alias.size()), ' ');
                std::cout << filler << value.help << '\n';
                if (value.default_value.size())
                {
                    std::string filler2(maxl - 1, ' ');
                    std::cout << filler2 << "default value: " << value.default_value << '\n';
                }
                temp.push_back(value.alias);
            }
            std::exit(0);
        }
        std::vector<std::string> aliaed = {};
        for (const auto &[key, value] : rules)
        {
            if (std::find(aliaed.begin(), aliaed.end(), key) != aliaed.end())
            {
                continue;
            }
            if (flags.find(key) == flags.end() && flags.find(value.alias) == flags.end())
            {
                if (value.default_value.size())
                {
                    flags[key] = value.default_value;
                    aliaed.push_back(key);
                    aliaed.push_back(value.alias);
                }
                else
                {
                    descriptionPrinter();
                    std::cout << "Required flag \"" << key << "\" as argument. For more information, please use " << name << " -h or " << '\n';
                    std::string _(63 + key.size(), ' ');
                    std::cout << _ << name << " --help" << std::endl;
                    std::exit(-1);
                }
            }
            else
            {
                aliaed.push_back(key);
                aliaed.push_back(value.alias);
            }
            for (const std::string &collide : value.collide)
            {
                if (flags.find(collide) != flags.end())
                {
                    descriptionPrinter();
                    std::cout << "Flag \"" << key << "\" collides with flag \"" << collide << "\". For more information, please use " << name << " -h or " << '\n';
                    std::string _(63 + key.size(), ' ');
                    std::cout << _ << name << " --help" << std::endl;
                    std::exit(-1);
                }
            }
        }
        std::unordered_map<std::string, std::string> new_flags;
        for (const auto &[key, value] : flags)
        {
            if (rules.find(key) == rules.end())
            {
                if (_strict)
                {
                    std::cerr << "Error: Flag \"" << key << "\" not defined" << std::endl;
                    std::exit(-2);
                }
                else
                {
                    std::cerr << "Warning: Flag \"" << key << "\" not defined" << '\n';
                }
            }
            if (std::find(org.begin(), org.end(), key) == org.end())
            {
                new_flags[rules.at(key).alias] = value;
            }
            else
            {
                new_flags[key] = value;
            }
        }
        return new_flags;
    }
};