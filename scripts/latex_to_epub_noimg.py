#!/usr/bin/env python3
import argparse
import html as htmllib
import re
import subprocess
import sys
from pathlib import Path


def merge_tex(path, root_dir, seen):
    path = path.resolve()
    if path in seen:
        return f"\n% SKIP INPUT {path.name} (already included)\n"
    seen.add(path)

    text = path.read_text(encoding="utf-8", errors="ignore")
    pattern = re.compile(r"\\input\{([^}]+)\}")

    def repl(match):
        name = match.group(1).strip()
        if not name:
            return match.group(0)
        target = root_dir / name
        if target.suffix == "":
            target = target.with_suffix(".tex")
        if not target.exists():
            return match.group(0)
        content = merge_tex(target, root_dir, seen)
        return f"\n% BEGIN INPUT {target.name}\n{content}\n% END INPUT {target.name}\n"

    return pattern.sub(repl, text)


def find_bib(root_dir):
    for name in ("refs.bib", "references.bib", "ref.bib"):
        candidate = root_dir / name
        if candidate.exists():
            return candidate
    return None


def run_pandoc(tex_path, html_out, root_dir, bib_path, mathml):
    cmd = [
        "pandoc",
        str(tex_path),
        "-o",
        str(html_out),
        "--standalone",
        f"--resource-path={root_dir}",
    ]
    if mathml:
        cmd.append("--mathml")
    if bib_path:
        cmd += ["--citeproc", f"--bibliography={bib_path}"]
    subprocess.run(cmd, check=True)


def strip_tags(text):
    return re.sub(r"<[^>]+>", "", text)


def strip_figures(html_text):
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

    html_text = re.sub(r"<figure[^>]*>.*?</figure>", figure_repl, html_text, flags=re.DOTALL)

    def embed_repl(match):
        src = match.group(1)
        return f"<p>[Figure omitted | src: {src}]</p>"

    html_text = re.sub(
        r'<embed[^>]*src="([^"]+)"[^>]*/?>',
        embed_repl,
        html_text,
        flags=re.IGNORECASE,
    )
    html_text = re.sub(
        r'<img[^>]*src="([^"]+)"[^>]*/?>',
        embed_repl,
        html_text,
        flags=re.IGNORECASE,
    )
    return html_text


def tex_to_html(tex):
    tex = htmllib.unescape(tex).strip()
    if tex.startswith("$") and tex.endswith("$"):
        tex = tex[1:-1]
    if tex.startswith("\\(") and tex.endswith("\\)"):
        tex = tex[2:-2]
    if tex.startswith("\\[") and tex.endswith("\\]"):
        tex = tex[2:-2]

    tex = tex.replace("\\left", "").replace("\\right", "")
    tex = tex.replace("~", " ")
    tex = tex.replace("\\,", " ").replace("\\;", " ").replace("\\:", " ")

    command_map = {
        "times": "×",
        "cdot": "·",
        "pm": "±",
        "leq": "≤",
        "le": "≤",
        "geq": "≥",
        "ge": "≥",
        "neq": "≠",
        "approx": "≈",
        "in": "∈",
        "notin": "∉",
        "to": "→",
        "rightarrow": "→",
        "leftarrow": "←",
        "sum": "∑",
        "prod": "∏",
        "dots": "…",
        "ldots": "…",
        "cdots": "…",
    }
    greek_map = {
        "alpha": "α",
        "beta": "β",
        "gamma": "γ",
        "delta": "δ",
        "epsilon": "ε",
        "zeta": "ζ",
        "eta": "η",
        "theta": "θ",
        "iota": "ι",
        "kappa": "κ",
        "lambda": "λ",
        "mu": "μ",
        "nu": "ν",
        "xi": "ξ",
        "omicron": "ο",
        "pi": "π",
        "rho": "ρ",
        "sigma": "σ",
        "tau": "τ",
        "upsilon": "υ",
        "phi": "φ",
        "chi": "χ",
        "psi": "ψ",
        "omega": "ω",
        "Gamma": "Γ",
        "Delta": "Δ",
        "Theta": "Θ",
        "Lambda": "Λ",
        "Xi": "Ξ",
        "Pi": "Π",
        "Sigma": "Σ",
        "Upsilon": "Υ",
        "Phi": "Φ",
        "Psi": "Ψ",
        "Omega": "Ω",
    }
    text_commands = {
        "text",
        "mathrm",
        "mathbf",
        "mathbb",
        "mathcal",
        "mathit",
        "mathsf",
        "operatorname",
    }
    passthrough_commands = {"left", "right", "big", "Big", "bigg", "Bigg"}

    def parse_group(src, idx):
        if idx >= len(src) or src[idx] != "{":
            return "", idx
        return parse_tex(src, idx + 1, stop_char="}")

    def parse_command(src, idx):
        if idx + 1 < len(src) and not src[idx + 1].isalpha():
            escaped = src[idx + 1]
            return htmllib.escape(escaped), idx + 2
        m = re.match(r"\\([A-Za-z]+)", src[idx:])
        if not m:
            return htmllib.escape(src[idx]), idx + 1
        cmd = m.group(1)
        idx += len(m.group(0))
        if cmd in passthrough_commands:
            return "", idx
        if cmd == "frac":
            num, idx = parse_group(src, idx)
            den, idx = parse_group(src, idx)
            return f"({num})/({den})", idx
        if cmd in text_commands:
            content, idx = parse_group(src, idx)
            return content, idx
        if cmd in command_map:
            return htmllib.escape(command_map[cmd]), idx
        if cmd in greek_map:
            return htmllib.escape(greek_map[cmd]), idx
        return htmllib.escape(cmd), idx

    def parse_script_target(src, idx):
        if idx >= len(src):
            return "", idx
        if src[idx] == "{":
            return parse_group(src, idx)
        if src[idx] == "\\":
            return parse_command(src, idx)
        return htmllib.escape(src[idx]), idx + 1

    def parse_tex(src, idx=0, stop_char=None):
        out = []
        while idx < len(src):
            ch = src[idx]
            if stop_char and ch == stop_char:
                return "".join(out), idx + 1
            if ch == "\\":
                rendered, idx = parse_command(src, idx)
                out.append(rendered)
                continue
            if ch in {"^", "_"}:
                tag = "sup" if ch == "^" else "sub"
                idx += 1
                content, idx = parse_script_target(src, idx)
                out.append(f"<{tag}>{content}</{tag}>")
                continue
            if ch == "{":
                content, idx = parse_group(src, idx)
                out.append(content)
                continue
            out.append(htmllib.escape(ch))
            idx += 1
        return "".join(out), idx

    rendered, _ = parse_tex(tex, 0, None)
    rendered = re.sub(r"\s+", " ", rendered).strip()
    return rendered


def inline_math_to_html(html_text):
    def repl(match):
        inner = match.group(1)
        if "$" not in inner and "\\" not in inner:
            return match.group(0)
        plain = tex_to_html(re.sub(r"<[^>]+>", "", inner))
        return f'<span class="math inline">{plain}</span>'

    def repl_display(match):
        inner = match.group(1)
        if "$" not in inner and "\\" not in inner:
            return match.group(0)
        plain = tex_to_html(re.sub(r"<[^>]+>", "", inner))
        return f'<div class="math display">{plain}</div>'

    html_text = re.sub(
        r'<span[^>]*class="math inline"[^>]*>(.*?)</span>',
        repl,
        html_text,
        flags=re.DOTALL,
    )
    html_text = re.sub(
        r'<div[^>]*class="math display"[^>]*>(.*?)</div>',
        repl_display,
        html_text,
        flags=re.DOTALL,
    )
    return html_text


def main():
    parser = argparse.ArgumentParser(
        description="Convert arXiv LaTeX to a no-image EPUB for CrossPoint Reader."
    )
    parser.add_argument("--tex", required=True, help="Path to the main .tex file.")
    parser.add_argument("--out", required=True, help="Path to the output .epub file.")
    parser.add_argument("--title", default=None, help="Override title metadata.")
    parser.add_argument("--bib", default=None, help="Optional .bib file path.")
    parser.add_argument("--split-level", type=int, default=1)
    parser.add_argument("--toc-depth", type=int, default=2)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Directory for intermediate files (default: <tex dir>/build).",
    )
    parser.add_argument("--no-merge", action="store_true", help="Skip merging \\input{}.")
    parser.add_argument(
        "--keep-clean-html",
        action="store_true",
        help="Keep the cleaned mathfix HTML output.",
    )
    parser.add_argument(
        "--mathml",
        action="store_true",
        help="Use MathML output to render math on device.",
    )
    parser.add_argument(
        "--keep-tex-math",
        action="store_true",
        help="Keep inline TeX math instead of converting to plain text.",
    )
    args = parser.parse_args()

    tex_path = Path(args.tex).resolve()
    if not tex_path.exists():
        raise FileNotFoundError(f"TeX file not found: {tex_path}")

    root_dir = tex_path.parent
    build_dir = Path(args.build_dir).resolve() if args.build_dir else root_dir / "build"
    build_dir.mkdir(parents=True, exist_ok=True)

    merged_tex = tex_path
    if not args.no_merge:
        merged_tex = build_dir / f"{tex_path.stem}_merged.tex"
        merged = merge_tex(tex_path, root_dir, set())
        merged_tex.write_text(merged, encoding="utf-8")

    html_path = build_dir / f"{tex_path.stem}.html"
    noimg_html = build_dir / f"{tex_path.stem}.noimg.html"
    clean_html = build_dir / f"{tex_path.stem}.noimg.mathfix.html"

    bib_path = Path(args.bib).resolve() if args.bib else find_bib(root_dir)
    run_pandoc(merged_tex, html_path, root_dir, bib_path, args.mathml)

    html_text = html_path.read_text(encoding="utf-8", errors="ignore")
    noimg_text = strip_figures(html_text)
    if not args.keep_tex_math and not args.mathml:
        noimg_text = inline_math_to_html(noimg_text)
    noimg_html.write_text(noimg_text, encoding="utf-8")

    script_dir = Path(__file__).resolve().parent
    mathfix_script = script_dir / "html_to_epub_mathfix.py"
    if not mathfix_script.exists():
        raise FileNotFoundError(f"Missing script: {mathfix_script}")

    out_epub = Path(args.out).resolve()
    cmd = [
        sys.executable,
        str(mathfix_script),
        str(noimg_html),
        str(out_epub),
        "--split-level",
        str(args.split_level),
        "--toc-depth",
        str(args.toc_depth),
    ]
    if args.mathml:
        cmd.append("--mathml")
    if args.title:
        cmd += ["--title", args.title]
    if args.keep_clean_html:
        cmd += ["--output-html", str(clean_html)]
    subprocess.run(cmd, check=True)

    print(f"Wrote {out_epub}")


if __name__ == "__main__":
    main()
