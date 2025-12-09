#pragma once

#include "safesol.hpp"

#include "raylib.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace LuaProc
{
struct Window
{
    int width         = -1;
    int height        = -1;
    int frameRate     = 60;
    int flags         = 0;
    std::string title = "LuaProc";
};

struct Canvas
{
    enum class Renderer
    {
        P2D,
        P3D
    };

    enum class ColorMode
    {
        RGB,
        HSB
    };

    Renderer renderer   = Canvas::Renderer::P2D;
    ColorMode colorMode = Canvas::ColorMode::RGB;
    Camera3D camera     = {0};

    Color background    = Color{128, 128, 128, 255};
    Color fill          = Color{255, 255, 255, 255};
    Color stroke        = Color{255, 255, 255, 255};
    bool noFill         = false;
    bool noStroke       = false;
};

struct Lua
{
    enum class State
    {
        Setup,
        PostSetup,
        Draw
    };

    struct PostSetupFunction
    {
        std::function<void(const std::vector<sol::object> &)> function;
        std::vector<sol::object> args;
    };

    sol::state lua;
    Window window;
    Canvas canvas;
    State state;
    std::vector<PostSetupFunction> postSetupFuncs;

    void update();
    void draw();
};

void setup(std::shared_ptr<Lua> luaptr);
}