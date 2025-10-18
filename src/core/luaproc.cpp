#include "luaproc.hpp"
#include "environment.hpp"

#include "raylib.h"

#include <print>

namespace LuaProc
{

// ---------- OUTPUT ----------

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
    sol::state &lua        = *m_lua;

    lua["displayHeight"]   = Environment::displayHeight;
    lua["displayWidth"]    = Environment::displayWidth;
    lua["focused"]         = Environment::focused;
    lua["fullScreen"]      = [&](sol::variadic_args va) { Environment::fullScreen(va, m_window); };
    lua["frameRate"]       = [&](sol::variadic_args va) { Environment::frameRate(va, m_window); };
    lua["height"]          = Environment::height;
    lua["size"]            = [&](sol::variadic_args va) { Environment::size(va, m_window); };
    lua["width"]           = Environment::width;
    lua["windowMove"]      = Environment::windowMove;
    lua["windowResizable"] = [&](sol::variadic_args va) { Environment::windowResizable(va, m_window); };
    lua["windowResize"]    = Environment::windowResize;
    lua["windowTitle"]     = [&](sol::variadic_args va) { Environment::windowTitle(va, m_window); };
}
}