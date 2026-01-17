# Converting HTML With Math to EPUB (device-safe)

The device does not render MathJax SVG images and does not ship math fonts, so
equations can show up as duplicated text or question marks. The workflow below
keeps inline math as text (no duplicates) and converts display math blocks into
plain text so they do not disappear. Some readers also ignore CSS, so list
formatting is baked into the HTML to avoid extra indentation or blank bullets.
This version also removes Typora’s inline TOC and wrapper divs so pandoc can
split chapters correctly.

## Requirements

- Python 3
- pandoc

## Recommended script (HTML -> EPUB)

Use the helper script to clean MathJax output, bake list formatting, remove the
inline TOC, split chapters, and prune empty chapters in one step.

PowerShell:

```sh
python scripts/html_to_epub_mathfix.py input.html output.epub --title "Your Title"
```

Optional: keep the cleaned HTML for debugging:

```sh
python scripts/html_to_epub_mathfix.py input.html output.epub --output-html cleaned.html
```

The script:
- removes MathJax SVG output (so inline math is not duplicated),
- keeps inline TeX scripts for pandoc to render as text,
- converts display MathML into a plain-text block,
- bakes list formatting by removing paragraph wrappers inside list items,
- removes Typora’s inline TOC block,
- removes outer wrapper divs so `--split-level` works,
- drops subtitle-only H2 blocks that would become empty chapters,
- removes title-only or heading-only chapters after EPUB creation.

## Manual steps (if needed)

If you want to run the steps manually, use the cleaner below.

## 1) Prepare a cleaned HTML file (math + list baking)

This script:
- removes MathJax SVG output (so inline math is not duplicated),
- keeps inline TeX scripts for pandoc to render as text,
- converts display MathML into a plain-text block,
- bakes list formatting by removing paragraph wrappers inside list items,
- removes Typora’s inline TOC block,
- removes outer wrapper divs so `--split-level` works,
- drops a subtitle H2 if it is immediately followed by another H2.

Save as `math_html_clean.py`:

```python
from pathlib import Path
import re
import html as htmllib

src = Path("input.html")
out = Path("input.mathfix.html")

text = src.read_text(encoding="utf-8", errors="ignore")

mjx_re = re.compile(r"<mjx-container[^>]*>.*?</mjx-container>", re.DOTALL)

def replace_mjx(match):
    mjx = match.group(0)
    is_display = "display=\"true\"" in mjx or "display=\"block\"" in mjx
    if not is_display:
        # Inline math: drop rendered SVG; keep the TeX script elsewhere.
        return ""

    m = re.search(r"<mjx-assistive-mml[^>]*>(.*?)</mjx-assistive-mml>", mjx, re.DOTALL)
    if not m:
        return ""

    mathml = m.group(1)
    plain = re.sub(r"<[^>]+>", "", mathml)
    plain = htmllib.unescape(plain)
    plain = re.sub(r"\s+", " ", plain).strip()
    return f'<div class="math-text">{plain}</div>'

text = mjx_re.sub(replace_mjx, text)

# Remove any leftover assistive MathML nodes.
text = re.sub(r"<mjx-assistive-mml[^>]*>.*?</mjx-assistive-mml>", "", text, flags=re.DOTALL)

# Bake list formatting: remove paragraph wrappers inside list items.
text = re.sub(r"(<li[^>]*>)\s*<p>", r"\1", text, flags=re.IGNORECASE)
text = re.sub(r"</p>\s*(</li>|<ul|<ol)", r"\1", text, flags=re.IGNORECASE)

# Drop Typora’s inline TOC block (prevents a giant TOC section in content).
text = re.sub(r"<div class=['\"]md-toc[^>]*>.*?</div>", "", text, flags=re.DOTALL | re.IGNORECASE)

# Drop the first H1 to avoid an empty title chapter; metadata title stays in EPUB.
text = re.sub(r"<h1[^>]*>.*?</h1>", "", text, count=1, flags=re.IGNORECASE | re.DOTALL)

# Drop the first H2 if it is immediately followed by another H2 (subtitle-only section).
text = re.sub(r"<h2[^>]*>.*?</h2>\s*(<hr\s*/?>\s*)?(?=<h2)", "", text, count=1, flags=re.IGNORECASE | re.DOTALL)

# Remove outer Typora wrapper divs to allow proper chapter splitting.
text = re.sub(r"<div class=['\"]typora-export-content['\"]>\s*", "", text, count=1, flags=re.IGNORECASE)
text = re.sub(r"<div id=['\"]write['\"][^>]*>\s*", "", text, count=1, flags=re.IGNORECASE)
text = re.sub(r"\s*</div>\s*</div>\s*</body>", "</body>", text, count=1, flags=re.IGNORECASE | re.DOTALL)

out.write_text(text, encoding="utf-8")
print(f"Wrote {out}")
```

Run:

```sh
python math_html_clean.py
```

## 2) Convert to EPUB

```sh
pandoc input.mathfix.html -o output.epub --standalone --toc --toc-depth=3 \
  --split-level=2 --epub-title-page=false --metadata title="Your Title"
```

## 3) Upload (keep unique filenames)

Avoid overwriting when debugging:

```sh
curl -F "file=@output.epub;filename=output_debug.epub" http://DEVICE_IP/upload?path=/
```

## Notes

- If you still see "?" in inline math, the device is missing glyphs. This is a
  font coverage issue, not UTF-8. You will need a math-capable font on the
  device to render symbols like script letters or angle brackets.
- If the reader ignores CSS, list indentation fixes will not apply. In that
  case, keep the list baking step above.
- Pandoc always injects a title-only chapter when a title is set. The helper
  script prunes empty chapters, including title-only ones.
- If you prefer MathML and the device supports it, you can strip SVG output but
  keep assistive MathML. In that case, remove the block that replaces MathML
  with plain text and only delete the `mjx-container` SVG output.
