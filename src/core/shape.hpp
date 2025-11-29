#pragma once

#include "luaproc.hpp"
#include "msghandler.hpp"

namespace LuaProc
{
// SHAPE

inline void setupShape(Application &app)
{
    sol::state &lua = app.lua();

    lua["line"]     = [](sol::variadic_args va) {
        if ((va.size() != 4) && (va.size() != 6))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "line", "4 or 6", va.size());
        }
        checkArgType("line", va, sol::type::number);
        if (va.size() == 4)
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), 0.0f}, Vector3{va[2].as<float>(), va[3].as<float>(), 0.0f}, WHITE);
        }
        else {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), va[2].as<float>()},
                           Vector3{va[3].as<float>(), va[4].as<float>(), va[5].as<float>()}, WHITE);
        }
    };
}
}