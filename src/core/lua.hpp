#pragma once

#include "safesol.hpp"

#include "raylib.h"

#include <functional>
#include <string>
#include <vector>

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

struct Canvas
{
    Color background;
};

class Lua
{
  public:
    enum class State
    {
        Setup,
        PostSetup,
        Draw
    };

    Lua() = default;
    Lua(const char *filename);

    const Window &window() const;
    const Canvas &canvas() const;

    void setScript(const char *filename);
    void draw();

  private:
    struct PostSetupFunction
    {
        std::function<void(const std::vector<sol::object> &)> function;
        std::vector<sol::object> args;
    };

    sol::state m_lua;
    Window m_window;
    Canvas m_canvas;
    State m_state;
    std::vector<PostSetupFunction> m_postSetupFuncs;

    void setup();

    void setupColor();
    void setupEnvironment();
    void setupMath();
    void setupOutput();
    void setupShape();
    void setupTransform();
};
}