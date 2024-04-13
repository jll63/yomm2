from dominate import tags
import html
import glob
import json
from pathlib import Path
import re
import sys

repository = Path(__file__).absolute()
while not (repository / ".git").exists():
    repository = repository.parent


def split_list(text, sep=","):
    return list(map(str.strip, text.split(sep)))


hrefs = {
    "home": "https://github.com/jll63/yomm2",
    "reference": "/yomm2",
}
in_path = repository.joinpath("docs.in", "reference")
in_files = list(
    Path(ref_str)
    for ref_str in sorted(
        glob.glob("**/*.md", root_dir=in_path, recursive=True)
        + glob.glob("**/*.cpp", root_dir=in_path, recursive=True),
        key=str.lower,
    )
)


for ref in in_files:
    md_path = str(ref.with_suffix(".html")).replace("docs.in", "")
    hrefs[str(ref.with_suffix("")).replace("::", "-")] = f"/yomm2/reference/{md_path}"
    with open(in_path / ref) as rh:
        for text in rh.readlines():
            if m := re.match(r"^(?:entry|macro): *(.*)", text):
                for symbol in split_list(m[1]):
                    symbol = (
                        symbol.strip()
                        .replace("*", "")
                        .replace("yorel::yomm2::", "")
                        .replace("::", "-")
                    )
                    if symbol != ref.stem:
                        hrefs[symbol] = f"/yomm2/reference/{md_path}"
            if m := re.search(r"hrefs: *(.*)", text):
                for href in split_list(m.group(1)):
                    hrefs[href] = f"/yomm2/reference/{md_path}"

with open(repository.joinpath(".hrefs"), "w", encoding="ascii") as fh:
    json.dump(hrefs, fh, indent=4, sort_keys=True)


LINK_EXCLUSIONS = set("name".split())


def replace_links(text: str, source: str):
    def sub(m):
        path = m.group(2)
        target = hrefs.get(path.replace("::", "-"))
        if target is None and path not in LINK_EXCLUSIONS:
            print(f"{source}: POSSIBLY BROKEN: {path}", file=sys.stderr)
            return f"->{path}"
        return f"[{m.group(1)}{path.split('-')[-1]}{m.group(3)}]({target})"

    return re.sub(r"->(`?)([/:\w_\-]+)(`?)", sub, text)


HEADER_FONT_SIZE = "150%"


def format_entry(text: str):
    return f"<strong>{text}</strong>"


# , cls="heading-element"


def format_cpp_entry(symbol: str):
    parts = ["yorel", "yomm2", *symbol.replace("yorel::yomm2::", "").split("::")]
    leaf = parts.pop()
    return "{}::<strong>{}</strong>".format("::".join([*parts]), leaf)


def replace_md(text: str, source: str = "n/a", trail: list[str] = None):
    def replace_entries(m):
        return '<span style="font-size:xx-large;">{}</span><br/>'.format(
            "<br/>\n".join([format_cpp_entry(symbol) for symbol in split_list(m[1])])
        )

    text = re.sub(
        r"^entry: *(.*)",
        replace_entries,
        text,
        flags=re.MULTILINE,
    )

    text = re.sub(
        r"^macro: *(.*)",
        lambda m: '<span style="font-size:xx-large;">{}</span><br/>'.format(
            "".join([f"{format_entry(macro)}<br/>" for macro in split_list(m[1])])
        ),
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
            segments.append(", also provided by ")
            segments.append(", ".join([f"<{header}>" for header in others]))

        return "".join(("<sub>", *segments, "</sub>", "<br/>"))

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

    # trail = source.split("/")
    # trail.pop(0)
    # trail.pop(0)
    # trail.pop()
    # if trail is not None and "->home" not in text:
    #     trailer = ["[home](/yomm2/README.md)"]
    #     for i, node in enumerate(trail):
    #         path = "/".join(trail[: i + 1])

    #         trailer.append(f"[{node}](/yomm2/{path})")

    #     text = f"<sub>{' / '.join(trailer)}</sub><br>\n" + text

    text = replace_links(text, source)

    return text
