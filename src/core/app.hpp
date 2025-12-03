#pragma once

#include "lua.hpp"

namespace LuaProc
{
class Application
{
  public:
    Application();
    ~Application();

    void run();

  private:
    Lua m_lua;
};
}