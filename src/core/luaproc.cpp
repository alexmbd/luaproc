#include "luaproc.hpp"
#include "msghandler.hpp"

#include "color.hpp"
#include "environment.hpp"
#include "math.hpp"
#include "output.hpp"
#include "shape.hpp"
#include "transform.hpp"

namespace LuaProc
{
void customLog(int msgType, const char *text, va_list args) {}

Application::Application()
{
    // Defaults
    m_window.width      = 640;
    m_window.height     = 360;
    m_window.frameRate  = 60;
    m_window.flags      = 0;
    m_window.title      = "LuaProc";
    m_window.background = Color{128, 128, 128, 255};

    // ---------- CALLBACKS ----------
    m_lua["__MSG_HANDLER__"] = [](const std::string &msg) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, msg); };
    sol::protected_function::set_default_handler(m_lua["__MSG_HANDLER__"]);

    m_lua.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> exception, sol::string_view description) {
        // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
        conditionalExit(MessageType::CPP_ERROR, Message::GENERIC, description.data());
        return sol::stack::push(L, description);
    });

    m_currentState = State::Setup;
    setupColor(*this);
    setupEnvironment(*this);
    setupMath(*this);
    setupOutput(*this);
    setupShape(*this);
    setupTransform(*this);

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
    sol::protected_function draw = m_lua["draw"];
    if (!draw.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "draw"); }

    // TEMP
    Camera3D camera   = {0};
    camera.position   = (Vector3){0.0f, 100.0f, 10.0f}; // Camera position
    camera.target     = (Vector3){0.0f, 0.0f, 0.0f};    // Camera looking at point
    camera.up         = (Vector3){0.0f, 10.0f, 0.0f};   // Camera up vector (rotation towards target)
    camera.fovy       = 60.0f;                          // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        if (IsKeyReleased(KEY_A)) { std::println("{} x {}", m_lua["displayWidth"]().get<int>(), m_lua["displayHeight"]().get<int>()); }

        if (IsKeyDown(KEY_LEFT_CONTROL)) { UpdateCamera(&camera, CAMERA_ORTHOGRAPHIC); }
        ClearBackground(m_window.background);
        BeginDrawing();
        BeginMode3D(camera);
        DrawGrid(100, 10.0f);
        draw();
        EndMode3D();
        EndDrawing();
    }
}

sol::state &Application::lua() { return m_lua; }

Window &Application::window() { return m_window; }

bool Application::isState(State state) const { return m_currentState == state; }

void Application::addToPostSetup(Function<PostSetupVariant> func) { m_postSetupFuncs.push_back(func); }
}