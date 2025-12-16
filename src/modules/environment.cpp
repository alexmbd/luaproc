#include "environment.hpp"
#include "core/constants.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

#include "rlgl.h"

namespace LuaProc
{
namespace Environment
{
void cursor(const std::vector<sol::object> &va)
{
    if (va.size() == 0) { return ShowCursor(); }

    checkArgSize("cursor", 1, va.size());
    checkArgType("cursor", va, sol::type::number);

    int cursorType = va[0].as<int>();

    ShowCursor();
    if (cursorType > MOUSE_CURSOR_NOT_ALLOWED || cursorType < 0)
    {
        // Just set to the default cursor if an invalid number was passed (no error thrown)
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        std::println("[LUAPROC WARNING] '{}' was passed as an invalid argument to '{}'. Using default cursor", cursorType, "cursor");
    }
    else
    {
        SetMouseCursor(cursorType);
    }
}

void noCursor(const std::vector<sol::object> &va)
{
    checkArgSize("noCursor", 0, va.size());
    HideCursor();
};

// ---------- ENVIRONMENT ----------
// ENVIRONMENT (Not implemented)
// delay, displayDensity
// frameCount
// noSmooth
// pixelDensity, pixelHeight, pixelWidth
// settings, smooth
// windowMoved, windowRatio, windowResized

// ENVIRONMENT (API changes)
// frameCount changed from a variable to a function
// frameRate variable and function merged

void setupEnvironment(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua      = luaptr->lua;
    lua["DEFAULT"]       = MOUSE_CURSOR_DEFAULT;
    lua["ARROW"]         = MOUSE_CURSOR_ARROW;
    lua["IBEAM"]         = MOUSE_CURSOR_IBEAM;
    lua["CROSSHAIR"]     = MOUSE_CURSOR_CROSSHAIR;
    lua["POINTING_HAND"] = MOUSE_CURSOR_POINTING_HAND;
    lua["RESIZE_EW"]     = MOUSE_CURSOR_RESIZE_EW;
    lua["RESIZE_NS"]     = MOUSE_CURSOR_RESIZE_NS;
    lua["RESIZE_NWSE"]   = MOUSE_CURSOR_RESIZE_NWSE;
    lua["RESIZE_NESW"]   = MOUSE_CURSOR_RESIZE_NESW;
    lua["RESIZE_ALL"]    = MOUSE_CURSOR_RESIZE_ALL;
    lua["NOT_ALLOWED"]   = MOUSE_CURSOR_NOT_ALLOWED;

    lua["P2D"]           = static_cast<int>(Canvas::Renderer::P2D);
    lua["P3D"]           = static_cast<int>(Canvas::Renderer::P3D);

    lua["cursor"]        = [luaptr](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (luaptr->state == Lua::State::Setup)
        {
            luaptr->postSetupFuncs.emplace_back(Environment::cursor, vec);
            return;
        }
        Environment::cursor(vec);
    };

    lua["displayHeight"] = [](sol::variadic_args va) {
        checkArgSize("displayHeight", 0, va.size());
        return GetMonitorHeight(0);
    };

    lua["displayWidth"] = [](sol::variadic_args va) {
        checkArgSize("displayWidth", 0, va.size());
        return GetMonitorWidth(0);
    };

    lua["focused"] = [](sol::variadic_args va) {
        checkArgSize("focused", 0, va.size());
        return IsWindowFocused();
    };

    lua["fullScreen"] = [luaptr](sol::variadic_args va) {
        checkArgSize("fullScreen", 0, va.size());
        luaptr->window.flags |= FLAG_FULLSCREEN_MODE;
    };

    lua["frameCount"] = [luaptr](sol::variadic_args va) {
        checkArgSize("frameCount", 0, va.size());
        return luaptr->window.frameCount;
    };

    lua["frameRate"] = [luaptr](sol::variadic_args va) {
        if (va.size() > 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "frameRate", "0 to 1", va.size()); }
        if (va.size() == 0) { return luaptr->window.frameRate; }
        checkArgType("frameRate", va, sol::type::number);
        luaptr->window.frameRate = va[0].as<int>();
        return luaptr->window.frameRate;
    };

    lua["height"] = [](sol::variadic_args va) {
        checkArgSize("height", 0, va.size());
        return GetScreenHeight();
    };

    lua["noCursor"] = [luaptr](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (luaptr->state == Lua::State::Setup)
        {
            luaptr->postSetupFuncs.emplace_back(Environment::noCursor, vec);
            return;
        }
        Environment::noCursor(vec);
    };

    lua["size"] = [luaptr](sol::variadic_args va) {
        if ((va.size() < 2) || (va.size() > 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "size", "2 to 3", va.size());
        }
        checkArgType("size", va, sol::type::number);
        luaptr->window.width    = va[0].as<int>();
        luaptr->window.height   = va[1].as<int>();
        luaptr->canvas.renderer = va.size() == 2 ? Canvas::Renderer::P2D : static_cast<Canvas::Renderer>(va[2].as<int>());

        if (luaptr->canvas.renderer == Canvas::Renderer::P2D) { luaptr->canvas.camera2D.zoom = 1.0f; }
        else if (luaptr->canvas.renderer == Canvas::Renderer::P3D)
        {
            luaptr->canvas.camera3D.fovy = static_cast<float>(luaptr->window.height);
            luaptr->canvas.camera3D.position =
                Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f,
                        -(luaptr->window.height / 2.0f) / static_cast<float>(std::tan(luaptr->canvas.camera3D.fovy * Math::PI_ / 360.0f))};
            luaptr->canvas.camera3D.target     = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, 0.0f};
            luaptr->canvas.camera3D.up         = Vector3{0.0f, 1.0f, 0.0f};
            luaptr->canvas.camera3D.projection = CAMERA_PERSPECTIVE;
            rlSetClipPlanes(-luaptr->canvas.camera3D.position.z / 10.0f, -luaptr->canvas.camera3D.position.z * 10.0f);
        }
    };

    lua["width"] = [](sol::variadic_args va) {
        checkArgSize("width", 0, va.size());
        return GetScreenWidth();
    };

    lua["windowMove"] = [](sol::variadic_args va) {
        checkArgSize("windowMove", 2, va.size());
        checkArgType("windowMove", va, sol::type::number);
        SetWindowPosition(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowResizable"] = [luaptr](sol::variadic_args va) {
        checkArgSize("windowResizable", 1, va.size());
        checkArgType("windowResizable", va, sol::type::boolean);
        if (va[0].as<bool>()) { luaptr->window.flags |= FLAG_WINDOW_RESIZABLE; }
    };

    lua["windowResize"] = [](sol::variadic_args va) {
        checkArgSize("windowResize", 2, va.size());
        checkArgType("windowResize", va, sol::type::number);
        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowTitle"] = [luaptr](sol::variadic_args va) {
        checkArgSize("windowTitle", 1, va.size());
        checkArgType("windowTitle", va, sol::type::string);
        luaptr->window.title = va[0].as<std::string>();
    };
}
}
}