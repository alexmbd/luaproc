#pragma once

#include <memory>

namespace sol
{
class state;
}

namespace LuaProc
{
struct Window
{
    int width;
    int height;
    int frameRate;
    int flags;
    std::string title;
};

class LuaProc
{
  public:
    LuaProc();
    ~LuaProc();

    void run();

  private:
    std::unique_ptr<sol::state> m_lua;
    Window m_window;

    void setupOutput();
    void setupEnvironment();
};
}