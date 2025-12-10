#include "app.hpp"
#include "msghandler.hpp"

#include "rlgl.h"

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
        if (m_lua->canvas.renderer == Canvas::Renderer::P2D)
        {
            BeginMode2D(m_lua->canvas.camera2D);
            m_lua->draw();
            if (m_lua->canvas.needToPopMatrix)
            {
                rlPopMatrix();
                m_lua->canvas.needToPopMatrix = false;
            }
            EndMode2D();
        }
        else
        {
            BeginMode3D(m_lua->canvas.camera3D);
            m_lua->draw();
            if (m_lua->canvas.needToPopMatrix)
            {
                rlPopMatrix();
                m_lua->canvas.needToPopMatrix = false;
            }
            EndMode3D();
        }
        EndDrawing();
    }
}
}