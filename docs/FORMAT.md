# The .rcp recipe format

A recipe is a UTF-8 text file at `/inkrecipes/<name>.rcp`.

Line 1: magic `RCP 1`. Then:

| Line | Meaning |
|---|---|
| `M title <text>` | recipe title |
| `M serves <n>` | the servings the quantities are written for |
| `M time <minutes>` | total time estimate |
| `I <qty>\|<unit>\|<name>` | ingredient; qty is a decimal or empty for "to taste" |
| `S <seconds>` | starts a step; 0 = no timer |
| `T <text>` | one pre-wrapped step line (40 columns for the big cooking type) |

Unknown tags are skipped. The companion wraps step text at build time;
cooking mode renders it at 3x scale (15x21 px glyphs), so 40 columns fits
both panels.

Scaling happens on device with `scaleQuantity` (host-tested): small
amounts snap to kitchen fractions (1/8, 1/4, 1/3, 1/2, 2/3, 3/4), 10-100
round to integers, 100+ to the nearest 10, and nothing ever scales to zero.
