import html
import glob
import json
from pathlib import Path
import re
import sys


def split_list(text, sep=","):
    return list(map(str.strip, text.split(sep)))


hrefs = {"home": "/README.md", "reference": "/reference/README.md"}
in_path = Path(__file__).parent.parent / "reference.in"
in_files = list(
    Path(ref_str)
    for ref_str in sorted(
        glob.glob("**/*.md", root_dir=in_path, recursive=True)
        + glob.glob("**/*.cpp", root_dir=in_path, recursive=True),
        key=str.lower,
    )
)

for ref in in_files:
    md_path = str(ref.with_suffix(".md")).replace("reference.in", "reference")
    hrefs[str(ref.with_suffix(""))] = f"/reference/{md_path}"
    with open(in_path / ref) as rh:
        for text in rh.readlines():
            if "---" in text:
                break
            m = re.match(r"^entry: +(.*)", text)
            if m:
                for symbol in split_list(m[1]):
                    symbol = symbol.strip().replace("yorel::yomm2::", "")
                    if symbol != ref.stem:
                        hrefs[symbol] = f"/reference/{md_path}"
            if m := re.search(r"hrefs: *([\w_-]+)", text):
                for href in split_list(m.group(1)):
                    hrefs[href] = f"/reference/{md_path}"

with open(".hrefs", "w", encoding="ascii") as fh:
    json.dump(hrefs, fh, indent=4)


def replace_links(text):
    def sub(m):
        path = m.group(2)
        target = hrefs.get(path.replace("::", "/"))
        if target is None:
            print("POSSIBLY BROKEN:", path, file=sys.stderr)
            return f"->{path}"
        return f"[{m.group(1)}{path.split('/')[-1]}{m.group(3)}]({target})"

    return re.sub(r"->(`?)([/:\w_\-]+)(`?)", sub, text)


def replace_md(text: str, trail: list[str] = None):
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

    def location(m):
        locations = split_list(m[1], ";")
        assert len(locations) <= 2

        namespace = "yorel::yomm2"
        if len(locations) == 2:
            namespace = f"{namespace}::{locations.pop(0)}"

        locations = split_list(locations.pop(0))

        segments = [f"defined in {namespace} by <{locations.pop(0)}>"]

        if len(locations) > 0:
            segments.append(", also provided by ")
            segments.append(", ".join([f"<{header}>" for header in locations]))

        return "".join(("<sub>", *segments, "</sub>", "\n"))

    text = re.sub(
        r"^location: *(.*)",
        location,
        text,
        flags=re.MULTILINE,
    )

    text = re.sub(r"@(.)", lambda m: html.escape(m[1]), text)

    if trail is not None and "->home" not in text:
        trailer = ["[home](/README.md)"]
        for i, node in enumerate(trail):
            path = "/".join(trail[:i + 1])
            trailer.append(f"[{node}](/{path}.md)")

        text = f"<sub>{' / '.join(trailer)}</sub><br>\n" + text

    text = replace_links(text)

    return text
