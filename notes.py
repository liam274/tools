import sys
import os
from typing import Iterator
import re

path_join = os.path.join


def shorten_path(path: str) -> str:
    """
    Replace the user's home directory prefix with '~' if the given path
    is inside or equal to the home directory.

    Args:
        path: Absolute or relative path.

    Returns:
        Path with home replaced by '~' if applicable, otherwise the
        normalized absolute path.
    """
    home = os.path.expanduser("~")
    abs_path = os.path.abspath(os.path.expanduser(path))
    home = os.path.normpath(home)
    if abs_path == home:
        return "~"
    if abs_path.startswith(home + os.sep):
        return "~" + abs_path[len(home) :]
    return abs_path


def parser(path: str) -> dict[str, dict[str, list[str]]]:
    if os.path.exists(path) and os.path.isfile(path):
        pass
    else:
        print(f'Error: File "{shorten_path(path)}" does not exist or is not a file')
        sys.exit(1)
    iterator: Iterator[str]
    with open(path, "r") as file:
        iterator = (i for i in file.readlines())
    result: dict[str, dict[str, list[str]]] = {}
    section_title: str = ""
    fragment_title: str = ""
    in_section: bool = False
    fragment: list[str] = []
    frags: dict[str, list[str]] = {}
    for line in iterator:
        line = line.strip()
        if in_section:
            if line.startswith("#"):
                if fragment:
                    frags.update({fragment_title: fragment})
                    fragment = []
                fragment_title = line.lstrip("#").strip()
            elif line:
                fragment.append(line + "\n")
            else:
                frags.update({fragment_title: fragment})
                fragment = []
                in_section = False
        else:
            if line.startswith("#"):
                if section_title and frags:
                    result.update({section_title: frags})
                    frags = {}
                    fragment = []
                    fragment_title = ""
                section_title = line.lstrip("#").strip()
                fragment_title = "".join(i for i in section_title)
                in_section = True
    if fragment:
        frags.update({fragment_title: fragment})
    if frags:
        result.update({section_title: frags})
    return result


def has(data: str, find: str, escape: str) -> int:
    skip: bool = False
    i: int = -1
    for char in data:
        i += 1
        if skip:
            if char in find:
                return i
            skip = False
            continue
        if char in escape:
            skip = True
        if char in find:
            return i
    return 0


class args_parser:
    def __init__(self):
        self.required_args: set[tuple[str, str, str, str]] = set()
        self.maxs: int = 0
        self.namelist: set[str] = set()

    def append(
        self, arg_name: str, alias: str, description: str, default_value: str = ""
    ):
        self.required_args.add((arg_name, alias, description, default_value))
        self.namelist.add(arg_name)
        self.namelist.add(alias)
        if len(arg_name) + len(alias) > self.maxs:
            self.maxs = len(arg_name) + len(alias)

    def find(self, value: str) -> tuple[str, str, str, str]:
        for i in self.required_args:
            if i[0] == value or i[1] == value:
                return i
        return ("", "", "", "")

    def apply(self, args: list[str]) -> dict[str, str]:
        result: dict[str, str] = {}
        is_arg: bool = True
        arg_name: str = ""
        for arg in args:
            is_arg = not is_arg
            if is_arg:
                result.update({arg_name: arg})
                continue
            index: int = has(arg, "=", "\\")
            if index:
                result.update({arg[:index]: arg[index + 1 :]})
                is_arg = False
            else:
                arg_name = arg
                result.update({arg_name: ""})
        if "-h" in result or "--help" in result:
            value: str = result.get("-h", result.get("--help", ""))
            maxs: int = self.maxs + 6
            if value in self.namelist:
                arg_name, alias, description, default_value = self.find(value)
                print(
                    f"{arg_name}, {alias}",
                    " " * (maxs - len(arg_name) - len(alias) - 1),
                    description,
                )
                print(" " * maxs, default_value)
            for arg_name, alias, description, default_value in self.required_args:
                print(
                    f"{arg_name}, {alias}",
                    " " * (maxs - len(arg_name) - len(alias) - 1),
                    description,
                )
                print(" " * (maxs + 2), default_value)
            sys.exit(0)
        for arg_name, alias, description, default_value in self.required_args:
            if default_value:
                result.update(
                    {arg_name: result.get(arg_name, result.get(alias, default_value))}
                )
            elif alias in result:
                result.update({arg_name: result[alias]})
            elif arg_name not in result:
                print(
                    f'Error: Missing argument "{arg_name}" or "{alias}", please run -h or --help for further information'
                )
                sys.exit(1)
        for arg_name in result:
            if arg_name not in self.namelist:
                print(
                    f'Error: Unknown argument "{arg_name}", please run -h or --help for further information'
                )
                sys.exit(1)
        return result


def welcome():
    print("Welcome to the world of Notes!")


YELLOW = "\033[33m"
RESET = "\033[0m"


def find(
    query: str, data: dict[str, dict[str, list[str]]]
) -> dict[str, dict[str, list[str]]]:
    result: dict[str, dict[str, list[str]]] = {}
    pattern: str = re.escape(query)

    def replacer(match: re.Match[str]) -> str:
        return f"{YELLOW}{match.group(0)}{RESET}"

    for key, value in data.items():
        if query in key:
            key = re.sub(pattern, replacer, key)
            result.update({key: value})
        ok: bool = False
        _temp: dict[str, list[str]] = {}
        for k, v in value.items():
            if query in k:
                ok = True
                break
            for i in v:
                if query in i or ok:
                    ok = True
                    break
            else:
                continue
            break
        if ok:
            for k, v in value.items():
                _temp.update(
                    {
                        re.sub(pattern, replacer, k): [
                            re.sub(pattern, replacer, i) for i in v
                        ]
                    }
                )
            result.update({key: _temp})
    for _, __ in result.items():
        for key, value in __.items():
            temp: list[str] = []
            for i in value:
                temp.append(re.sub(pattern, replacer, i))
            __.update({key: temp})
        result.update({_: __})
    return result


def main():
    parse: args_parser = args_parser()
    parse.append(
        "-f",
        "--file",
        "The file that should be see as note.",
        path_join(os.getcwd(), "skills"),
    )
    parse.append("-s", "--search", "Search for something")
    args: dict[str, str] = parse.apply(sys.argv[1:])
    tree: dict[str, dict[str, list[str]]] = parser(args["-f"])
    result: dict[str, dict[str, list[str]]] = find(args["-s"], tree)
    for key, value in result.items():
        print(key + ":", end="")
        print(
            "\n\t"
            + "\t".join(
                f"{k}:\n\t\t{"\t\t".join(v)}" for k, v in value.items()  # type: ignore
            )
        )


if __name__ == "__main__":
    welcome()
    main()
