#pragma once

namespace LuaProc
{
class Application;

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
void setup(LuaProc::Application &app);
}
}