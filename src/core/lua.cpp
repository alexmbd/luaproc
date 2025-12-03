#include "lua.hpp"
#include "constants.hpp"
#include "msghandler.hpp"

#include "rlgl.h"

#include <cmath>

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
        else if (va[i].get_type() == sol::type::number) { text += std::format("{}", va[i].as<double>()); }
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

// ---------- LUA ----------

Lua::Lua(const char *filename)
{
    // Defaults
    m_window.width      = 640;
    m_window.height     = 360;
    m_window.frameRate  = 60;
    m_window.flags      = 0;
    m_window.title      = "LuaProc";

    m_canvas.background = Color{128, 128, 128, 255};

    // Error Handlers
    m_lua["__MSG_HANDLER__"] = [](const std::string &msg) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, msg); };
    sol::protected_function::set_default_handler(m_lua["__MSG_HANDLER__"]);

    m_lua.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> exception, sol::string_view description) {
        // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
        conditionalExit(MessageType::CPP_ERROR, Message::GENERIC, description.data());
        return sol::stack::push(L, description);
    });

    setScript(filename);
}

const Window &Lua::window() const { return m_window; }

const Canvas &Lua::canvas() const { return m_canvas; }

void Lua::setScript(const char *filename)
{
    m_lua.safe_script_file(filename, [](lua_State *L, sol::protected_function_result pfr) {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, pfr.get<std::string>());
        return pfr;
    });

    // Start setup
    m_state = State::Setup;
    setup();

    // Start postSetup
    m_state = State::PostSetup;
    for (const auto &psFunc : m_postSetupFuncs) { psFunc.function(psFunc.args); }

    // Start draw
    m_state = State::Draw;
}

void Lua::draw()
{
    sol::protected_function drawLua = m_lua["draw"];
    if (!drawLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "draw"); }
    drawLua();
}

void Lua::setup()
{
    setupColor();
    setupEnvironment();
    setupMath();
    setupOutput();
    setupShape();
    setupTransform();

    sol::protected_function setupLua = m_lua["setup"];
    if (!setupLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "setup"); }
    setupLua();

    SetTargetFPS(m_window.frameRate);
    SetConfigFlags(m_window.flags);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
}

// ---------- COLOR ----------
void Lua::setupColor()
{
    m_lua["background"] = [&](sol::variadic_args va) {
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
            if ((arg.as<double>() < 0) || (arg.as<double>() > 255))
            {
                conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "'background' arguments can only be numbers from 0 to 255");
            }
        }

        if (va.size() == 1)
        {
            auto gray           = static_cast<unsigned char>(std::round(va[0].as<double>()));
            m_canvas.background = Color{gray, gray, gray, 255};
        }
        else if (va.size() == 2)
        {
            auto gray           = static_cast<unsigned char>(std::round(va[0].as<double>()));
            auto alpha          = static_cast<unsigned char>(std::round(va[1].as<double>()));
            m_canvas.background = Color{gray, gray, gray, alpha};
        }
        else if (va.size() == 3)
        {
            auto r              = static_cast<unsigned char>(std::round(va[0].as<double>()));
            auto g              = static_cast<unsigned char>(std::round(va[1].as<double>()));
            auto b              = static_cast<unsigned char>(std::round(va[2].as<double>()));
            m_canvas.background = Color{r, g, b, 255};
        }
        else {
            auto r              = static_cast<unsigned char>(std::round(va[0].as<double>()));
            auto g              = static_cast<unsigned char>(std::round(va[1].as<double>()));
            auto b              = static_cast<unsigned char>(std::round(va[2].as<double>()));
            auto a              = static_cast<unsigned char>(std::round(va[3].as<double>()));
            m_canvas.background = Color{r, g, b, a};
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

void Lua::setupEnvironment()
{
    m_lua["DEFAULT"]       = MOUSE_CURSOR_DEFAULT;
    m_lua["ARROW"]         = MOUSE_CURSOR_ARROW;
    m_lua["IBEAM"]         = MOUSE_CURSOR_IBEAM;
    m_lua["CROSSHAIR"]     = MOUSE_CURSOR_CROSSHAIR;
    m_lua["POINTING_HAND"] = MOUSE_CURSOR_POINTING_HAND;
    m_lua["RESIZE_EW"]     = MOUSE_CURSOR_RESIZE_EW;
    m_lua["RESIZE_NS"]     = MOUSE_CURSOR_RESIZE_NS;
    m_lua["RESIZE_NWSE"]   = MOUSE_CURSOR_RESIZE_NWSE;
    m_lua["RESIZE_NESW"]   = MOUSE_CURSOR_RESIZE_NESW;
    m_lua["RESIZE_ALL"]    = MOUSE_CURSOR_RESIZE_ALL;
    m_lua["NOT_ALLOWED"]   = MOUSE_CURSOR_NOT_ALLOWED;

    m_lua["cursor"]        = [&](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (m_state == State::Setup)
        {
            m_postSetupFuncs.emplace_back(Environment::cursor, vec);
            return;
        }
        Environment::cursor(vec);
    };

    m_lua["displayHeight"] = [](sol::variadic_args va) {
        checkArgSize("displayHeight", 0, va.size());
        return GetMonitorHeight(0);
    };

    m_lua["displayWidth"] = [](sol::variadic_args va) {
        checkArgSize("displayWidth", 0, va.size());
        return GetMonitorWidth(0);
    };

    m_lua["focused"] = [](sol::variadic_args va) {
        checkArgSize("focused", 0, va.size());
        return IsWindowFocused();
    };

    m_lua["fullScreen"] = [&](sol::variadic_args va) {
        checkArgSize("fullScreen", 0, va.size());
        m_window.flags |= FLAG_FULLSCREEN_MODE;
    };

    m_lua["frameRate"] = [&](sol::variadic_args va) {
        if (va.size() == 0) { return m_window.frameRate; }

        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "frameRate", 1, va.size()); }
        checkArgType("frameRate", va, sol::type::number);
        m_window.frameRate = va[0].as<int>();
        return m_window.frameRate;
    };

    m_lua["height"] = [](sol::variadic_args va) {
        checkArgSize("height", 0, va.size());
        return GetScreenHeight();
    };

    m_lua["noCursor"] = [&](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (m_state == State::Setup)
        {
            m_postSetupFuncs.emplace_back(Environment::noCursor, vec);
            return;
        }
        Environment::noCursor(vec);
    };

    m_lua["size"] = [&](sol::variadic_args va) {
        checkArgSize("size", 2, va.size());
        checkArgType("size", va, sol::type::number);
        m_window.width  = va[0].as<int>();
        m_window.height = va[1].as<int>();
    };

    m_lua["width"] = [](sol::variadic_args va) {
        checkArgSize("width", 0, va.size());
        return GetScreenWidth();
    };

    m_lua["windowMove"] = [](sol::variadic_args va) {
        checkArgSize("windowMove", 2, va.size());
        checkArgType("windowMove", va, sol::type::number);
        SetWindowPosition(va[0].as<int>(), va[1].as<int>());
    };

    m_lua["windowResizable"] = [&](sol::variadic_args va) {
        checkArgSize("windowResizable", 1, va.size());
        checkArgType("windowResizable", va, sol::type::boolean);
        if (va[0].as<bool>()) { m_window.flags |= FLAG_WINDOW_RESIZABLE; }
    };

    m_lua["windowResize"] = [](sol::variadic_args va) {
        checkArgSize("windowResize", 2, va.size());
        checkArgType("windowResize", va, sol::type::number);
        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    m_lua["windowTitle"] = [&](sol::variadic_args va) {
        checkArgSize("windowTitle", 1, va.size());
        checkArgType("windowTitle", va, sol::type::string);
        m_window.title = va[0].as<std::string>();
    };
}

// ---------- MATH ----------
void Lua::setupMath()
{
    m_lua["HALF_PI"]    = Math::HALF_PI;
    m_lua["PI"]         = Math::PI_;
    m_lua["QUARTER_PI"] = Math::QUARTER_PI;
    m_lua["TWO_PI"]     = Math::TWO_PI;
    m_lua["TAU"]        = Math::TWO_PI;

    m_lua["degrees"]    = [](sol::variadic_args va) {
        checkArgSize("degrees", 1, va.size());
        checkArgType("degrees", va, sol::type::number);
        return va[0].as<double>() * (180 / Math::PI_);
    };

    m_lua["radians"] = [](sol::variadic_args va) {
        checkArgSize("radians", 1, va.size());
        checkArgType("radians", va, sol::type::number);
        return va[0].as<double>() * (Math::PI_ / 180);
    };
}

// ---------- OUTPUT ----------
void Lua::setupOutput()
{
    m_lua["print"]   = [](sol::variadic_args va) { Output::print(va, false); };
    m_lua["println"] = [](sol::variadic_args va) { Output::print(va, true); };
}

// ---------- SHAPE ----------
void Lua::setupShape()
{
    m_lua["line"] = [](sol::variadic_args va) {
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
void Lua::setupTransform()
{
    // NOTE: All angles in lua are in radians

    m_lua["rotateX"] = [](sol::variadic_args va) {
        checkArgSize("rotateX", 1, va.size());
        checkArgType("rotateX", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    m_lua["rotateY"] = [](sol::variadic_args va) {
        checkArgSize("rotateY", 1, va.size());
        checkArgType("rotateY", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    m_lua["rotateZ"] = [](sol::variadic_args va) {
        checkArgSize("rotateZ", 1, va.size());
        checkArgType("rotateZ", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    m_lua["rotate"] = [](sol::variadic_args va) {
        checkArgSize("rotate", 1, va.size());
        checkArgType("rotate", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    m_lua["translate"] = [](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }
        checkArgType("translate", va, sol::type::number);

        float z = va.size() == 3 ? va[2].as<double>() : 0.0f;
        rlTranslatef(va[0].as<double>(), va[1].as<double>(), z);
    };
}
}