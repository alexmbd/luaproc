#include "transform.hpp"
#include "core/constants.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

#include "rlgl.h"

namespace LuaProc
{
namespace TransformNS
{
// ---------- TRANSFORM ----------
void setupTransform(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    // NOTE: All angles in lua are in radians

    lua["popMatrix"] = [](sol::variadic_args va) {
        checkArgSize("popMatrix", 0, va.size());
        rlPopMatrix();
    };

    lua["pushMatrix"] = [](sol::variadic_args va) {
        checkArgSize("pushMatrix", 0, va.size());
        rlPushMatrix();
    };

    lua["rotateX"] = [luaptr](sol::variadic_args va) {
        checkArgSize("rotateX", 1, va.size());
        checkArgType("rotateX", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    lua["rotateY"] = [luaptr](sol::variadic_args va) {
        checkArgSize("rotateY", 1, va.size());
        checkArgType("rotateY", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    lua["rotateZ"] = [luaptr](sol::variadic_args va) {
        checkArgSize("rotateZ", 1, va.size());
        checkArgType("rotateZ", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    lua["rotate"] = [luaptr](sol::variadic_args va) {
        checkArgSize("rotate", 1, va.size());
        checkArgType("rotate", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    lua["scale"] = [luaptr](sol::variadic_args va) {
        if ((va.size() < 1) || (va.size() > 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "scale", "1 to 3", va.size());
        }
        checkArgType("scale", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        auto x = va[0].as<double>();
        auto y = va.size() == 2 ? va[1].as<double>() : x;
        auto z = va.size() == 3 ? va[2].as<double>() : x;
        rlScalef(x, y, z);
    };

    lua["translate"] = [luaptr](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }
        checkArgType("translate", va, sol::type::number);
        if (!luaptr->canvas.needToPopMatrix)
        {
            rlPushMatrix();
            luaptr->canvas.needToPopMatrix = true;
        }
        float z = va.size() == 3 ? va[2].as<double>() : 0.0f;
        rlTranslatef(va[0].as<double>(), va[1].as<double>(), z);
    };
}
}
}