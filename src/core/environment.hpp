#pragma once

#include "sol.hpp"

namespace LuaProc
{
struct Window;

// ENVIRONMENT
// Not implemented:
// cursor
// displayDensity, delay
// frameCount
// noCursor, noSmooth
// pixelDensity, pixelHeight, pixelWidth
// settings, smooth
// windowMoved, windowRatio, windowResized
namespace Environment
{
int displayHeight(sol::variadic_args va);
int displayWidth(sol::variadic_args va);
bool focused(sol::variadic_args va);
void fullScreen(sol::variadic_args va, Window &window);
void frameRate(sol::variadic_args va, Window &window);
int height(sol::variadic_args va);
void size(sol::variadic_args va, Window &window);
int width(sol::variadic_args va);
void windowMove(sol::variadic_args va);
void windowResizable(sol::variadic_args va, Window &window);
void windowResize(sol::variadic_args va);
void windowTitle(sol::variadic_args va, Window &window);
}
}