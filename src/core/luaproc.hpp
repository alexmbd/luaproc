#pragma once

#include "funchandler.hpp"

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

    enum MouseCursor
    {
        DEFAULT       = 0, // Default pointer shape
        ARROW         = 1, // Arrow shape
        IBEAM         = 2, // Text writing cursor shape
        CROSSHAIR     = 3, // Cross shape
        POINTING_HAND = 4, // Pointing hand cursor
        RESIZE_EW     = 5, // Horizontal resize/move arrow shape
        RESIZE_NS     = 6, // Vertical resize/move arrow shape
        RESIZE_NWSE   = 7, // Top-left to bottom-right diagonal resize/move arrow shape
        RESIZE_NESW   = 8, // The top-right to bottom-left diagonal resize/move arrow shape
        RESIZE_ALL    = 9, // The omnidirectional resize/move cursor shape
        NOT_ALLOWED   = 10 // The operation-not-allowed shape
    };
};

class LuaProc
{
  public:
    enum class State
    {
        Setup,
        PostSetup,
        Draw
    };

    LuaProc();
    ~LuaProc();

    void run();

  private:
    std::unique_ptr<sol::state> m_lua;
    State m_currentState;
    Window m_window;
    FunctionHandler<PostSetupVariant> m_postSetupFuncs;

    void setupOutput();
    void setupEnvironment();
};
}