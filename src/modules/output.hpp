#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace Output
{
void setupOutput(std::shared_ptr<Lua> luaptr);
}
}