import argparse
import html as htmllib
import os
import re
import subprocess
import tempfile
import zipfile


def clean_html(text):
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
    text = re.sub(
        r"<mjx-assistive-mml[^>]*>.*?</mjx-assistive-mml>", "", text, flags=re.DOTALL
    )

    # Bake list formatting: remove paragraph wrappers inside list items.
    text = re.sub(r"(<li[^>]*>)\s*<p>", r"\1", text, flags=re.IGNORECASE)
    text = re.sub(r"</p>\s*(</li>|<ul|<ol)", r"\1", text, flags=re.IGNORECASE)

    # Drop Typora’s inline TOC block.
    text = re.sub(
        r"<div class=['\"]md-toc[^>]*>.*?</div>", "", text, flags=re.DOTALL | re.IGNORECASE
    )

    # Drop the first H1 to avoid an empty title chapter.
    text = re.sub(r"<h1[^>]*>.*?</h1>", "", text, count=1, flags=re.IGNORECASE | re.DOTALL)

    # Drop the first H2 if it is immediately followed by another H2.
    text = re.sub(
        r"<h2[^>]*>.*?</h2>\s*(<hr\s*/?>\s*)?(?=<h2)",
        "",
        text,
        count=1,
        flags=re.IGNORECASE | re.DOTALL,
    )

    # Remove outer Typora wrapper divs to allow proper chapter splitting.
    text = re.sub(
        r"<div class=['\"]typora-export-content['\"]>\s*",
        "",
        text,
        count=1,
        flags=re.IGNORECASE,
    )
    text = re.sub(
        r"<div id=['\"]write['\"][^>]*>\s*",
        "",
        text,
        count=1,
        flags=re.IGNORECASE,
    )
    text = re.sub(
        r"\s*</div>\s*</div>\s*</body>",
        "</body>",
        text,
        count=1,
        flags=re.IGNORECASE | re.DOTALL,
    )

    return text


def is_empty_chapter(xhtml):
    body = re.search(r"<body[^>]*>(.*)</body>", xhtml, re.DOTALL | re.IGNORECASE)
    if not body:
        return False
    content = body.group(1)
    content = re.sub(r"<h[1-6][^>]*>.*?</h[1-6]>", "", content, flags=re.DOTALL | re.IGNORECASE)
    content = re.sub(r"<[^>]+>", "", content)
    content = htmllib.unescape(content)
    content = re.sub(r"\s+", "", content)
    return content == ""


def prune_empty_chapters(epub_path):
    with zipfile.ZipFile(epub_path, "r") as zf:
        names = zf.namelist()
        chapters = [n for n in names if n.startswith("EPUB/text/ch") and n.endswith(".xhtml")]
        empty = []
        for name in chapters:
            data = zf.read(name).decode("utf-8", errors="ignore")
            if is_empty_chapter(data):
                empty.append(name)

        if not empty:
            return

        opf = zf.read("EPUB/content.opf").decode("utf-8", errors="ignore")
        nav = zf.read("EPUB/nav.xhtml").decode("utf-8", errors="ignore")
        toc = zf.read("EPUB/toc.ncx").decode("utf-8", errors="ignore")

        removed_ids = []
        for name in empty:
            m = re.search(r'id="([^"]+)"[^>]*href="' + re.escape(name) + r'"', opf)
            if m:
                removed_ids.append(m.group(1))

        for name in empty:
            opf = re.sub(
                r"\s*<item[^>]*href=\"" + re.escape(name) + r"\"[^>]*/>",
                "",
                opf,
            )
        for item_id in removed_ids:
            opf = re.sub(
                r"\s*<itemref[^>]*idref=\"" + re.escape(item_id) + r"\"[^>]*/>",
                "",
                opf,
            )

        for name in empty:
            nav = re.sub(
                r"\s*<li[^>]*>\s*<a[^>]*href=\"" + re.escape(name) + r"\"[^>]*>.*?</a>\s*</li>",
                "",
                nav,
                flags=re.DOTALL,
            )
            toc = re.sub(
                r"\s*<navPoint[^>]*>\s*<navLabel>.*?</navLabel>\s*<content src=\""
                + re.escape(name)
                + r"\"[^>]*/>\s*</navPoint>",
                "",
                toc,
                flags=re.DOTALL,
            )

        tmp_path = epub_path + ".tmp"
        with zipfile.ZipFile(tmp_path, "w") as out:
            out.writestr("mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)
            for name in names:
                if name == "mimetype":
                    continue
                if name in empty:
                    continue
                if name == "EPUB/content.opf":
                    out.writestr(name, opf, compress_type=zipfile.ZIP_DEFLATED)
                elif name == "EPUB/nav.xhtml":
                    out.writestr(name, nav, compress_type=zipfile.ZIP_DEFLATED)
                elif name == "EPUB/toc.ncx":
                    out.writestr(name, toc, compress_type=zipfile.ZIP_DEFLATED)
                else:
                    out.writestr(name, zf.read(name), compress_type=zipfile.ZIP_DEFLATED)

    os.replace(tmp_path, epub_path)


def main():
    parser = argparse.ArgumentParser(description="Convert HTML with MathJax to device-safe EPUB.")
    parser.add_argument("input_html")
    parser.add_argument("output_epub")
    parser.add_argument("--title", default=None)
    parser.add_argument("--split-level", type=int, default=2)
    parser.add_argument("--toc-depth", type=int, default=3)
    parser.add_argument("--keep-html", action="store_true")
    parser.add_argument("--output-html", default=None)
    parser.add_argument("--pandoc", default="pandoc")
    args = parser.parse_args()

    input_path = os.path.abspath(args.input_html)
    output_path = os.path.abspath(args.output_epub)
    title = args.title or os.path.splitext(os.path.basename(input_path))[0]

    text = clean_html(Path(input_path).read_text(encoding="utf-8", errors="ignore"))

    if args.output_html:
        html_path = os.path.abspath(args.output_html)
        Path(html_path).write_text(text, encoding="utf-8")
    else:
        fd, html_path = tempfile.mkstemp(suffix=".mathfix.html")
        os.close(fd)
        Path(html_path).write_text(text, encoding="utf-8")

    try:
        cmd = [
            args.pandoc,
            html_path,
            "-o",
            output_path,
            "--standalone",
            "--toc",
            f"--toc-depth={args.toc_depth}",
            f"--split-level={args.split_level}",
            "--epub-title-page=false",
            f"--metadata=title={title}",
        ]
        subprocess.run(cmd, check=True)
        prune_empty_chapters(output_path)
    finally:
        if not args.keep_html and not args.output_html:
            try:
                os.remove(html_path)
            except OSError:
                pass


if __name__ == "__main__":
    from pathlib import Path

    main()
