import json
from pathlib import Path
import re
import sys


def split_list(text):
    return re.split(" *, *", text)


hrefs = {"home": "/README.md", "reference": "/reference/README.md"}

cpps = set()
for ref in (Path(__file__).parent.parent / "reference.in").iterdir():
    if ref.suffix == ".cpp" or (ref.suffix == ".md" and ref.stem not in cpps):
        cpps.add(ref.stem)
        md_path = ref.with_suffix(".md").name.replace("reference.in", "reference")
        hrefs[ref.stem] = md_path
        with open(ref) as rh:
            for text in rh.readlines():
                if "---" in text:
                    break
                m = re.match(r"^entry: +(.*)", text)
                if m:
                    for symbol in split_list(m[1]):
                        symbol = symbol.strip().replace("yorel::yomm2::", "")
                        if symbol != ref.stem:
                            hrefs[symbol] = md_path
                if m := re.search(r"hrefs: *([\w_-]+)", text):
                    for href in split_list(m.group(1)):
                        hrefs[href] = md_path

with open(".hrefs", "w", encoding="ascii") as fh:
    json.dump(hrefs, fh, indent=4)


def replace_links(text, **kwargs):
    def sub(m):
        text = m.group(1)
        symbol = text.replace("`", "")
        target = hrefs.get(symbol)
        if target is None:
            print("BROKEN:", symbol, file=sys.stderr)
            symbol = f"{symbol} (BROKEN)"
        return f"[{text}]({target})"

    return re.sub(r"->(`?[\w_-]+`?)", sub, text, **kwargs)


def replace_md(text):
    text = re.sub(
        r"^entry: *(.*)",
        lambda m: ", ".join([f"**{entry}**" for entry in split_list(m[1])]) + "<br>",
        text,
        flags=re.MULTILINE,
    )
    text = re.sub(
        r"^experimental: *(.*)",
        lambda m: ", ".join([f"**{entry}**" for entry in split_list(m[1])])
        + " <small>(experimental)</small><br>",
        text,
        flags=re.MULTILINE,
    )
    text = re.sub(r"^hrefs:.*$", "", text, flags=re.MULTILINE)

    def headers(m):
        first, *others = split_list(m[1])
        segments = [f"defined in <{first}>"]

        if len(others) > 0:
            segments.append(", also provided by")
            segments.append(", ".join([f"<{header}>" for header in others]))

        return "".join(
            (
                "<sub>",
                *segments,
                "</sub>",
            )
        )

    text = re.sub(
        r"^headers: *(.*)",
        headers,
        text,
        flags=re.MULTILINE,
    )

    text = replace_links(text)

    return text
