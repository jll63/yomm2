from pathlib import Path
import re
import sys

targets = {"home": "/README.md"}


def index_targets():
    cpps = set()
    for ref in (Path(__file__).parent.parent / "reference.in").iterdir():
        if ref.suffix == ".cpp" or (ref.suffix == ".md" and ref.stem not in cpps):
            cpps.add(ref.stem)
            md_path = ref.with_suffix(".md").name.replace("reference.in", "reference")
            targets[ref.stem] = md_path
            with open(ref) as rh:
                for text in rh.readlines():
                    if "---" in text:
                        break
                    m = re.match(r"^(?:// *)?## +(.*)", text)
                    if m:
                        for symbol in m.group(1).split(","):
                            symbol = symbol.strip().replace("yorel::yomm2::", "")
                            if symbol != ref.stem:
                                targets[symbol] = md_path
                    m = re.search(r"target: *([\w_-]+)", text)
                    if m:
                        targets[m.group(1)] = md_path
                                
                        

    return targets

def replace_link(text, **kwargs):
    def sub(m):
        text = m.group(1)
        symbol = text.replace("`", "")
        target = targets.get(symbol)
        if target is None:
            print("BROKEN:", symbol, file=sys.stderr)
            symbol = f"{symbol} (BROKEN)"
        return f"[{text}]({target})"
    
    return re.sub(r"->(`?[\w_-]+`?)", sub, text, **kwargs)
