# Authoring recipes in Markdown

`inkrecipes convert my-recipe.md --out my-recipe.rcp` reads this convention:

```markdown
# Recipe title

Serves: 2
Time: 25 min

## Ingredients
- 300 g kimchi, chopped
- 1 1/2 cups cooked rice
- salt, to taste

## Steps
1. Do the first thing.
2. Fry until it smells sweet. [4 min]
3. Serve.
```

- Quantities understand decimals, fractions (`1 1/2`, `½`) and metric or
  imperial units (g, kg, ml, l, tsp, tbsp, cup, oz, lb, ...). Lines that
  start without a number ("salt, to taste") keep an empty quantity and are
  never scaled.
- A trailing `[N min]` (or `[90 s]`, `[1 hour 10 min]`) on a step becomes
  that step's countdown timer.

`inkrecipes convert https://example.com/recipe --out x.rcp` alternatively
scrapes schema.org Recipe JSON-LD from a page (most recipe sites), taking
name, yield, total time, ingredients and instruction steps.
