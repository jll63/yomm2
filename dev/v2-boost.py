#!/usr/bin/python3

import os
from pathlib import Path
import re
from subprocess import check_call
import subprocess
import sys

BOOST_NAME = "openmethod"
BOOST_INCLUDE = Path("include", "boost", BOOST_NAME)
BOOST_OPENMETHOD_PATH = Path.home().joinpath("dev", "boost", "libs", BOOST_NAME)
YOMM2_V2_PATH = Path.home().joinpath("dev", "yomm2")
YOMM2_DIRS = "include/yorel/yomm2 tests examples ce".split()
SKIP = [
    "cute",
    "keywords",
    "benchmark",
    "pss1",
    "lab",
    "manual_call",
    "generator",
    "containers",
    "conan",
    "vcpkg",
    "README",
    "cmakeyomm2",
    "templates",
]
REPLACE = (
    ("YOREL_YOMM2_", f"BOOST_{BOOST_NAME.upper()}_"),
    ("YOMM2_", f"BOOST_{BOOST_NAME.upper()}_"),
    ("yOMM2_", f"BOOST_{BOOST_NAME.upper()}_DETAIL_"),
    ("BOOST_OPENMETHOD_METHOD", "BOOST_OPENMETHOD"),
    ("#include <yorel/yomm2/", f"#include <boost/{BOOST_NAME}/"),
    ("<boost/openmethod/keywords.hpp>", "<boost/openmethod.hpp>"),
    ("yorel", "boost"),
    ("yomm2", "openmethod"),
    ("YOMM2_DECLARE|declare_method", "BOOST_OPENMETHOD"),
    ("YOMM2_DEFINE|define_method", "BOOST_OPENMETHOD_OVERRIDE"),
    ("YOMM2_CLASS(ES)?|register_class(es)?", "BOOST_OPENMETHOD_CLASSES"),
)

git = f"git diff --name-only -- boost test examples"
print(git)

modified = subprocess.check_output(git.split(), encoding="ascii")

if modified:
    sys.exit("Unstaged changes:\n" + modified)

git = f"git -C {YOMM2_V2_PATH} rev-parse --abbrev-ref HEAD"
print(git)

version = subprocess.check_output(git.split(), encoding="ascii").strip()

if version != "v2":
    sys.exit(f"Source directory is at version {version}, v2 is required.\n")


def skip(path):
    for skip in SKIP:
        if skip in str(path):
            return True

    return False


for dir in YOMM2_DIRS:
    for from_path in (YOMM2_V2_PATH / dir).rglob("*.?pp"):
        if skip(from_path):
            continue

        with from_path.open() as f:
            content = f.read()

        content = re.sub(r'#include "(yorel/yomm2/[^"])+"', r"#include <\1>", content)

        for replace in REPLACE:
            content = re.sub(*replace, content)

        to_path = Path(
            str(from_path)
            .replace(str(YOMM2_V2_PATH), str(BOOST_OPENMETHOD_PATH))
            .replace("yorel/yomm2", f"boost/{BOOST_NAME}")
            .replace("tests/", "test/")
        )

        print(f"{from_path} -> {to_path}...")

        to_path.parent.mkdir(parents=True, exist_ok=True)

        with to_path.open("w") as f:
            f.write(content)

        check_call(["clang-format", "-i", str(to_path)])

print("done")
