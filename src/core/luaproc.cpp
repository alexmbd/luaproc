#include "luaproc.hpp"
#include "environment.hpp"
#include "utils.hpp"

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
    m_currentState = State::Setup;
    setupOutput();
    setupEnvironment();

    // TEMP
    const char *file                      = "cmake/dist/main.lua";
    sol::protected_function_result script = lua.safe_script_file(file);
    sol::protected_function setup         = lua["setup"];
    if (!setup.valid())
    {
        std::println("[LUAPROC ERROR] '{}' function not found", "setup");
        std::exit(-1);
    }
    setup();

    SetTraceLogCallback(customLog);
    SetTargetFPS(m_window.frameRate);
    SetConfigFlags(m_window.flags);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());

    m_currentState = State::PostSetup;
    for (const auto &function : m_postSetupFuncs)
    {
        if (const FuncVoid *func = std::get_if<FuncVoid>(&function.function)) { (*func)(function.args); }
    }

    m_currentState = State::Draw;
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
    sol::state &lua      = *m_lua;

    lua["DEFAULT"]       = Window::MouseCursor::DEFAULT;
    lua["ARROW"]         = Window::MouseCursor::ARROW;
    lua["IBEAM"]         = Window::MouseCursor::IBEAM;
    lua["CROSSHAIR"]     = Window::MouseCursor::CROSSHAIR;
    lua["POINTING_HAND"] = Window::MouseCursor::POINTING_HAND;
    lua["RESIZE_EW"]     = Window::MouseCursor::RESIZE_EW;
    lua["RESIZE_NS"]     = Window::MouseCursor::RESIZE_NS;
    lua["RESIZE_NWSE"]   = Window::MouseCursor::RESIZE_NWSE;
    lua["RESIZE_NESW"]   = Window::MouseCursor::RESIZE_NESW;
    lua["RESIZE_ALL"]    = Window::MouseCursor::RESIZE_ALL;
    lua["NOT_ALLOWED"]   = Window::MouseCursor::NOT_ALLOWED;

    lua["cursor"]        = [&](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (m_currentState == State::Setup)
        {
            Function<PostSetupVariant> func{Environment::cursor, vec};
            m_postSetupFuncs.push_back(func);
            return;
        }
        Environment::cursor(vec);
    };
    lua["displayHeight"] = [](sol::variadic_args va) { return Environment::displayHeight(std::vector<sol::object>(va.begin(), va.end())); };
    lua["displayWidth"]  = [](sol::variadic_args va) { return Environment::displayWidth(std::vector<sol::object>(va.begin(), va.end())); };
    lua["focused"]       = [](sol::variadic_args va) { return Environment::focused(std::vector<sol::object>(va.begin(), va.end())); };
    lua["fullScreen"] = [&](sol::variadic_args va) { Environment::fullScreen(std::vector<sol::object>(va.begin(), va.end()), m_window); };
    lua["frameRate"]  = [&](sol::variadic_args va) {
        return Environment::frameRate(std::vector<sol::object>(va.begin(), va.end()), m_window);
    };
    lua["height"]   = [](sol::variadic_args va) { return Environment::height(std::vector<sol::object>(va.begin(), va.end())); };
    lua["noCursor"] = [&](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (m_currentState == State::Setup)
        {
            Function<PostSetupVariant> func{Environment::noCursor, vec};
            m_postSetupFuncs.push_back(func);
            return;
        }
        Environment::noCursor(vec);
    };
    lua["size"]            = [&](sol::variadic_args va) { Environment::size(std::vector<sol::object>(va.begin(), va.end()), m_window); };
    lua["width"]           = [](sol::variadic_args va) { return Environment::width(std::vector<sol::object>(va.begin(), va.end())); };
    lua["windowMove"]      = [](sol::variadic_args va) { Environment::windowMove(std::vector<sol::object>(va.begin(), va.end())); };
    lua["windowResizable"] = [&](sol::variadic_args va) {
        Environment::windowResizable(std::vector<sol::object>(va.begin(), va.end()), m_window);
    };
    lua["windowResize"] = [](sol::variadic_args va) { Environment::windowResize(std::vector<sol::object>(va.begin(), va.end())); };
    lua["windowTitle"] = [&](sol::variadic_args va) { Environment::windowTitle(std::vector<sol::object>(va.begin(), va.end()), m_window); };
}
}