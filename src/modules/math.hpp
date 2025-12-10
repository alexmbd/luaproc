#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace Math
{
void setupMath(std::shared_ptr<Lua> luaptr);
}
}