# InkRecipes architecture

## Fork vs standalone

Standalone firmware on inkkit, like the rest of the ecosystem: a kitchen
reader shares no logic with an EPUB reader, and the device layer is common
via the pinned inkkit tag.

## Design decisions

- **Two screens per recipe, exactly as specified.** Ingredients checklist
  (with the servings scaler on LEFT/RIGHT) and cooking mode (one step per
  page). No nesting deeper than that: kitchen UIs fail when they need
  precision taps, so every action is one press of a physical button.
- **Big type by integer scaling.** Cooking mode renders the ecosystem 5x7
  font at 3x. Blocky but unambiguous at a distance; a proper large face is
  recorded as an inkkit gap rather than half-solved here.
- **Scaler is pure and device-side.** `scaleQuantity`/`formatQuantity`
  live in src/core with host tests: fraction snapping for small amounts,
  integer/decade rounding above, never scaling an ingredient to zero. The
  companion normalises quantities to decimals at build time so the device
  never parses "1 ½ cups".
- **E-ink honest timers.** A countdown repaints the corner every 15 s and
  only goes per-second inside the final 10; completion is a triple
  full-contrast flash (the panel's loudest available "sound").
- **Companion does the parsing.** Unit-aware quantity parsing (metric and
  imperial, vulgar fractions) and the two input paths (Markdown
  convention, schema.org JSON-LD scraping) are Python with pytest; the
  device reads only the pre-chewed .rcp.
