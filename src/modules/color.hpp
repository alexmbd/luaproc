#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace ColorNS
{
void setupColor(std::shared_ptr<Lua> luaptr);
}
}