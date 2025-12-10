#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace LightsCamera
{
void setupLightsCamera(std::shared_ptr<Lua> luaptr);
}
}