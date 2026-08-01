# Hardware testing checklist

The recipe core (parser, scaler) is host-tested; the shipped recipes are
parse-validated in the same suite. Device items, each a TODO(hardware-test):

## Build and flash

```sh
pio run -e xteink_x4            # run envs one at a time
pio run -e xteink_x4 -t upload
```

Copy `recipes/built/*.rcp` to `/inkrecipes/` on the SD card.

## Items to verify on device

- Boot order; recipe list; checklist tick flow with gloves on.
- Servings scaler on screen: quantities re-render correctly at 1-12 covers.
- Cooking mode type size at arm's length; 40-column wrap on both panels.
- Timer cadence: corner updates every 15 s (every second in the last 10),
  the triple full-refresh completion flash, and panel health across many
  flashes.
- Keep-awake: no idle behaviour interrupts a 20-minute simmer step; power
  hold is ignored mid-cook (by design; confirm this is the right call).
- BACK exits cooking mode explicitly and restores normal sleep behaviour.
