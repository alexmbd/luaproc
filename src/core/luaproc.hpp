#pragma once

#include "funchandler.hpp"

#include "raylib.h"

namespace LuaProc
{
struct Window
{
    int width;
    int height;
    int frameRate;
    int flags;
    std::string title;
    Color background;
};

class Application
{
  public:
    enum class State
    {
        Setup,
        PostSetup,
        Draw
    };

    Application();
    ~Application();

    void run();

    sol::state &lua();
    Window &window();

    bool isState(State state) const;
    void addToPostSetup(Function<PostSetupVariant> func);

  private:
    sol::state m_lua;
    Window m_window;
    State m_currentState;
    FunctionHandler<PostSetupVariant> m_postSetupFuncs;
};
}