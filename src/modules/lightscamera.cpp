#include "lightscamera.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

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
        checkArgSize("ortho", 0, va.size());
        luaptr->canvas.camera3D.projection = CAMERA_ORTHOGRAPHIC;
    };
}
}
}