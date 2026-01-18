# Converting arXiv LaTeX to device-readable EPUB (no images)

This workflow turns an arXiv LaTeX source into a readable EPUB for the
CrossPoint Reader. It skips all images (the device does not render them) and
normalizes math into simple HTML (sup/sub + symbols) so it renders on device.

## Requirements

- Python 3
- pandoc

Optional (only if you want PDF -> PNG for other targets): PyMuPDF (`fitz`)

## Reusable script (recommended)

This script does all steps (merge `\input{}`, convert TeX -> HTML, strip
figures, run math/list cleanup, and build the EPUB):

```sh
python scripts\latex_to_epub_noimg.py ^
  --tex D:\path\to\workdir\arxiv.tex ^
  --out D:\path\to\workdir\build\Your_Title.epub ^
  --title "Your Title"
```

Options:

- `--bib` if your bibliography is not `refs.bib`.
- `--build-dir` to control intermediate file location.
- `--no-merge` to skip `\input{}` merging.
- `--keep-clean-html` to keep the cleaned mathfix HTML.
- `--mathml` to emit MathML (not recommended on this device).
- `--keep-tex-math` to keep inline TeX (default converts to HTML).

## Steps

### 1) Extract the arXiv source

```sh
tar -xf arXiv-2202.00275v1.tar.gz -C D:\path\to\workdir
```

### 2) Merge `\input{}` files into a single TeX file

Pandoc does not follow `\input{}` by default. Create a merged TeX file:

```sh
@'
from pathlib import Path
import re

src_dir = Path(r"D:\path\to\workdir")
main = src_dir / "arxiv.tex"
out = src_dir / "build" / "arxiv_merged.tex"
out.parent.mkdir(parents=True, exist_ok=True)

text = main.read_text(encoding="utf-8", errors="ignore")
pattern = re.compile(r"\\input\\{([^}]+)\\}")

def repl(match):
    name = match.group(1)
    path = src_dir / (name if name.endswith(".tex") else name + ".tex")
    if not path.exists():
        return match.group(0)
    content = path.read_text(encoding="utf-8", errors="ignore")
    return f"\n% BEGIN INPUT {path.name}\n{content}\n% END INPUT {path.name}\n"

merged = pattern.sub(repl, text)
out.write_text(merged, encoding="utf-8")
print(out)
'@ | python -
```

### 3) Convert the merged TeX to HTML

Use `--resource-path` so figures are discoverable (even though we will strip
them later).

```sh
pandoc D:\path\to\workdir\build\arxiv_merged.tex ^
  -o D:\path\to\workdir\build\arxiv.html ^
  --standalone ^
  --resource-path=D:\path\to\workdir ^
  --citeproc ^
  --bibliography=D:\path\to\workdir\refs.bib
```

### 4) Replace figures with placeholders (no images on device)

This removes `<figure>`, `<img>`, and `<embed>` content and replaces each
figure with a plain-text placeholder.

```sh
@'
from pathlib import Path
import re

src = Path(r"D:\path\to\workdir\build\arxiv.html")
out = Path(r"D:\path\to\workdir\build\arxiv.noimg.html")
text = src.read_text(encoding="utf-8", errors="ignore")

def strip_tags(s):
    return re.sub(r"<[^>]+>", "", s)

def figure_repl(match):
    fig = match.group(0)
    fig_id = None
    m_id = re.search(r'<figure[^>]*id="([^"]+)"', fig)
    if m_id:
        fig_id = m_id.group(1)
    m_src = re.search(r'(?:<embed|<img)[^>]*src="([^"]+)"', fig)
    src = m_src.group(1) if m_src else None
    caption_text = ""
    m_caption = re.search(r"<figcaption[^>]*>(.*?)</figcaption>", fig, flags=re.DOTALL)
    if m_caption:
        caption_text = strip_tags(m_caption.group(1)).strip()
    parts = ["Figure omitted"]
    if fig_id:
        parts.append(f"id: {fig_id}")
    if src:
        parts.append(f"src: {src}")
    if caption_text:
        parts.append(f"caption: {caption_text}")
    return "<p>[" + " | ".join(parts) + "]</p>"

text = re.sub(r"<figure[^>]*>.*?</figure>", figure_repl, text, flags=re.DOTALL)
text = re.sub(r'<embed[^>]*src="([^"]+)"[^>]*/?>',
              lambda m: f"<p>[Figure omitted | src: {m.group(1)}]</p>",
              text, flags=re.IGNORECASE)
text = re.sub(r'<img[^>]*src="([^"]+)"[^>]*/?>',
              lambda m: f"<p>[Figure omitted | src: {m.group(1)}]</p>",
              text, flags=re.IGNORECASE)

out.write_text(text, encoding="utf-8")
print(out)
'@ | python -
```

### 5) Clean math + bake lists and convert to EPUB

Use the repo helper to strip MathJax SVG and bake list formatting so the device
does not show double formulas or broken list bullets.

```sh
python scripts\html_to_epub_mathfix.py ^
  D:\path\to\workdir\build\arxiv.noimg.html ^
  D:\path\to\workdir\build\Your_Title.epub ^
  --title "Your Title" ^
  --split-level 1 ^
  --toc-depth 2 ^
  --output-html D:\path\to\workdir\build\arxiv.noimg.mathfix.html
```

## Upload to device

CrossPoint Reader mode:

```sh
curl -F "file=@D:\path\to\workdir\build\Your_Title.epub" "http://crosspoint.local/upload?path=/"
```

If you prefer a folder:

```sh
curl -F "file=@D:\path\to\workdir\build\Your_Title.epub" "http://crosspoint.local/upload?path=/Books"
```

## Quick verification

Confirm the EPUB has no image tags or MathJax artifacts:

```sh
@'
from zipfile import ZipFile
import re

path = r"D:\path\to\workdir\build\Your_Title.epub"
patterns = [re.compile(r"<img\\b", re.IGNORECASE),
            re.compile(r"<embed\\b", re.IGNORECASE),
            re.compile(r"<mjx-container", re.IGNORECASE)]

hits = {"img": [], "embed": [], "mjx": []}
with ZipFile(path, "r") as zf:
    for name in zf.namelist():
        if not name.endswith(".xhtml"):
            continue
        data = zf.read(name).decode("utf-8", errors="ignore")
        if patterns[0].search(data):
            hits["img"].append(name)
        if patterns[1].search(data):
            hits["embed"].append(name)
        if patterns[2].search(data):
            hits["mjx"].append(name)

print(hits)
'@ | python -
```

## Notes

- If inline math shows "?" on device, the glyph is missing from the device
  font. This is a font coverage issue, not an encoding problem.
- Pandoc may warn about duplicate list roles; the output still renders fine.
- Inline TeX that pandoc cannot convert is normalized into simple HTML
  (sup/sub + symbols) by default; use `--keep-tex-math` to preserve raw TeX.
- MathML does not render reliably on this device; prefer the HTML math output.
- The HTML math uses Unicode symbols (e.g., `×`, `∑`, `≤`, `≥`). If a symbol
  is missing from the device font, it will appear as "?". Use `--keep-tex-math`
  or adjust the text to ASCII as a fallback.
