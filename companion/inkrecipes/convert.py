"""Builds .rcp files from Markdown recipes or schema.org Recipe JSON-LD."""

from __future__ import annotations

import json
import re
import textwrap
import urllib.request
from html.parser import HTMLParser
from pathlib import Path

from . import quantities

MAGIC = "RCP 1"
STEP_WRAP = 40  # cooking mode renders 15px-wide glyphs; 40 columns fits X4/X3


def _wrap_step(text: str) -> list[str]:
    return textwrap.wrap(text, width=STEP_WRAP) or [""]


def _serialise(title: str, serves: int, minutes: int,
               ingredients: list[quantities.ParsedIngredient],
               steps: list[tuple[int, str]]) -> str:
    out = [MAGIC, f"M title {title}", f"M serves {serves}"]
    if minutes:
        out.append(f"M time {minutes}")
    for ing in ingredients:
        qty = "" if ing.quantity <= 0 else f"{ing.quantity:g}"
        out.append(f"I {qty}|{ing.unit}|{ing.name}")
    for seconds, text in steps:
        out.append(f"S {seconds}")
        for line in _wrap_step(text):
            out.append(f"T {line}")
    return "\n".join(out) + "\n"


# --- Markdown convention (docs/AUTHORING.md) --------------------------------

def convert_markdown(text: str) -> str:
    title = "Untitled"
    serves = 1
    minutes = 0
    ingredients: list[quantities.ParsedIngredient] = []
    steps: list[tuple[int, str]] = []
    section = ""

    for raw in text.splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.startswith("# ") and not line.startswith("## "):
            title = line[2:].strip()
        elif line.lower().startswith("serves:"):
            serves = int(re.sub(r"\D", "", line) or 1)
        elif line.lower().startswith("time:"):
            minutes = quantities.parse_duration_seconds(line.split(":", 1)[1]) // 60
        elif line.startswith("## "):
            section = line[3:].strip().lower()
        elif section.startswith("ingredient") and line.startswith(("-", "*")):
            ingredients.append(quantities.parse_ingredient(line))
        elif section.startswith("step"):
            m = re.match(r"^\d+[.)]\s*(.*)$", line)
            if m:
                body = m.group(1)
                timer = 0
                tm = re.search(r"\[([^\]]+)\]\s*$", body)
                if tm:
                    timer = quantities.parse_duration_seconds(tm.group(1))
                    if timer:
                        body = body[: tm.start()].rstrip()
                steps.append((timer, body))

    if not steps:
        raise ValueError("no steps found (need a '## Steps' numbered list)")
    return _serialise(title, serves, minutes, ingredients, steps)


# --- schema.org Recipe JSON-LD ----------------------------------------------

class _JsonLdExtractor(HTMLParser):
    def __init__(self):
        super().__init__()
        self._in_jsonld = False
        self.blocks: list[str] = []
        self._buf: list[str] = []

    def handle_starttag(self, tag, attrs):
        if tag == "script" and dict(attrs).get("type", "") == "application/ld+json":
            self._in_jsonld = True
            self._buf = []

    def handle_endtag(self, tag):
        if tag == "script" and self._in_jsonld:
            self._in_jsonld = False
            self.blocks.append("".join(self._buf))

    def handle_data(self, data):
        if self._in_jsonld:
            self._buf.append(data)


def _find_recipe_node(data):
    """Recursively finds the first node whose @type includes Recipe."""
    if isinstance(data, dict):
        node_type = data.get("@type", "")
        types = node_type if isinstance(node_type, list) else [node_type]
        if any(t == "Recipe" for t in types):
            return data
        for value in data.values():
            found = _find_recipe_node(value)
            if found:
                return found
    elif isinstance(data, list):
        for item in data:
            found = _find_recipe_node(item)
            if found:
                return found
    return None


def _iso8601_minutes(value: str) -> int:
    m = re.match(r"^PT(?:(\d+)H)?(?:(\d+)M)?", value or "")
    if not m:
        return 0
    return int(m.group(1) or 0) * 60 + int(m.group(2) or 0)


def convert_jsonld_html(html: str) -> str:
    extractor = _JsonLdExtractor()
    extractor.feed(html)
    recipe = None
    for block in extractor.blocks:
        try:
            recipe = _find_recipe_node(json.loads(block))
        except json.JSONDecodeError:
            continue
        if recipe:
            break
    if not recipe:
        raise ValueError("no schema.org Recipe JSON-LD found in the page")

    title = recipe.get("name", "Untitled")
    yields = recipe.get("recipeYield", 1)
    if isinstance(yields, list):
        yields = yields[0]
    serves = int(re.sub(r"\D", "", str(yields)) or 1)
    minutes = _iso8601_minutes(recipe.get("totalTime", "")) or _iso8601_minutes(
        recipe.get("cookTime", ""))

    ingredients = [quantities.parse_ingredient(i)
                   for i in recipe.get("recipeIngredient", [])]

    steps: list[tuple[int, str]] = []
    for instr in recipe.get("recipeInstructions", []):
        if isinstance(instr, dict):
            text = instr.get("text", "")
        else:
            text = str(instr)
        text = re.sub(r"\s+", " ", text).strip()
        if text:
            steps.append((quantities.parse_duration_seconds(text) if "minute" in text.lower() else 0, text))
    if not steps:
        raise ValueError("recipe has no instructions")
    return _serialise(title, serves, minutes, ingredients, steps)


def fetch_html(source: str) -> str:
    if source.startswith(("http://", "https://")):
        req = urllib.request.Request(source, headers={"User-Agent": "inkrecipes/0.1"})
        with urllib.request.urlopen(req, timeout=20) as resp:
            return resp.read().decode("utf-8", "replace")
    return Path(source).read_text(encoding="utf-8")
