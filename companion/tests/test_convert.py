from pathlib import Path

import pytest

from inkrecipes import convert
from inkrecipes.cli import main

REPO = Path(__file__).resolve().parents[2]
FIX = Path(__file__).parent / "fixtures"


def test_markdown_conversion_of_shipped_recipe():
    doc = convert.convert_markdown((REPO / "recipes/src/kimchi-fried-rice.md").read_text())
    lines = doc.splitlines()
    assert lines[0] == "RCP 1"
    assert "M title Kimchi fried rice (kimchi bokkeumbap)" in lines
    assert "M serves 2" in lines
    assert "I 300|g|kimchi, well fermented, chopped" in lines
    assert "S 240" in lines  # [4 min]
    assert "S 0" in lines


def test_markdown_requires_steps():
    with pytest.raises(ValueError):
        convert.convert_markdown("# T\n\n## Ingredients\n- 1 g x\n")


def test_jsonld_conversion():
    doc = convert.convert_jsonld_html((FIX / "recipe_page.html").read_text())
    lines = doc.splitlines()
    assert "M title Fixture Soup" in lines
    assert "M serves 4" in lines
    assert "M time 45" in lines
    assert "I 2|tbsp|olive oil" in lines
    assert any(ln.startswith("T Simmer everything") for ln in lines)


def test_jsonld_missing_recipe():
    with pytest.raises(ValueError):
        convert.convert_jsonld_html("<html><body>plain page</body></html>")


def test_cli_roundtrip(tmp_path):
    out = tmp_path / "kimchi.rcp"
    rc = main(["convert", str(REPO / "recipes/src/kimchi-fried-rice.md"), "--out", str(out)])
    assert rc == 0
    assert out.read_text().startswith("RCP 1\n")
