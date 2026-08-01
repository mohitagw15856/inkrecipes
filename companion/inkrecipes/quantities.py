"""Unit-aware quantity parsing (metric and imperial)."""

from __future__ import annotations

import re
from dataclasses import dataclass

# Canonical unit spellings; parsing is case-insensitive and plural-tolerant.
UNITS = {
    "g": ["g", "gram", "grams"],
    "kg": ["kg", "kilogram", "kilograms"],
    "ml": ["ml", "milliliter", "milliliters", "millilitre", "millilitres"],
    "l": ["l", "liter", "liters", "litre", "litres"],
    "tsp": ["tsp", "teaspoon", "teaspoons"],
    "tbsp": ["tbsp", "tablespoon", "tablespoons"],
    "cup": ["cup", "cups"],
    "oz": ["oz", "ounce", "ounces"],
    "lb": ["lb", "lbs", "pound", "pounds"],
    "pinch": ["pinch", "pinches"],
    "clove": ["clove", "cloves"],
    "piece": ["piece", "pieces"],
    "sheet": ["sheet", "sheets"],
    "can": ["can", "cans"],
}
_UNIT_LOOKUP = {alias: canon for canon, aliases in UNITS.items() for alias in aliases}

_VULGAR = {"½": 0.5, "¼": 0.25, "¾": 0.75, "⅓": 1 / 3, "⅔": 2 / 3, "⅛": 0.125}

_QTY = re.compile(
    r"^\s*(?P<whole>\d+)?\s*(?P<vulgar>[½¼¾⅓⅔⅛])?\s*"
    r"(?:(?P<num>\d+)\s*/\s*(?P<den>\d+))?\s*(?P<dec>\.\d+)?"
)


@dataclass
class ParsedIngredient:
    quantity: float  # 0.0 when absent ("salt, to taste")
    unit: str        # canonical, may be ""
    name: str


def parse_quantity(text: str) -> tuple[float, str]:
    """Parses a leading quantity, returning (value, rest-of-string)."""
    m = _QTY.match(text)
    value = 0.0
    end = 0
    if m and (m.group("whole") or m.group("vulgar") or m.group("num") or m.group("dec")):
        end = m.end()
        if m.group("whole"):
            value += float(m.group("whole"))
        if m.group("dec"):
            value += float("0" + m.group("dec"))
        if m.group("vulgar"):
            value += _VULGAR[m.group("vulgar")]
        if m.group("num") and m.group("den") and int(m.group("den")) != 0:
            value += float(m.group("num")) / float(m.group("den"))
    return value, text[end:].strip()


def parse_ingredient(line: str) -> ParsedIngredient:
    """Parses lines like "1 1/2 cups cooked rice" or "salt, to taste"."""
    line = line.strip().lstrip("-*").strip()
    qty, rest = parse_quantity(line)
    unit = ""
    if qty > 0 and rest:
        first, _, remainder = rest.partition(" ")
        canon = _UNIT_LOOKUP.get(first.lower().rstrip("."))
        if canon:
            unit = canon
            rest = remainder.strip()
    return ParsedIngredient(quantity=qty, unit=unit, name=rest)


def parse_duration_seconds(text: str) -> int:
    """Extracts a duration like "5 min", "1 hour 20 minutes", "90 s"."""
    total = 0
    for value, unit in re.findall(r"(\d+)\s*(hours?|hrs?|h|minutes?|mins?|m|seconds?|secs?|s)\b",
                                  text, flags=re.IGNORECASE):
        n = int(value)
        u = unit.lower()
        if u.startswith("h"):
            total += n * 3600
        elif u.startswith("m"):
            total += n * 60
        else:
            total += n
    return total
