#include "environment.hpp"
#include "luaproc.hpp"
#include "msghandler.hpp"

#include "raylib.h"

namespace LuaProc
{
namespace Environment
{
void cursor(const std::vector<sol::object> &va)
{
    if (va.size() == 0) { return ShowCursor(); }

    if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "cursor", 1, va.size()); }
    if (va[0].get_type() != sol::type::number)
    {
        conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "cursor", solTypeToString(sol::type::number));
    }

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
    if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "noCursor", 0, va.size()); }
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
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "displayHeight", 0, va.size()); }
        return GetMonitorHeight(0);
    };

    lua["displayWidth"] = [](sol::variadic_args va) {
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "displayWidth", 0, va.size()); }
        return GetMonitorWidth(0);
    };

    lua["focused"] = [](sol::variadic_args va) {
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "focused", 0, va.size()); }
        return IsWindowFocused();
    };

    lua["fullScreen"] = [&app](sol::variadic_args va) {
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "fullScreen", 0, va.size()); }
        app.window().flags |= FLAG_FULLSCREEN_MODE;
    };

    lua["frameRate"] = [&app](sol::variadic_args va) {
        if (va.size() == 0) { return app.window().frameRate; }

        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "frameRate", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "frameRate", solTypeToString(sol::type::number));
        }
        app.window().frameRate = va[0].as<int>();
        return app.window().frameRate;
    };

    lua["height"] = [](sol::variadic_args va) {
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "height", 0, va.size()); }
        return GetScreenHeight();
    };

    lua["noCursor"] = [&app](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (app.isState(Application::State::Setup)) { return app.addToPostSetup(Function<PostSetupVariant>{noCursor, vec}); }
        noCursor(vec);
    };

    lua["size"] = [&app](sol::variadic_args va) {
        if (va.size() != 2) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "size", 2, va.size()); }
        if ((va[0].get_type() != sol::type::number) || (va[1].get_type() != sol::type::number))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "size", solTypeToString(sol::type::number));
        }
        app.window().width  = va[0].as<int>();
        app.window().height = va[1].as<int>();
    };

    lua["width"] = [](sol::variadic_args va) {
        if (va.size() != 0) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "width", 0, va.size()); }
        return GetScreenWidth();
    };

    lua["windowMove"] = [](sol::variadic_args va) {
        if (va.size() != 2) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "windowMove", 2, va.size()); }
        if ((va[0].get_type() != sol::type::number) || (va[1].get_type() != sol::type::number))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "windowMove", solTypeToString(sol::type::number));
        }
        SetWindowPosition(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowResizable"] = [&app](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "windowResizable", 1, va.size()); }
        if (va[0].get_type() != sol::type::boolean)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "windowResizable", solTypeToString(sol::type::boolean));
        }
        if (va[0].as<bool>()) { app.window().flags |= FLAG_WINDOW_RESIZABLE; }
    };

    lua["windowResize"] = [](sol::variadic_args va) {
        if (va.size() != 2) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "windowResize", 2, va.size()); }
        if ((va[0].get_type() != sol::type::number) || (va[1].get_type() != sol::type::number))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "windowResize", solTypeToString(sol::type::number));
        }
        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowTitle"] = [&app](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "windowTitle", 1, va.size()); }
        if (va[0].get_type() != sol::type::string)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "windowTitle", solTypeToString(sol::type::string));
        }
        app.window().title = va[0].as<std::string>();
    };
}
}
}