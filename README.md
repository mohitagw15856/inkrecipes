# InkRecipes

**Kitchen-mode recipe reader for the Xteink X4/X3.** Big text, gloves-on
button navigation, one step per page, and per-step countdown timers with an
e-ink friendly full-screen flash when they finish.

Status: builds in CI, not yet verified on device.

Compatible with the CrossPoint ecosystem: same hardware, device layer from
[inkkit](https://github.com/mohitagw15856/inkkit) (pinned in
`platformio.ini`), recipes under `/inkrecipes` on the SD card.

## Features

- Ingredients checklist with button-toggle ticks and a servings scaler
  (LEFT/RIGHT, 1-12) using sensible kitchen-fraction rounding (host-tested
  pure logic)
- Cooking mode: one numbered step per page in 3x-scaled type, next/prev on
  the side buttons
- Step timers: SELECT starts the countdown shown in a corner (repainted
  every 15 s, per-second in the final 10), triple full-contrast flash on
  completion
- Keep-awake while cooking, explicit BACK to exit; power-hold is ignored
  mid-cook so a pocket press cannot kill a simmer timer

Five sample recipes ship as Markdown source (`recipes/src/`) and compiled
`.rcp` (`recipes/built/`), including kimchi fried rice.

## Install and flash

```sh
pio run -e xteink_x4              # or -e xteink_x3; run one at a time
pio run -e xteink_x4 -t upload
```

Copy `recipes/built/*.rcp` to `/inkrecipes/` on the SD card.

## Companion

```sh
pip install -e "companion[test]"
inkrecipes convert recipes/src/kimchi-fried-rice.md --out kimchi.rcp
inkrecipes convert https://your-favourite-site/recipe --out scraped.rcp
```

The converter reads the Markdown convention in
[docs/AUTHORING.md](docs/AUTHORING.md) or scrapes schema.org Recipe JSON-LD
from a URL, with unit-aware quantity parsing (metric and imperial).

## Documentation

- [docs/FORMAT.md](docs/FORMAT.md) - the .rcp format and scaling rules
- [docs/AUTHORING.md](docs/AUTHORING.md) - writing recipes in Markdown
- [docs/HARDWARE_TESTING.md](docs/HARDWARE_TESTING.md) - on-device checklist
- [docs/INKKIT_GAPS.md](docs/INKKIT_GAPS.md) - device-layer gaps for inkkit
- [ARCHITECTURE.md](ARCHITECTURE.md)

## Licence

MIT, see [LICENSE](LICENSE). The sample recipes are original text written
for this repo. The 5x7 text renderer is shared ecosystem code originating
in InkQuest (same author, MIT).
