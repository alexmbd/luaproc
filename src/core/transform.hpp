#pragma once

#include "constants.hpp"
#include "luaproc.hpp"
#include "msghandler.hpp"

#include "rlgl.h"

namespace LuaProc
{
// TRANSFORM

inline void setupTransform(Application &app)
{
    sol::state &lua = app.lua();

    // NOTE: All angles in lua are in radians

    lua["rotateX"] = [](sol::variadic_args va) {
        checkArgSize("rotateX", 1, va.size());
        checkArgType("rotateX", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    lua["rotateY"] = [](sol::variadic_args va) {
        checkArgSize("rotateY", 1, va.size());
        checkArgType("rotateY", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    lua["rotateZ"] = [](sol::variadic_args va) {
        checkArgSize("rotateZ", 1, va.size());
        checkArgType("rotateZ", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    lua["rotate"] = [](sol::variadic_args va) {
        checkArgSize("rotate", 1, va.size());
        checkArgType("rotate", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    lua["translate"] = [](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }
        checkArgType("translate", va, sol::type::number);

        float z = va.size() == 3 ? va[2].as<float>() : 0.0f;
        rlTranslatef(va[0].as<float>(), va[1].as<float>(), z);
    };
}
}