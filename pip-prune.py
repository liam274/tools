#!/usr/bin/env python3
"""交互式清理 pip 孤儿包。"""

import subprocess
import sys
import json

def get_orphan_packages():
    """使用 pipdeptree 获取所有未被依赖的包（孤儿包）。"""
    result = subprocess.run(
        ["pipdeptree", "--json-tree"],
        capture_output=True, text=True, check=True
    )
    tree = json.loads(result.stdout)
    # 收集所有作为“依赖项”出现的包
    depended_on = set()
    for pkg in tree:
        for dep in pkg.get("dependencies", []):
            depended_on.add(dep["key"])
    # 孤儿包 = 所有包 - 被依赖的包
    all_packages = {pkg["key"]: pkg["package_name"] for pkg in tree}
    orphans = {
        key: name for key, name in all_packages.items()
        if key not in depended_on
    }
    return orphans

def main():
    orphans = get_orphan_packages()
    if not orphans:
        print("✅ 没有发现孤儿包。")
        return

    print(f"发现 {len(orphans)} 个孤儿包：\n")
    for i, (key, name) in enumerate(sorted(orphans.items()), 1):
        print(f"  {i:3}. {name} ({key})")

    print("\n请输入要卸载的包的编号（空格分隔），或输入 'all' 卸载全部，'q' 退出：")
    choice = input("> ").strip().lower()

    if choice == "q":
        print("已取消。")
        return

    if choice == "all":
        to_remove = list(orphans.values())
    else:
        try:
            indices = [int(x) for x in choice.split()]
            to_remove = [list(sorted(orphans.values()))[i - 1] for i in indices]
        except (ValueError, IndexError):
            print("❌ 输入无效。")
            sys.exit(1)

    if not to_remove:
        print("未选择任何包。")
        return

    print(f"\n即将卸载：{', '.join(to_remove)}")
    if input("确认？(y/N) ").lower() != "y":
        print("已取消。")
        return

    subprocess.run([sys.executable, "-m", "pip", "uninstall", "-y"] + to_remove)
    print("✅ 清理完成。")

if __name__ == "__main__":
    main()
