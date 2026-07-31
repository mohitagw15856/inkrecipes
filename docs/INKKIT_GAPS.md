# inkkit gaps

InkRecipes depends on inkkit v0.1.0-rc1. Gaps found:

## 1. No text or font engine, and no larger type (seventh app)

Cooking mode needs genuinely large type; InkRecipes integer-scales the
copied 5x7 font to 3x (blocky but readable). A real inkkit text module
with proper large faces (the ecosystem .cpfont system) would look far
better on a kitchen counter.

## 2. No keep-awake/idle contract

The spec's keep-awake requirement can only be expressed as "the app never
calls deepSleep while cooking". Whether the vendored power layer has its
own idle behaviour that needs suppressing is unknowable without hardware;
inkkit documents no idle/keep-awake contract. TODO(hardware-test) carried
in main.cpp.

## 3. No timer/alarm surface

A countdown that must survive display refreshes wants a monotonic timer
helper and, ideally, a wake-from-light-sleep alarm so the panel could rest
between updates. InkRecipes polls millis() instead.

## 4. No key-value settings helper (fourth app)
