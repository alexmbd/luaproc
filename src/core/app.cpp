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

        ClearBackground(m_lua->canvas.background);
        BeginDrawing();
        BeginMode3D(m_lua->canvas.camera);
        m_lua->draw();
        EndMode3D();
        EndDrawing();
    }
}
}