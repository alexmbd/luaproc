#pragma once

#include <memory>

namespace LuaProc
{
struct Lua;

namespace TransformNS
{
void setupTransform(std::shared_ptr<Lua> luaptr);
}
}