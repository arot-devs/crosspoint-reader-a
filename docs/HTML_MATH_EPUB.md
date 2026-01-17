# Converting HTML With Math to EPUB (device-safe)

The device does not render MathJax SVG images and does not ship math fonts, so
equations can show up as duplicated text or question marks. The workflow below
keeps inline math as text (no duplicates) and converts display math blocks into
plain text so they do not disappear. Some readers also ignore CSS, so list
formatting is baked into the HTML to avoid extra indentation or blank bullets.

## Requirements

- Python 3
- pandoc

## 1) Prepare a cleaned HTML file (math + list baking)

This script:
- removes MathJax SVG output (so inline math is not duplicated),
- keeps inline TeX scripts for pandoc to render as text,
- converts display MathML into a plain-text block.
- bakes list formatting by removing paragraph wrappers inside list items.

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

out.write_text(text, encoding="utf-8")
print(f"Wrote {out}")
```

Run:

```sh
python math_html_clean.py
```

## 2) Convert to EPUB

```sh
pandoc input.mathfix.html -o output.epub --toc --standalone --metadata title="Your Title"
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
- If you prefer MathML and the device supports it, you can strip SVG output but
  keep assistive MathML. In that case, remove the block that replaces MathML
  with plain text and only delete the `mjx-container` SVG output.
