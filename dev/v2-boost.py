#!/usr/bin/python3

import os
from pathlib import Path
import re
from subprocess import check_call
import subprocess
import sys

BOOST_NAME = "openmethod"
BOOST_INCLUDE = Path("include", "boost", BOOST_NAME)
YOMM2_INCLUDE = Path("include", "yorel", "yomm2")
YOMM2_TESTS = Path("tests")
SKIP = "cute keywords benchmark pss1 lab manual_call generator".split()
REPLACE = (
    ("YOREL_YOMM2_", f"BOOST_{BOOST_NAME.upper()}_"),
    ("#include <yorel/yomm2/", f"#include <boost/{BOOST_NAME}/"),
    ("<boost/openmethod/keywords.hpp>", "<boost/openmethod.hpp>"),
    ("yorel", "boost"),
    ("yomm2", "openmethod"),
    ("YOMM2_DECLARE|declare_method", "BOOST_OPENMETHOD"),
    ("YOMM2_DEFINE|define_method", "BOOST_OPENMETHOD_OVERRIDE"),
    ("YOMM2_CLASS(ES)?|register_class(es)?", "BOOST_OPENMETHOD_CLASSES"),
)

modified = subprocess.check_output(
    "git diff --name-only include/boost test".split(), encoding="ascii"
)

if modified:
    sys.exit("Output directories have unstaged changes:\n" + modified)


def skip(path):
    for skip in SKIP:
        if skip in str(path):
            return True

    return False


for from_path in list(YOMM2_INCLUDE.rglob("*.hpp")) + list(YOMM2_TESTS.rglob("*.?pp")):
    if skip(from_path):
        continue

    with from_path.open() as f:
        content = f.read()

    content = re.sub(r'#include "(yorel/yomm2/[^"])+"', r"#include <\1>", content)

    for replace in REPLACE:
        content = re.sub(*replace, content)

    to_path = Path(
        str(from_path)
        .replace("yorel/yomm2", f"boost/{BOOST_NAME}")
        .replace("tests/", "test/")
    )

    print(f"{from_path} -> {to_path}...", end="")

    to_path.parent.mkdir(parents=True, exist_ok=True)

    with to_path.open("w") as f:
        f.write(content)

    check_call(["clang-format", "-i", str(to_path)])

    print("done")
