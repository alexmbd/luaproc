#include "app.hpp"
#include "msghandler.hpp"

namespace LuaProc
{
void customLog(int msgType, const char *text, va_list args) {}

Application::Application()
{
    SetTraceLogCallback(customLog);

    // TEMP
    const char *filename = "examples/sin_circle.lua";
    m_lua                = std::make_shared<Lua>();

    sol::state &lua      = m_lua->lua;
    setup(m_lua);

    lua.safe_script_file(filename, [](lua_State *L, sol::protected_function_result pfr) {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, pfr.get<std::string>());
        return pfr;
    });
    sol::protected_function setupLua = lua["setup"];
    if (!setupLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "setup"); }
    setupLua();

    if ((m_lua->window.width <= 0) || (m_lua->window.height <= 0))
    {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "window size not valid must be greater than 0");
    }
    SetTargetFPS(m_lua->window.frameRate);
    SetConfigFlags(m_lua->window.flags);
    InitWindow(m_lua->window.width, m_lua->window.height, m_lua->window.title.c_str());

    // Start postSetup
    m_lua->state = Lua::State::PostSetup;
    for (const auto &psFunc : m_lua->postSetupFuncs) { psFunc.function(psFunc.args); }

    // Start draw
    m_lua->state = Lua::State::Draw;
}

Application::~Application() { CloseWindow(); }

void Application::run()
{
    while (!WindowShouldClose())
    {
        m_lua->update();

        ClearBackground(m_lua->canvas.background);
        BeginDrawing();
        BeginMode3D(m_lua->canvas.camera);
        m_lua->draw();
        EndMode3D();
        EndDrawing();
    }
}
}