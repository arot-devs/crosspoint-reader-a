# Uploading Content to the Device

This device runs the classic FSBrowser-style API. The web UI at `/edit` uses the
endpoints below. Use these for programmatic uploads.

## Quick upload (curl)

Upload `AGENTS.epub` to the SD root:

```sh
curl -F "data=@AGENTS.epub;filename=/AGENTS.epub" http://192.168.100.121/edit
```

Upload to a subfolder (folder must exist):

```sh
curl -F "data=@AGENTS.epub;filename=/Books/AGENTS.epub" http://192.168.100.121/edit
```

## PDF to EPUB (image per page)

This device expects a table of contents. For PDFs, an image-per-page EPUB with
an auto-generated TOC is the most reliable option. This keeps every page as a
JPEG image (no text extraction).

1) Install the renderer:

```sh
python -m pip install pymupdf
```

2) Create `pdf_to_epub_images.py`:

```python
import argparse
import os
import tempfile
import uuid
import zipfile
from datetime import datetime, timezone
from pathlib import Path

import fitz


def build_epub(pdf_path, out_epub, title=None, scale=2.0, lang="en"):
    if title is None:
        title = Path(pdf_path).stem

    book_id = f"urn:uuid:{uuid.uuid4()}"
    modified = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        meta_inf = tmp_path / "META-INF"
        oebps = tmp_path / "OEBPS"
        meta_inf.mkdir(parents=True, exist_ok=True)
        oebps.mkdir(parents=True, exist_ok=True)

        (meta_inf / "container.xml").write_text(
            """<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml" />
  </rootfiles>
</container>
""",
            encoding="utf-8",
        )

        (oebps / "styles.css").write_text(
            """body { margin: 0; padding: 0; }
.page { text-align: center; }
.page img { width: 100%; height: auto; display: block; }
""",
            encoding="utf-8",
        )

        doc = fitz.open(pdf_path)
        manifest_items = []
        spine_items = []
        toc_points = []
        nav_items = []

        for index in range(doc.page_count):
            page = doc.load_page(index)
            pix = page.get_pixmap(matrix=fitz.Matrix(scale, scale), alpha=False)
            img_name = f"image{index + 1:04d}.jpg"
            pix.save(str(oebps / img_name))

            html_name = f"page{index + 1:04d}.xhtml"
            page_title = f"Page {index + 1}"
            (oebps / html_name).write_text(
                f"""<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta charset="utf-8" />
  <title>{page_title}</title>
  <link rel="stylesheet" type="text/css" href="styles.css" />
</head>
<body>
  <div class="page"><img src="{img_name}" alt="{page_title}" /></div>
</body>
</html>
""",
                encoding="utf-8",
            )

            manifest_items.append((f"page{index + 1}", html_name, "application/xhtml+xml"))
            manifest_items.append((f"img{index + 1}", img_name, "image/jpeg"))
            spine_items.append(f"page{index + 1}")
            toc_points.append((index + 1, page_title, html_name))
            nav_items.append((page_title, html_name))

        nav_entries = "\n".join(
            f'    <li><a href="{href}">{label}</a></li>' for label, href in nav_items
        )
        (oebps / "nav.xhtml").write_text(
            f"""<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops">
<head>
  <meta charset="utf-8" />
  <title>Table of Contents</title>
</head>
<body>
  <nav epub:type="toc" id="toc">
    <h1>Table of Contents</h1>
    <ol>
{nav_entries}
    </ol>
  </nav>
</body>
</html>
""",
            encoding="utf-8",
        )

        toc_entries = []
        for play_order, label, href in toc_points:
            toc_entries.append(
                f"""  <navPoint id="navPoint-{play_order}" playOrder="{play_order}">
    <navLabel><text>{label}</text></navLabel>
    <content src="{href}" />
  </navPoint>"""
            )

        (oebps / "toc.ncx").write_text(
            f"""<?xml version="1.0" encoding="utf-8"?>
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">
  <head>
    <meta name="dtb:uid" content="{book_id}" />
    <meta name="dtb:depth" content="1" />
    <meta name="dtb:totalPageCount" content="0" />
    <meta name="dtb:maxPageNumber" content="0" />
  </head>
  <docTitle><text>{title}</text></docTitle>
  <navMap>
{os.linesep.join(toc_entries)}
  </navMap>
</ncx>
""",
            encoding="utf-8",
        )

        manifest_lines = [
            '<item id="nav" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav" />',
            '<item id="toc" href="toc.ncx" media-type="application/x-dtbncx+xml" />',
            '<item id="css" href="styles.css" media-type="text/css" />',
        ]
        for item_id, href, media_type in manifest_items:
            manifest_lines.append(
                f'<item id="{item_id}" href="{href}" media-type="{media_type}" />'
            )

        spine_lines = [f'<itemref idref="{item_id}" />' for item_id in spine_items]

        (oebps / "content.opf").write_text(
            f"""<?xml version="1.0" encoding="utf-8"?>
<package version="3.0" unique-identifier="bookid" xmlns="http://www.idpf.org/2007/opf">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="bookid">{book_id}</dc:identifier>
    <dc:title>{title}</dc:title>
    <dc:language>{lang}</dc:language>
    <meta property="dcterms:modified">{modified}</meta>
  </metadata>
  <manifest>
    {os.linesep.join(manifest_lines)}
  </manifest>
  <spine toc="toc">
    {os.linesep.join(spine_lines)}
  </spine>
</package>
""",
            encoding="utf-8",
        )

        with zipfile.ZipFile(out_epub, "w") as zf:
            zf.writestr("mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)
            for path in meta_inf.rglob("*"):
                if path.is_file():
                    zf.write(path, path.relative_to(tmp_path).as_posix(), compress_type=zipfile.ZIP_DEFLATED)
            for path in oebps.rglob("*"):
                if path.is_file():
                    zf.write(path, path.relative_to(tmp_path).as_posix(), compress_type=zipfile.ZIP_DEFLATED)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert PDF to image-per-page EPUB with TOC.")
    parser.add_argument("input_pdf")
    parser.add_argument("output_epub")
    parser.add_argument("--scale", type=float, default=2.0, help="Render scale (2.0 ~= 144 dpi).")
    parser.add_argument("--title", default=None)
    parser.add_argument("--lang", default="en")
    args = parser.parse_args()
    build_epub(args.input_pdf, args.output_epub, title=args.title, scale=args.scale, lang=args.lang)
```

3) Convert, then upload:

```sh
python pdf_to_epub_images.py lecture2_compressed.pdf lecture2_compressed.epub --scale 2.0
curl -F "data=@lecture2_compressed.epub;filename=/lecture2_compressed.epub" http://192.168.100.121/edit
```

## Verify contents

List a directory:

```sh
curl "http://192.168.100.121/list?dir=/"
```

## Status (optional)

```sh
curl "http://192.168.100.121/status"
```

## How /edit works (from the on-device UI)

The `/edit` page uploads with multipart form data:

- Method: `POST /edit`
- Field name: `data`
- Filename: the full target path (e.g. `/Books/MyBook.epub`)

That is why the curl examples pass `filename=/path/to/file` for the upload.

## Create a folder (optional)

```sh
curl -X PUT -F "path=/Books/" http://192.168.100.121/edit
```

## Rename or move (optional)

```sh
curl -X PUT -F "path=/Books/NewName.epub" -F "src=/Books/OldName.epub" \
  http://192.168.100.121/edit
```

## Delete (optional)

```sh
curl -X DELETE -F "path=/Books/OldName.epub" http://192.168.100.121/edit
```
