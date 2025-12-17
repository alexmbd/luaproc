#include "lua.hpp"
#include "constants.hpp"
#include "msghandler.hpp"

#include "modules/color.hpp"
#include "modules/environment.hpp"
#include "modules/lightscamera.hpp"
#include "modules/math.hpp"
#include "modules/output.hpp"
#include "modules/shape.hpp"
#include "modules/transform.hpp"

#include "raymath.h"
#include "rlgl.h"

namespace LuaProc
{
void beginDrawing(const Lua &lua)
{
    if (lua.canvas.renderer == Canvas::Renderer::P2D) { return; }

    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    double fov    = 60.0;
    double aspect = static_cast<double>(lua.window.width / lua.window.height);
    double near   = 1.0;
    double far    = 10000.0;

    if (lua.canvas.projection == Canvas::Projection::ORTHOGRAPHIC)
    {
        double top   = lua.window.height / 2.0;
        double right = top * aspect;
        rlOrtho(-right, right, -top, top, near, far);
    }
    else
    {
        rlMultMatrixf(MatrixToFloat(MatrixPerspective(fov * Math::PI_ / 180.0, aspect, near, far)));
    }

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    rlScalef(1.0f, -1.0f, 1.0f);

    rlTranslatef(-lua.window.width * 0.5f, -lua.window.height * 0.5f, 0.0f);
    rlTranslatef(0.0f, 0.0f, -(lua.window.height * 0.5f) / std::tan(fov * Math::PI_ / 360.0f));

    rlEnableDepthTest();
}

void endDrawing(const Lua &lua)
{
    if (lua.canvas.renderer == Canvas::Renderer::P2D) { return; }

    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    rlDisableDepthTest();
}

// ---------- LUA ----------
void setupScript(std::shared_ptr<Lua> luaptr, const std::string &filename)
{
    sol::state &lua = luaptr->lua;

    // Error Handlers
    lua["__MSG_HANDLER__"] = [](const std::string &msg) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, msg); };
    sol::protected_function::set_default_handler(lua["__MSG_HANDLER__"]);

    lua.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> exception, sol::string_view description) {
        // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
        conditionalExit(MessageType::CPP_ERROR, Message::GENERIC, description.data());
        return sol::stack::push(L, description);
    });

    // Start setup
    luaptr->state = Lua::State::Setup;
    ColorNS::setupColor(luaptr);
    Environment::setupEnvironment(luaptr);
    Math::setupMath(luaptr);
    Output::setupOutput(luaptr);
    LightsCamera::setupLightsCamera(luaptr);
    Shape::setupShape(luaptr);
    TransformNS::setupTransform(luaptr);

    lua.safe_script_file(filename, [](lua_State *L, sol::protected_function_result pfr) {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, pfr.get<std::string>());
        return pfr;
    });
    sol::protected_function setupLua = lua["setup"];
    if (!setupLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "setup"); }
    setupLua();

    if ((luaptr->window.width <= 0) || (luaptr->window.height <= 0))
    {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "window size not valid must be greater than 0");
    }
    SetTargetFPS(luaptr->window.frameRate);
    SetConfigFlags(luaptr->window.flags);
    InitWindow(luaptr->window.width, luaptr->window.height, luaptr->window.title.c_str());

    // Start postSetup
    luaptr->state = Lua::State::PostSetup;
    for (const auto &psFunc : luaptr->postSetupFuncs) { psFunc.function(psFunc.args); }

    // Start draw
    luaptr->state = Lua::State::Draw;
}

void Lua::update()
{
    // TODO: Add mouse and keyboard inputs
}

void Lua::draw()
{
    sol::protected_function drawLua = lua["draw"];
    if (!drawLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "draw"); }

    window.frameCount++;
    canvas.zOrder = 0.0f; // Need to reset every draw call

    beginDrawing(*this);
    drawLua();
    if (canvas.needToPopMatrix)
    {
        rlPopMatrix();
        canvas.needToPopMatrix = false;
    }
    endDrawing(*this);
}
}