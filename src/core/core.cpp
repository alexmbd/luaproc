#include "core.hpp"
#include "constants.hpp"
#include "luaproc.hpp"
#include "msghandler.hpp"

#include "raylib.h"
#include "rlgl.h"

#include <cmath>

namespace LuaProc
{
// ---------- COLOR ----------
void setupColor(Application &app)
{
    sol::state &lua   = app.lua();

    lua["background"] = [&app](sol::variadic_args va) {
        // (gray)
        // (gray, a)
        // (r, g, b)
        // (r, g, b, a)
        // No support for HSV yet

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "background", "1 to 4", va.size());
        }

        for (const sol::stack_proxy &arg : va)
        {
            if (arg.get_type() != sol::type::number)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "background", solTypeToString(sol::type::number));
            }
            if (arg.as<float>() < 0)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "'background' arguments can only be numbers from 0 to 255");
            }
        }

        Window &window = app.window();
        if (va.size() == 1)
        {
            auto gray         = static_cast<unsigned char>(std::round(va[0].as<float>()));
            window.background = Color{gray, gray, gray, 255};
        }
        else if (va.size() == 2)
        {
            auto gray         = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto alpha        = static_cast<unsigned char>(std::round(va[1].as<float>()));
            window.background = Color{gray, gray, gray, alpha};
        }
        else if (va.size() == 3)
        {
            auto r            = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto g            = static_cast<unsigned char>(std::round(va[1].as<float>()));
            auto b            = static_cast<unsigned char>(std::round(va[2].as<float>()));
            window.background = Color{r, g, b, 255};
        }
        else {
            auto r            = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto g            = static_cast<unsigned char>(std::round(va[1].as<float>()));
            auto b            = static_cast<unsigned char>(std::round(va[2].as<float>()));
            auto a            = static_cast<unsigned char>(std::round(va[3].as<float>()));
            window.background = Color{r, g, b, a};
        }
    };
}

// ---------- ENVIRONMENT ----------
// ENVIRONMENT (Not implemented)
// delay, displayDensity
// frameCount
// noSmooth
// pixelDensity, pixelHeight, pixelWidth
// settings, smooth
// windowMoved, windowRatio, windowResized

// ENVIRONMENT (API changes)
// frameRate variable and function merged

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
    else {
        SetMouseCursor(cursorType);
    }
}

void noCursor(const std::vector<sol::object> &va)
{
    checkArgSize("noCursor", 0, va.size());
    HideCursor();
};
}

void setupEnvironment(Application &app)
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
        if (app.isState(Application::State::Setup)) { return app.addToPostSetup(Function<PostSetupVariant>{Environment::cursor, vec}); }
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

    lua["fullScreen"] = [&app](sol::variadic_args va) {
        checkArgSize("fullScreen", 0, va.size());
        app.window().flags |= FLAG_FULLSCREEN_MODE;
    };

    lua["frameRate"] = [&app](sol::variadic_args va) {
        if (va.size() == 0) { return app.window().frameRate; }

        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "frameRate", 1, va.size()); }
        checkArgType("frameRate", va, sol::type::number);
        app.window().frameRate = va[0].as<int>();
        return app.window().frameRate;
    };

    lua["height"] = [](sol::variadic_args va) {
        checkArgSize("height", 0, va.size());
        return GetScreenHeight();
    };

    lua["noCursor"] = [&app](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (app.isState(Application::State::Setup)) { return app.addToPostSetup(Function<PostSetupVariant>{Environment::noCursor, vec}); }
        Environment::noCursor(vec);
    };

    lua["size"] = [&app](sol::variadic_args va) {
        checkArgSize("size", 2, va.size());
        checkArgType("size", va, sol::type::number);
        app.window().width  = va[0].as<int>();
        app.window().height = va[1].as<int>();
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

    lua["windowResizable"] = [&app](sol::variadic_args va) {
        checkArgSize("windowResizable", 1, va.size());
        checkArgType("windowResizable", va, sol::type::boolean);
        if (va[0].as<bool>()) { app.window().flags |= FLAG_WINDOW_RESIZABLE; }
    };

    lua["windowResize"] = [](sol::variadic_args va) {
        checkArgSize("windowResize", 2, va.size());
        checkArgType("windowResize", va, sol::type::number);
        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowTitle"] = [&app](sol::variadic_args va) {
        checkArgSize("windowTitle", 1, va.size());
        checkArgType("windowTitle", va, sol::type::string);
        app.window().title = va[0].as<std::string>();
    };
}

// ---------- MATH ----------
void setupMath(Application &app)
{
    sol::state &lua   = app.lua();

    lua["HALF_PI"]    = Math::HALF_PI;
    lua["PI"]         = Math::PI_;
    lua["QUARTER_PI"] = Math::QUARTER_PI;
    lua["TWO_PI"]     = Math::TWO_PI;
    lua["TAU"]        = Math::TWO_PI;

    lua["degrees"]    = [](sol::variadic_args va) {
        checkArgSize("degrees", 1, va.size());
        checkArgType("degrees", va, sol::type::number);
        return va[0].as<double>() * (180 / Math::PI_);
    };

    lua["radians"] = [](sol::variadic_args va) {
        checkArgSize("radians", 1, va.size());
        checkArgType("radians", va, sol::type::number);
        return va[0].as<double>() * (Math::PI_ / 180);
    };
}

// ---------- OUTPUT ----------

namespace Output
{
void print(sol::variadic_args va, bool newline)
{
    // Get Type (https://github.com/ThePhD/sol2/issues/850#issuecomment-515720422)
    std::string text = "";
    for (int i = 0; i < va.size(); i++)
    {
        if (va[i].get_type() == sol::type::nil) { text += "nil"; }
        else if (va[i].get_type() == sol::type::boolean) { text += va[i].as<bool>() ? "true" : "false"; }
        else {
            text += va[i].as<std::string>();
        }

        if (i != va.size() - 1) { text += " "; }
    }
    if (newline) { std::println("{}", text); }
    else {
        std::print("{}", text);
    }
}
}

void setupOutput(Application &app)
{
    sol::state &lua = app.lua();

    lua["print"]    = [](sol::variadic_args va) { Output::print(va, false); };
    lua["println"]  = [](sol::variadic_args va) { Output::print(va, true); };
}

// ---------- SHAPE ----------
void setupShape(Application &app)
{
    sol::state &lua = app.lua();

    lua["line"]     = [](sol::variadic_args va) {
        if ((va.size() != 4) && (va.size() != 6))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "line", "4 or 6", va.size());
        }
        checkArgType("line", va, sol::type::number);
        if (va.size() == 4)
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), 0.0f}, Vector3{va[2].as<float>(), va[3].as<float>(), 0.0f}, WHITE);
        }
        else {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), va[2].as<float>()},
                           Vector3{va[3].as<float>(), va[4].as<float>(), va[5].as<float>()}, WHITE);
        }
    };
}

// ---------- TRANSFORM ----------
void setupTransform(Application &app)
{
    sol::state &lua = app.lua();

    // NOTE: All angles in lua are in radians

    lua["rotateX"] = [](sol::variadic_args va) {
        checkArgSize("rotateX", 1, va.size());
        checkArgType("rotateX", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    lua["rotateY"] = [](sol::variadic_args va) {
        checkArgSize("rotateY", 1, va.size());
        checkArgType("rotateY", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    lua["rotateZ"] = [](sol::variadic_args va) {
        checkArgSize("rotateZ", 1, va.size());
        checkArgType("rotateZ", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    lua["rotate"] = [](sol::variadic_args va) {
        checkArgSize("rotate", 1, va.size());
        checkArgType("rotate", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    lua["translate"] = [](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }
        checkArgType("translate", va, sol::type::number);

        float z = va.size() == 3 ? va[2].as<float>() : 0.0f;
        rlTranslatef(va[0].as<float>(), va[1].as<float>(), z);
    };
}

// ---------- CORE ----------
void setupCore(Application &app)
{
    setupColor(app);
    setupEnvironment(app);
    setupMath(app);
    setupOutput(app);
    setupShape(app);
    setupTransform(app);
}
}