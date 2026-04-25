#include "argument-parse.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string strip_prefix(const std::string &name, const std::string &prefix)
{
    if (name.rfind(prefix, 0) == 0)
        return name.substr(prefix.size());
    return name;
}

bool rename_with_conflict(const fs::path &old_path, const std::string &new_name,
                          bool force, bool skip)
{
    fs::path new_path = old_path.parent_path() / new_name;
    if (!fs::exists(new_path))
    {
        fs::rename(old_path, new_path);
        std::cout << "Renamed: " << old_path.filename().string() << " -> " << new_name << '\n';
        return true;
    }

    if (force)
    {
        fs::remove(new_path);
        fs::rename(old_path, new_path);
        std::cout << "Overwrote: " << old_path.filename().string() << " -> " << new_name << '\n';
        return true;
    }
    if (skip)
    {
        std::cout << "Skipped: " << old_path.filename().string() << " (target exists)\n";
        return false;
    }
    // 询问模式（默认）
    while (true)
    {
        std::cout << "Conflict: " << old_path.filename().string() << " -> " << new_name << " exists.\n"
                  << "Enter new name, 'skip', or 'overwrite': ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "skip")
            return false;
        if (input == "overwrite")
        {
            fs::remove(new_path);
            fs::rename(old_path, new_path);
            std::cout << "Overwrote.\n";
            return true;
        }
        if (!input.empty())
        {
            fs::path cand = old_path.parent_path() / input;
            if (!fs::exists(cand))
            {
                fs::rename(old_path, cand);
                std::cout << "Renamed to: " << input << '\n';
                return true;
            }
            std::cout << "Name also exists. Try again.\n";
        }
    }
}

int main(int argc, char *argv[])
{
    argv_verify parser(argv[0], true, "Remove prefix from filenames");

    // 给所有选项都加上非空默认值，避免被视为必需参数
    // -d 有默认值 "."
    // -p 是必需的，所以默认值留空（或者随便给一个，然后单独检查），但用户要求前缀必填，所以保留空字符串让它成为必需
    // -f 和 -s 给一个假的默认值 "false"，通过值是否为空字符串来判断用户是否显式提供
    parser.append("-d", "--dir", "Target directory", ".");
    parser.append("-p", "--prefix", "Prefix to remove (required)", "", {}); // 必需
    parser.append("-f", "--force", "Overwrite conflicts without asking", "false", {"-s"});
    parser.append("-s", "--skip", "Automatically skip conflicting files", "false", {"-f"});
    // 不注册 -n，默认就是询问模式

    auto args = parser.verify(argv, argc);

    std::string prefix = args["-p"];
    if (prefix.empty())
    {
        std::cerr << "Error: --prefix is required.\n";
        return 1;
    }

    fs::path dir = args["-d"];
    if (!fs::exists(dir) || !fs::is_directory(dir))
    {
        std::cerr << "Error: Directory '" << dir.string() << "' invalid.\n";
        return 1;
    }

    // 判断用户是否提供了 -f 或 -s：如果参数值为空字符串，说明用户显式写了该标志
    bool force = (args.find("-f") != args.end() && args.at("-f").empty());
    bool skip = (args.find("-s") != args.end() && args.at("-s").empty());

    // 如果同时提供了 -f 和 -s，库的 collide 会阻止（会报错退出），这里不需要额外处理

    int count = 0;
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file())
            continue;
        std::string old = entry.path().filename().string();
        std::string newname = strip_prefix(old, prefix);
        if (newname == old)
            continue;
        if (newname.empty())
        {
            std::cerr << "Warning: '" << old << "' becomes empty, skipped.\n";
            continue;
        }
        if (rename_with_conflict(entry.path(), newname, force, skip))
            ++count;
    }

    std::cout << "Done. Renamed " << count << " files.\n";
    return 0;
}