#include "environment.hpp"
#include "luaproc.hpp"
#include "utils.hpp"

#include "raylib.h"

namespace LuaProc
{
namespace Environment
{
void cursor(const std::vector<sol::object> &va)
{
    if (va.size() == 0) { return ShowCursor(); }

    checkVASize(va, "cursor", 1);
    checkType(va[0], "cursor", sol::type::number);

    int cursorType = va[0].as<int>();

    ShowCursor();
    if (cursorType > MOUSE_CURSOR_NOT_ALLOWED || cursorType < 0)
    {
        // Just set to the default cursor if an invalid number was passed (no error thrown)
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        std::println("[LUAPROC WARNING] '{}' was passed as an invalid argument to '{}'. Using default cursor", cursorType, "cursor");
    }
    else {
        SetMouseCursor(cursorType);
    }
}

void noCursor(const std::vector<sol::object> &va)
{
    checkVASize(va, "noCursor", 0);
    HideCursor();
};

void setup(Application &app)
{
    sol::state &lua      = app.lua();

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

    lua["cursor"]        = [&app](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (app.isState(Application::State::Setup)) { return app.addToPostSetup(Function<PostSetupVariant>{cursor, vec}); }
        cursor(vec);
    };

    lua["displayHeight"] = [](sol::variadic_args va) {
        checkVASize(va, "displayHeight", 0);
        return GetMonitorHeight(0);
    };

    lua["displayWidth"] = [](sol::variadic_args va) {
        checkVASize(va, "displayWidth", 0);
        return GetMonitorWidth(0);
    };

    lua["focused"] = [](sol::variadic_args va) {
        checkVASize(va, "focused", 0);
        return IsWindowFocused();
    };

    lua["fullScreen"] = [&app](sol::variadic_args va) {
        checkVASize(va, "fullScreen", 0);
        app.window().flags |= FLAG_FULLSCREEN_MODE;
    };

    lua["frameRate"] = [&app](sol::variadic_args va) {
        if (va.size() == 0) { return app.window().frameRate; }

        checkVASize(va, "frameRate", 1);
        checkType(va[0], "frameRate", sol::type::number);

        app.window().frameRate = va[0].as<int>();
        return app.window().frameRate;
    };

    lua["height"] = [](sol::variadic_args va) {
        checkVASize(va, "height", 0);
        return GetScreenHeight();
    };

    lua["noCursor"] = [&app](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (app.isState(Application::State::Setup)) { return app.addToPostSetup(Function<PostSetupVariant>{noCursor, vec}); }
        noCursor(vec);
    };

    lua["size"] = [&app](sol::variadic_args va) {
        checkVASize(va, "size", 2);
        checkType(va[0], "size", sol::type::number);
        checkType(va[1], "size", sol::type::number);

        app.window().width  = va[0].as<int>();
        app.window().height = va[1].as<int>();
    };

    lua["width"] = [](sol::variadic_args va) {
        checkVASize(va, "width", 0);
        return GetScreenWidth();
    };

    lua["windowMove"] = [](sol::variadic_args va) {
        checkVASize(va, "windowMove", 2);
        checkType(va[0], "windowMove", sol::type::number);
        checkType(va[1], "windowMove", sol::type::number);

        SetWindowPosition(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowResizable"] = [&app](sol::variadic_args va) {
        checkVASize(va, "windowResizable", 1);
        checkType(va[0], "windowResizable", sol::type::boolean);

        if (va[0].as<bool>()) { app.window().flags |= FLAG_WINDOW_RESIZABLE; }
    };

    lua["windowResize"] = [](sol::variadic_args va) {
        checkVASize(va, "windowResize", 2);
        checkType(va[0], "windowResize", sol::type::number);
        checkType(va[1], "windowResize", sol::type::number);

        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowTitle"] = [&app](sol::variadic_args va) {
        checkVASize(va, "windowTitle", 1);
        checkType(va[0], "windowTitle", sol::type::string);

        app.window().title = va[0].as<std::string>();
    };
}
}
}