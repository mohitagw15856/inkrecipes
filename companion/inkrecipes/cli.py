"""Command line interface: `inkrecipes convert`."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from . import convert as convert_mod


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="inkrecipes", description="Build .rcp recipes")
    sub = parser.add_subparsers(dest="command", required=True)

    c = sub.add_parser("convert", help="convert Markdown or a recipe URL to .rcp")
    c.add_argument("source", help="a .md file (authoring convention) or an http(s) URL / .html file with schema.org Recipe JSON-LD")
    c.add_argument("--out", required=True, type=Path)

    args = parser.parse_args(argv)
    try:
        if args.source.endswith(".md"):
            doc = convert_mod.convert_markdown(Path(args.source).read_text(encoding="utf-8"))
        else:
            doc = convert_mod.convert_jsonld_html(convert_mod.fetch_html(args.source))
    except (ValueError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(doc, encoding="utf-8")
    print(args.out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
