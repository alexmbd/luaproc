#include "lightscamera.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

#include "rlgl.h"

namespace LuaProc
{
namespace LightsCamera
{
// ---------- LIGHTS CAMERA ----------
void setupLightsCamera(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua["ortho"]    = [luaptr](sol::variadic_args va) {
        // TODO: Not implemented yet
        // ortho(left, right, bottom, top)
        // ortho(left, right, bottom, top, near, far)
        if (luaptr->canvas.renderer != Canvas::Renderer::P3D)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "'ortho' is only available in P3D");
        }
        checkArgSize("ortho", 0, va.size());
        luaptr->canvas.camera3D.fovy = static_cast<float>(luaptr->window.height);
        luaptr->canvas.camera3D.position =
            Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, -luaptr->canvas.camera3D.fovy};
        luaptr->canvas.camera3D.target     = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, 0.0f};
        luaptr->canvas.camera3D.up         = Vector3{0.0f, -1.0f, 0.0f};
        luaptr->canvas.camera3D.projection = CAMERA_ORTHOGRAPHIC;
        rlSetClipPlanes(0.0f, std::max(luaptr->window.width, luaptr->window.height) + 800.0f);
    };
}
}
}