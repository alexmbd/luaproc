#include "luaproc.hpp"

#include "raylib.h"
#include "sol.hpp"

#include <print>

namespace LuaProc
{
// ---------- FREE FUNCTIONS ----------

std::string solTypeToString(sol::type type)
{
    switch (type)
    {
    case sol::type::boolean:
        return "boolean";

    case sol::type::function:
        return "function";

    case sol::type::lightuserdata:
        return "lightuserdata";

    case sol::type::nil:
        return "nil";

    case sol::type::number:
        return "number";

    case sol::type::string:
        return "string";

    case sol::type::table:
        return "table";

    case sol::type::thread:
        return "thread";

    case sol::type::userdata:
        return "userdata";

    default:
        return "unknown type";
    }
}

void checkVASize(const sol::variadic_args &va, const std::string &name, int expectedSize)
{
    if (va.size() == expectedSize) { return; }
    std::println("[LUAPROC ERROR] '{}' expects {} arguments but got {}", name, expectedSize, va.size());
    std::exit(-1);
}

void checkType(const sol::stack_proxy &var, const std::string &name, sol::type type)
{
    if (var.get_type() == type) { return; }
    std::println("[LUAPROC ERROR] '{}' expects arguments of type '{}'", name, solTypeToString(type));
    std::exit(-1);
}

// ---------- OUTPUT ----------

void print(sol::variadic_args va, bool newline)
{
    // Get Type (https://github.com/ThePhD/sol2/issues/850#issuecomment-515720422)
    std::string text = "";
    for (int i = 0; i < va.size(); i++)
    {
        if (va[i].get_type() == sol::type::nil) { text += "nil"; }
        else if (va[i].get_type() == sol::type::boolean) { text += va[i].as<bool>() ? "true" : "false"; }
        else { text += va[i].as<std::string>(); }

        if (i != va.size() - 1) { text += " "; }
    }
    if (newline) { std::println("{}", text); }
    else { std::print("{}", text); }
}

void customLog(int msgType, const char *text, va_list args) {}

// ---------- LUAPROC CLASS ----------

LuaProc::LuaProc()
{
    // Set defaults in case size(), frameRate() and windowTitle() are not called
    m_window.width     = 640;
    m_window.height    = 360;
    m_window.frameRate = 60;
    m_window.flags     = 0;
    m_window.title     = "LuaProc";

    // Use unique_ptr to forward declare the sol::state and not pollute the header file
    m_lua           = std::make_unique<sol::state>();
    sol::state &lua = *m_lua;

    // ---------- CALLBACKS ----------
    setupOutput();
    setupEnvironment();

    // TEMP
    sol::protected_function_result script = lua.safe_script_file("cmake/dist/main.lua");
    sol::protected_function setup         = lua["Setup"];
    if (!setup.valid())
    {
        std::println("[LUAPROC ERROR] '{}' function not found", "Setup");
        std::exit(-1);
    }
    setup();

    SetTraceLogCallback(customLog);
    SetTargetFPS(m_window.frameRate);
    SetConfigFlags(m_window.flags);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
}

LuaProc::~LuaProc() { CloseWindow(); }

void LuaProc::run()
{
    while (!WindowShouldClose())
    {
        if (IsKeyReleased(KEY_A))
        {
            auto &lua = *m_lua;
            std::println("{} x {}", lua["displayWidth"]().get<int>(), lua["displayHeight"]().get<int>());
        }

        ClearBackground(BLACK);
        BeginDrawing();
        EndDrawing();
    }
}

void LuaProc::setupOutput()
{
    sol::state &lua = *m_lua;

    // OUTPUT
    lua["print"]   = [](sol::variadic_args va) { print(va, false); };
    lua["println"] = [](sol::variadic_args va) { print(va, true); };
}

void LuaProc::setupEnvironment()
{
    sol::state &lua = *m_lua;

    // ENVIRONMENT
    // Not implemented:
    // cursor
    // displayDensity, delay
    // frameCount
    // noCursor, noSmooth
    // pixelDensity, pixelHeight, pixelWidth
    // settings, smooth
    // windowMoved, windowRatio, windowResized
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
    lua["fullScreen"] = [&](sol::variadic_args va) {
        checkVASize(va, "fullScreen", 0);

        m_window.flags |= FLAG_FULLSCREEN_MODE;
    };
    lua["frameRate"] = [&](sol::variadic_args va) {
        checkVASize(va, "frameRate", 1);
        checkType(va[0], "frameRate", sol::type::number);

        m_window.frameRate = va[0].as<int>();
    };
    lua["height"] = [](sol::variadic_args va) {
        checkVASize(va, "height", 0);

        return GetScreenHeight();
    };
    lua["size"] = [&](sol::variadic_args va) {
        checkVASize(va, "size", 2);
        checkType(va[0], "size", sol::type::number);
        checkType(va[1], "size", sol::type::number);

        m_window.width  = va[0].as<int>();
        m_window.height = va[1].as<int>();
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
    lua["windowResizable"] = [&](sol::variadic_args va) {
        checkVASize(va, "windowResizable", 1);
        checkType(va[0], "windowResizable", sol::type::boolean);

        if (va[0].as<bool>()) { m_window.flags |= FLAG_WINDOW_RESIZABLE; }
    };
    lua["windowResize"] = [](sol::variadic_args va) {
        checkVASize(va, "windowResize", 2);
        checkType(va[0], "windowResize", sol::type::number);
        checkType(va[1], "windowResize", sol::type::number);

        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };
    lua["windowTitle"] = [&](sol::variadic_args va) {
        checkVASize(va, "windowTitle", 1);
        checkType(va[0], "windowTitle", sol::type::string);

        m_window.title = va[0].as<std::string>();
    };
}
}