#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace Environment
{
void setupEnvironment(std::shared_ptr<Lua> luaptr);
}
}