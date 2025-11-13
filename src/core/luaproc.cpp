#include "luaproc.hpp"
#include "environment.hpp"
#include "msghandler.hpp"

#include "raylib.h"

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

// ---------- APPLICATION CLASS ----------

Application::Application()
{
    // Set defaults in case size(), frameRate() and windowTitle() are not called
    m_window.width     = 640;
    m_window.height    = 360;
    m_window.frameRate = 60;
    m_window.flags     = 0;
    m_window.title     = "LuaProc";

    // ---------- CALLBACKS ----------
    m_lua["__MSG_HANDLER__"] = [](const std::string &msg) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, msg); };
    sol::protected_function::set_default_handler(m_lua["__MSG_HANDLER__"]);

    m_lua.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> exception, sol::string_view description) {
        // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
        conditionalExit(MessageType::CPP_ERROR, Message::GENERIC, description.data());
        return sol::stack::push(L, description);
    });

    m_currentState = State::Setup;
    setupOutput();
    Environment::setup(*this);

    // TEMP
    const char *file                      = "cmake/dist/main.lua";
    sol::protected_function_result script = m_lua.safe_script_file(file, [](lua_State *L, sol::protected_function_result pfr) {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, pfr.get<std::string>());
        return pfr;
    });
    sol::protected_function setup         = m_lua["setup"];
    if (!setup.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "setup"); }
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

Application::~Application() { CloseWindow(); }

void Application::run()
{
    while (!WindowShouldClose())
    {
        if (IsKeyReleased(KEY_A)) { std::println("{} x {}", m_lua["displayWidth"]().get<int>(), m_lua["displayHeight"]().get<int>()); }

        ClearBackground(BLACK);
        BeginDrawing();
        EndDrawing();
    }
}

sol::state &Application::lua() { return m_lua; }

Window &Application::window() { return m_window; }

bool Application::isState(State state) const { return m_currentState == state; }

void Application::addToPostSetup(Function<PostSetupVariant> func) { m_postSetupFuncs.push_back(func); }

void Application::setupOutput()
{
    // OUTPUT
    m_lua["print"]   = [](sol::variadic_args va) { print(va, false); };
    m_lua["println"] = [](sol::variadic_args va) { print(va, true); };
}
}