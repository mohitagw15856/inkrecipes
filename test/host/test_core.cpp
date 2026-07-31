// Host tests: .rcp parsing and the servings scaler.
#include <cmath>
#include <cstdio>
#include <string>

#include "core/Recipe.h"

using namespace inkrecipes;

static int g_checks = 0;
#define CHECK(cond)                                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    if (!(cond)) {                                                         \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      return 1;                                                            \
    }                                                                      \
  } while (0)

static const char* kDoc =
    "RCP 1\n"
    "M title Kimchi fried rice\n"
    "M serves 2\n"
    "M time 25\n"
    "I 300|g|kimchi, chopped\n"
    "I 2|tbsp|gochujang\n"
    "I 0.5|tsp|sesame oil\n"
    "I ||salt, to taste\n"
    "S 0\n"
    "T Chop the kimchi and drain,\n"
    "T keeping the juice.\n"
    "S 300\n"
    "T Fry rice on high heat.\n";

static int testParse() {
  Recipe r;
  CHECK(parseRecipe(kDoc, r));
  CHECK(r.title == "Kimchi fried rice");
  CHECK(r.serves == 2);
  CHECK(r.minutes == 25);
  CHECK(r.ingredients.size() == 4);
  CHECK(r.ingredients[0].quantity == 300 && r.ingredients[0].unit == "g");
  CHECK(r.ingredients[3].quantity == 0 && r.ingredients[3].name == "salt, to taste");
  CHECK(r.steps.size() == 2);
  CHECK(r.steps[0].seconds == 0 && r.steps[0].lines.size() == 2);
  CHECK(r.steps[1].seconds == 300);
  return 0;
}

static int testParseRejects() {
  Recipe r;
  CHECK(!parseRecipe("RCP 2\nS 0\nT x\n", r));
  CHECK(!parseRecipe("RCP 1\nM title no steps\n", r));
  return 0;
}

static int near(double a, double b) { return std::fabs(a - b) < 1e-6; }

static int testScaler() {
  // Grams: doubles cleanly, big values snap to 10s.
  CHECK(near(scaleQuantity(300, 2, 4), 600));
  CHECK(near(scaleQuantity(125, 2, 3), 190));   // 187.5 -> nearest 10
  CHECK(near(scaleQuantity(30, 2, 3), 45));     // mid range -> integer
  // Small quantities snap to kitchen fractions.
  CHECK(near(scaleQuantity(2, 2, 3), 3));
  CHECK(near(scaleQuantity(0.5, 2, 1), 0.25));
  CHECK(near(scaleQuantity(0.5, 2, 3), 0.75));
  CHECK(near(scaleQuantity(1.5, 2, 3), 2.25));
  CHECK(near(scaleQuantity(1.0, 3, 4), 1 + 1.0 / 3.0));  // 1.333 -> 1 1/3
  // Never scaled to zero.
  CHECK(scaleQuantity(0.25, 8, 1) > 0);
  // No-quantity ingredients pass through.
  CHECK(near(scaleQuantity(0, 2, 4), 0));
  return 0;
}

static int testFormat() {
  CHECK(formatQuantity(0) == "");
  CHECK(formatQuantity(3) == "3");
  CHECK(formatQuantity(0.5) == "1/2");
  CHECK(formatQuantity(1.5) == "1 1/2");
  CHECK(formatQuantity(2.25) == "2 1/4");
  CHECK(formatQuantity(1 + 1.0 / 3.0) == "1 1/3");
  CHECK(formatQuantity(600) == "600");
  return 0;
}

#include <fstream>
#include <sstream>

static int validateFile(const char* path) {
  std::ifstream in(path);
  CHECK(in.good());
  std::stringstream ss;
  ss << in.rdbuf();
  Recipe r;
  CHECK(parseRecipe(ss.str(), r));
  CHECK(!r.title.empty());
  CHECK(!r.ingredients.empty());
  std::printf("  recipe ok: %s (%zu ingredients, %zu steps)\n", path,
              r.ingredients.size(), r.steps.size());
  return 0;
}

int main(int argc, char** argv) {
  if (testParse()) return 1;
  if (testParseRejects()) return 1;
  if (testScaler()) return 1;
  if (testFormat()) return 1;
  for (int i = 1; i < argc; ++i) {
    if (validateFile(argv[i])) return 1;
  }
  std::printf("%d checks, 0 failures\n", g_checks);
  return 0;
}
