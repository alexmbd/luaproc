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
    int width              = -1;
    int height             = -1;
    int frameRate          = 60;
    int flags              = 0;
    std::size_t frameCount = 0;
    std::string title      = "LuaProc";
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

    Renderer renderer    = Canvas::Renderer::P2D;
    ColorMode colorMode  = Canvas::ColorMode::RGB;
    Camera2D camera2D    = {0};
    Camera3D camera3D    = {0};
    float zOrder         = 0.0f; // Used for 2D drawing in a Camera3D context

    Color background     = Color{128, 128, 128, 255};
    Color fill           = Color{255, 255, 255, 255};
    Color stroke         = Color{0, 0, 0, 255};
    bool noFill          = false;
    bool noStroke        = false;
    bool needToPopMatrix = false;
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

void setupScript(std::shared_ptr<Lua> luaptr, const std::string &filename);
}