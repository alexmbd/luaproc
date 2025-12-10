#include "shape.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

namespace LuaProc
{
namespace Shape
{
// ---------- SHAPE ----------
void setupShape(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua["line"]     = [luaptr](sol::variadic_args va) {
        if ((va.size() != 4) && (va.size() != 6))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "line", "4 or 6", va.size());
        }
        checkArgType("line", va, sol::type::number);
        if (va.size() == 4)
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), 0.0f}, Vector3{va[2].as<float>(), va[3].as<float>(), 0.0f},
                           luaptr->canvas.stroke);
        }
        else
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), va[2].as<float>()},
                           Vector3{va[3].as<float>(), va[4].as<float>(), va[5].as<float>()}, luaptr->canvas.stroke);
        }
    };

    lua["rect"] = [luaptr](sol::variadic_args va) {
        // TODO: Not implemented yet
        // rect(a, b, c, d, r)
        // rect(a, b, c, d, tl, tr, br, bl)
        checkArgSize("rect", 4, va.size());
        checkArgType("rect", va, sol::type::number);
        auto rect = Rectangle{va[0].as<float>(), va[1].as<float>(), va[2].as<float>(), va[3].as<float>()};
        if (!luaptr->canvas.noFill) { DrawRectangleRec(rect, luaptr->canvas.fill); }
        if (!luaptr->canvas.noStroke) { DrawRectangleLinesEx(rect, 1.0f, luaptr->canvas.stroke); }
    };

    lua["sphere"] = [luaptr](sol::variadic_args va) {
        checkArgSize("sphere", 1, va.size());
        checkArgType("sphere", va, sol::type::number);
        DrawSphere(Vector3{0.0f, 0.0f, 0.0f}, va[0].as<double>(), luaptr->canvas.fill);
    };
}
}
}