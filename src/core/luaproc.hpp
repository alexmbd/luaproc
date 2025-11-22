#pragma once

#include "funchandler.hpp"

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