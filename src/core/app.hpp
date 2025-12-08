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
    std::shared_ptr<Lua> m_lua;
};
}