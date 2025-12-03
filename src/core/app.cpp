#include "app.hpp"
#include "msghandler.hpp"

namespace LuaProc
{
void customLog(int msgType, const char *text, va_list args) {}

Application::Application()
{
    SetTraceLogCallback(customLog);

    // TEMP
    const char *file = "examples/main.lua";
    m_lua            = Lua(file);
}

Application::~Application() { CloseWindow(); }

void Application::run()
{
    while (!WindowShouldClose())
    {
        ClearBackground(m_lua.canvas().background);
        BeginDrawing();
        m_lua.draw();
        EndDrawing();
    }
}
}