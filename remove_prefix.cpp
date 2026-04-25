#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "argument-parse.h"

namespace fs = std::filesystem;

// 去除文件名前缀
std::string strip_prefix(const std::string &filename, const std::string &prefix)
{
    if (filename.rfind(prefix, 0) == 0)
        return filename.substr(prefix.size());
    return filename;
}

// 处理文件重命名（带冲突解决）
bool rename_file(const fs::path &old_path, const std::string &new_name,
                 bool force, bool auto_skip, bool interactive)
{
    fs::path new_path = old_path.parent_path() / new_name;
    if (!fs::exists(new_path))
    {
        fs::rename(old_path, new_path);
        std::cout << "Renamed: " << old_path.filename().string() << " -> " << new_name << "\n";
        return true;
    }

    // 冲突处理
    if (force)
    {
        fs::remove(new_path);
        fs::rename(old_path, new_path);
        std::cout << "Overwrote: " << old_path.filename().string() << " -> " << new_name << "\n";
        return true;
    }
    if (auto_skip)
    {
        std::cout << "Skipped: " << old_path.filename().string() << " (target " << new_name << " exists)\n";
        return false;
    }
    // interactive mode (default when no -f/-s)
    while (true)
    {
        std::cout << "Conflict: " << old_path.filename().string() << " -> " << new_name << " already exists.\n"
                  << "Enter new name, or 'skip', or 'overwrite': ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "skip")
        {
            std::cout << "Skipped.\n";
            return false;
        }
        if (input == "overwrite")
        {
            fs::remove(new_path);
            fs::rename(old_path, new_path);
            std::cout << "Overwrote.\n";
            return true;
        }
        if (!input.empty())
        {
            fs::path candidate = old_path.parent_path() / input;
            if (!fs::exists(candidate))
            {
                fs::rename(old_path, candidate);
                std::cout << "Renamed to: " << input << "\n";
                return true;
            }
            else
            {
                std::cout << "That name also exists. Try again.\n";
            }
        }
    }
}

int main(int argc, char *argv[])
{
    argv_verify parser(argv[0], true, "Remove prefix from filenames in a directory");

    // 定义参数（利用默认值和冲突检测）
    parser.append("-d", "--dir", "Target directory (default: current directory)", ".");
    parser.append("-p", "--prefix", "Prefix to remove (required)", "", {});
    parser.append("-f", "--force", "Overwrite conflicting files without asking", "", {"-s", "-n"});
    parser.append("-s", "--skip", "Automatically skip conflicting files", "", {"-f", "-n"});
    parser.append("-n", "--ask", "Ask for new name on conflict (default behavior)", "", {"-f", "-s"});

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
        std::cerr << "Error: Directory '" << dir.string() << "' does not exist.\n";
        return 1;
    }

    bool force_mode = args.find("-f") != args.end();
    bool skip_mode = args.find("-s") != args.end();
    bool ask_mode = args.find("-n") != args.end() || (!force_mode && !skip_mode); // 默认询问

    int count = 0;
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file())
            continue;
        std::string old_name = entry.path().filename().string();
        std::string new_name = strip_prefix(old_name, prefix);
        if (new_name == old_name)
            continue; // 前缀不匹配
        if (new_name.empty())
        {
            std::cerr << "Warning: '" << old_name << "' becomes empty name, skipped.\n";
            continue;
        }
        if (rename_file(entry.path(), new_name, force_mode, skip_mode, ask_mode))
            ++count;
    }

    std::cout << "Done. " << count << " files renamed.\n";
    return 0;
}