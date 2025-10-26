#include "luaproc.hpp"
#include "environment.hpp"

#include "raylib.h"

#include <fstream>
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

    addScriptCode();

    // ---------- CALLBACKS ----------
    m_currentState = State::Setup;
    setupOutput();
    setupEnvironment();

    // TEMP
    sol::protected_function_result script = lua.safe_script(m_scriptCode);
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

    m_currentState                    = State::PostSetup;
    script                            = lua.safe_script(m_scriptCode);
    sol::protected_function postSetup = lua["__postSetup__"];
    if (!postSetup.valid())
    {
        std::println("[LUAPROC ERROR] '{}' function not found", "__postSetup__");
        std::exit(-1);
    }
    postSetup();

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

LuaProc::State LuaProc::state() const { return m_currentState; }

void LuaProc::addToPostSetup(const std::string &code)
{
    // Insert code before the end of __postSetup__ (string "end" has length of 3 (+ 1))
    m_scriptCode.insert(m_scriptCode.length() - 4, code);
}

void LuaProc::addScriptCode()
{
    const char *file = "cmake/dist/main.lua";
    std::ifstream inputFile(file);
    if (!inputFile.is_open())
    {
        std::println("[LUAPROC ERROR] '{}' file not found", file);
        std::exit(-1);
    }

    // TODO: Check how slow is this
    std::string tempString = "";
    while (std::getline(inputFile, tempString)) { m_scriptCode += tempString + '\n'; }
    m_scriptCode += "\nfunction __postSetup__()\nend\n";
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

    lua["DEFAULT"]         = Window::MouseCursor::DEFAULT;
    lua["ARROW"]           = Window::MouseCursor::ARROW;
    lua["IBEAM"]           = Window::MouseCursor::IBEAM;
    lua["CROSSHAIR"]       = Window::MouseCursor::CROSSHAIR;
    lua["POINTING_HAND"]   = Window::MouseCursor::POINTING_HAND;
    lua["RESIZE_EW"]       = Window::MouseCursor::RESIZE_EW;
    lua["RESIZE_NS"]       = Window::MouseCursor::RESIZE_NS;
    lua["RESIZE_NWSE"]     = Window::MouseCursor::RESIZE_NWSE;
    lua["RESIZE_NESW"]     = Window::MouseCursor::RESIZE_NESW;
    lua["RESIZE_ALL"]      = Window::MouseCursor::RESIZE_ALL;
    lua["NOT_ALLOWED"]     = Window::MouseCursor::NOT_ALLOWED;

    lua["cursor"]          = [&](sol::variadic_args va) { Environment::cursor(va, *this); };
    lua["displayHeight"]   = Environment::displayHeight;
    lua["displayWidth"]    = Environment::displayWidth;
    lua["focused"]         = Environment::focused;
    lua["fullScreen"]      = [&](sol::variadic_args va) { Environment::fullScreen(va, m_window); };
    lua["frameRate"]       = [&](sol::variadic_args va) { return Environment::frameRate(va, m_window); };
    lua["height"]          = Environment::height;
    lua["size"]            = [&](sol::variadic_args va) { Environment::size(va, m_window); };
    lua["width"]           = Environment::width;
    lua["windowMove"]      = Environment::windowMove;
    lua["windowResizable"] = [&](sol::variadic_args va) { Environment::windowResizable(va, m_window); };
    lua["windowResize"]    = Environment::windowResize;
    lua["windowTitle"]     = [&](sol::variadic_args va) { Environment::windowTitle(va, m_window); };
}
}