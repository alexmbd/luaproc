#include "app.hpp"
#include "msghandler.hpp"

namespace LuaProc
{
void customLog(int msgType, const char *text, va_list args) {}

Application::Application(const std::string &filename)
{
    SetTraceLogCallback(customLog);

    m_lua = std::make_shared<Lua>();
    setupScript(m_lua, filename);
}

Application::~Application() { CloseWindow(); }

void Application::run()
{
    while (!WindowShouldClose())
    {
        m_lua->update();

        BeginDrawing();
        ClearBackground(m_lua->canvas.background);
        m_lua->draw();
        EndDrawing();
    }
}
}