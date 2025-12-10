#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace Shape
{
void setupShape(std::shared_ptr<Lua> luaptr);
}
}