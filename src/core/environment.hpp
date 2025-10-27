#pragma once

#include "sol.hpp"

namespace LuaProc
{
struct Window;
class LuaProc;

// ENVIRONMENT (Not implemented)
// delay, displayDensity
// frameCount
// noSmooth
// pixelDensity, pixelHeight, pixelWidth
// settings, smooth
// windowMoved, windowRatio, windowResized

// ENVIRONMENT (API changes)
// frameRate variable and function merged

namespace Environment
{
void cursor(const std::vector<sol::object> &va);
int displayHeight(const std::vector<sol::object> &va);
int displayWidth(const std::vector<sol::object> &va);
bool focused(const std::vector<sol::object> &va);
void fullScreen(const std::vector<sol::object> &va, Window &window);
int frameRate(const std::vector<sol::object> &va, Window &window);
int height(const std::vector<sol::object> &va);
void noCursor(const std::vector<sol::object> &va);
void size(const std::vector<sol::object> &va, Window &window);
int width(const std::vector<sol::object> &va);
void windowMove(const std::vector<sol::object> &va);
void windowResizable(const std::vector<sol::object> &va, Window &window);
void windowResize(const std::vector<sol::object> &va);
void windowTitle(const std::vector<sol::object> &va, Window &window);
}
}