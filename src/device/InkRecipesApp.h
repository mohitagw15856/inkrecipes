// InkRecipes screens: recipe list, ingredients checklist with a servings
// scaler, and cooking mode (one step per page in very large type, per-step
// countdown timers, keep-awake until explicit exit).
#pragma once

#ifdef ARDUINO

#include <string>
#include <vector>

#include <inkkit/Buttons.h>

#include "core/Recipe.h"
#include "device/TextRenderer.h"

namespace inkrecipes {

class InkRecipesApp {
 public:
  InkRecipesApp(TextRenderer& tr, inkkit::Buttons& buttons) : tr_(tr), buttons_(buttons) {}

  void begin();
  void tick();

  // Cooking mode keeps the device awake; main.cpp consults this before
  // honouring any idle sleep policy.
  bool keepAwake() const { return screen_ == Screen::Cooking; }

 private:
  enum class Screen : uint8_t { List, Ingredients, Cooking };

  void refreshList();
  bool loadRecipe(const std::string& name);

  void renderList(bool full);
  void renderIngredients(bool full);
  void renderCooking(bool full);
  void renderTimerCorner();
  void flashComplete();

  TextRenderer& tr_;
  inkkit::Buttons& buttons_;

  Screen screen_ = Screen::List;
  std::vector<std::string> names_;
  int listSel_ = 0;

  Recipe recipe_;
  int servings_ = 1;
  int ingSel_ = 0;
  size_t step_ = 0;

  bool timerRunning_ = false;
  uint32_t timerEndMs_ = 0;
  uint32_t lastTimerPaintMs_ = 0;
};

}  // namespace inkrecipes

#endif  // ARDUINO
