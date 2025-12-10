#pragma once

#include "lua.hpp"

namespace LuaProc
{
class Application
{
  public:
    Application(const std::string &filename);
    ~Application();

    void run();

  private:
    std::shared_ptr<Lua> m_lua;
};
}