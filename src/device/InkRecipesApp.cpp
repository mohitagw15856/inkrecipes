#include "device/InkRecipesApp.h"

#ifdef ARDUINO

#include <Arduino.h>

#include <inkkit/Storage.h>

namespace inkrecipes {

namespace {

constexpr const char* kAppRoot = "/inkrecipes";

// Ecosystem button convention. TODO(hardware-test): confirm indices; the
// checklist is designed for gloves, so every action is a single press.
constexpr uint8_t kBtnBack = 0;
constexpr uint8_t kBtnSelect = 1;
constexpr uint8_t kBtnLeft = 2;
constexpr uint8_t kBtnRight = 3;
constexpr uint8_t kBtnUp = 4;
constexpr uint8_t kBtnDown = 5;

constexpr int kCookScale = 3;  // 15x21 px glyphs: readable across a kitchen

}  // namespace

void InkRecipesApp::begin() {
  inkkit::sd::ensureDir(kAppRoot);
  refreshList();
  renderList(true);
}

void InkRecipesApp::refreshList() {
  names_.clear();
  inkkit::sd::listFiles(kAppRoot, ".rcp", [&](const std::string& path) {
    const size_t slash = path.find_last_of('/');
    names_.push_back(slash == std::string::npos ? path : path.substr(slash + 1));
  });
  if (listSel_ >= static_cast<int>(names_.size())) listSel_ = 0;
}

bool InkRecipesApp::loadRecipe(const std::string& name) {
  std::string text;
  if (!inkkit::sd::readWholeFile((std::string(kAppRoot) + "/" + name).c_str(), text)) {
    return false;
  }
  if (!parseRecipe(text, recipe_)) return false;
  servings_ = recipe_.serves;
  ingSel_ = 0;
  step_ = 0;
  timerRunning_ = false;
  return true;
}

void InkRecipesApp::tick() {
  buttons_.update();

  // Timer countdown repaint policy (e-ink honest): corner refresh every 15 s,
  // then every second inside the final 10 s, full-screen flash on zero.
  if (screen_ == Screen::Cooking && timerRunning_) {
    const uint32_t now = millis();
    if (now >= timerEndMs_) {
      timerRunning_ = false;
      flashComplete();
      renderCooking(true);
    } else {
      const uint32_t remaining = timerEndMs_ - now;
      const uint32_t interval = remaining <= 10000 ? 1000 : 15000;
      if (now - lastTimerPaintMs_ >= interval) {
        lastTimerPaintMs_ = now;
        renderCooking(false);
      }
    }
  }

  switch (screen_) {
    case Screen::List: {
      if (buttons_.wasPressed(kBtnDown) && listSel_ + 1 < static_cast<int>(names_.size())) {
        ++listSel_;
        renderList(false);
      } else if (buttons_.wasPressed(kBtnUp) && listSel_ > 0) {
        --listSel_;
        renderList(false);
      } else if (buttons_.wasPressed(kBtnSelect) && !names_.empty()) {
        if (loadRecipe(names_[static_cast<size_t>(listSel_)])) {
          screen_ = Screen::Ingredients;
          renderIngredients(true);
        }
      }
      break;
    }
    case Screen::Ingredients: {
      if (buttons_.wasPressed(kBtnDown) &&
          ingSel_ + 1 < static_cast<int>(recipe_.ingredients.size())) {
        ++ingSel_;
        renderIngredients(false);
      } else if (buttons_.wasPressed(kBtnUp) && ingSel_ > 0) {
        --ingSel_;
        renderIngredients(false);
      } else if (buttons_.wasPressed(kBtnSelect) && !recipe_.ingredients.empty()) {
        auto& ing = recipe_.ingredients[static_cast<size_t>(ingSel_)];
        ing.ticked = !ing.ticked;
        renderIngredients(false);
      } else if (buttons_.wasPressed(kBtnRight)) {
        if (servings_ < 12) {
          ++servings_;
          renderIngredients(false);
        } else {
          screen_ = Screen::Cooking;
          renderCooking(true);
        }
      } else if (buttons_.wasPressed(kBtnLeft) && servings_ > 1) {
        --servings_;
        renderIngredients(false);
      } else if (buttons_.wasPressed(kBtnBack)) {
        screen_ = Screen::List;
        renderList(true);
      }
      break;
    }
    case Screen::Cooking: {
      if (buttons_.wasPressed(kBtnRight) && step_ + 1 < recipe_.steps.size()) {
        ++step_;
        timerRunning_ = false;
        renderCooking(true);  // page change: full refresh
      } else if (buttons_.wasPressed(kBtnLeft) && step_ > 0) {
        --step_;
        timerRunning_ = false;
        renderCooking(true);
      } else if (buttons_.wasPressed(kBtnSelect)) {
        const Step& s = recipe_.steps[step_];
        if (s.seconds > 0 && !timerRunning_) {
          timerRunning_ = true;
          timerEndMs_ = millis() + s.seconds * 1000u;
          lastTimerPaintMs_ = 0;
          renderCooking(false);
        }
      } else if (buttons_.wasPressed(kBtnBack)) {
        // The explicit exit; keep-awake ends here.
        timerRunning_ = false;
        screen_ = Screen::Ingredients;
        renderIngredients(true);
      }
      break;
    }
  }
}

void InkRecipesApp::renderList(bool full) {
  tr_.clear();
  tr_.textInverted(8, 6, "InkRecipes  (SELECT opens)", tr_.lineHeight() + 6);
  int y = 2 * tr_.lineHeight() + 10;
  if (names_.empty()) {
    tr_.text(8, y, "No recipes in /inkrecipes.");
    tr_.text(8, y + tr_.lineHeight(), "Build some with: inkrecipes convert");
  }
  for (size_t i = 0; i < names_.size(); ++i) {
    const std::string marker = (static_cast<int>(i) == listSel_) ? "> " : "  ";
    tr_.text(8, y, marker + names_[i]);
    y += tr_.lineHeight();
  }
  tr_.flush(full);
}

void InkRecipesApp::renderIngredients(bool full) {
  tr_.clear();
  std::string head = recipe_.title + "  serves " + std::to_string(servings_);
  if (recipe_.minutes > 0) head += "  ~" + std::to_string(recipe_.minutes) + " min";
  tr_.textInverted(8, 6, head, tr_.lineHeight() + 6);
  tr_.text(8, tr_.lineHeight() + 12,
           "LEFT/RIGHT servings, SELECT ticks, RIGHT at max starts cooking");

  int y = 3 * tr_.lineHeight() + 8;
  for (size_t i = 0; i < recipe_.ingredients.size(); ++i) {
    if (y + tr_.lineHeight() > tr_.height() - 8) break;
    const auto& ing = recipe_.ingredients[i];
    std::string line = (static_cast<int>(i) == ingSel_) ? "> " : "  ";
    line += ing.ticked ? "[x] " : "[ ] ";
    const double scaled = scaleQuantity(ing.quantity, recipe_.serves, servings_);
    const std::string qty = formatQuantity(scaled);
    if (!qty.empty()) {
      line += qty;
      if (!ing.unit.empty()) line += " " + ing.unit;
      line += " ";
    }
    line += ing.name;
    tr_.text(8, y, line);
    y += tr_.lineHeight();
  }
  tr_.flush(full);
}

void InkRecipesApp::renderCooking(bool full) {
  tr_.clear();
  const Step& s = recipe_.steps[step_];

  char head[48];
  snprintf(head, sizeof(head), "Step %zu/%zu", step_ + 1, recipe_.steps.size());
  tr_.textInverted(8, 6, head, tr_.lineHeight() + 6);

  int y = tr_.lineHeight() + 18;
  for (const auto& line : s.lines) {
    if (y + tr_.lineHeightScaled(kCookScale) > tr_.height() - tr_.lineHeight()) break;
    tr_.textScaled(10, y, line, kCookScale);
    y += tr_.lineHeightScaled(kCookScale);
  }

  if (s.seconds > 0 && !timerRunning_) {
    tr_.text(8, tr_.height() - tr_.lineHeight() - 4,
             "SELECT starts a " + std::to_string(s.seconds / 60) + ":" +
                 (s.seconds % 60 < 10 ? "0" : "") + std::to_string(s.seconds % 60) +
                 " timer");
  }
  if (timerRunning_) renderTimerCorner();
  tr_.flush(full);
}

void InkRecipesApp::renderTimerCorner() {
  const uint32_t now = millis();
  const uint32_t remaining = timerEndMs_ > now ? (timerEndMs_ - now + 999) / 1000 : 0;
  char t[16];
  snprintf(t, sizeof(t), "%lu:%02lu", static_cast<unsigned long>(remaining / 60),
           static_cast<unsigned long>(remaining % 60));
  const int w = 5 * (5 + 1) * 2 + 8;
  tr_.textScaled(tr_.width() - w, 8, t, 2);
}

void InkRecipesApp::flashComplete() {
  // E-ink friendly alert: three full-contrast flips beat any sound we lack.
  // TODO(hardware-test): confirm three full refreshes reads as an alert and
  // does not stress the panel.
  for (int i = 0; i < 2; ++i) {
    tr_.clear();
    tr_.textInverted(0, tr_.height() / 2 - 12, std::string(" TIME! "), 24);
    tr_.flush(true);
  }
}

}  // namespace inkrecipes

#endif  // ARDUINO
