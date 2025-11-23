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
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "rotateX", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "rotateX", solTypeToString(sol::type::number));
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    lua["rotateY"] = [](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "rotateY", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "rotateY", solTypeToString(sol::type::number));
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    lua["rotateZ"] = [](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "rotateZ", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "rotateZ", solTypeToString(sol::type::number));
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    lua["rotate"] = [](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "rotate", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "rotate", solTypeToString(sol::type::number));
        }
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    lua["translate"] = [](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }

        for (const sol::stack_proxy &arg : va)
        {
            if (arg.get_type() != sol::type::number)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "translate", solTypeToString(sol::type::number));
            }
        }

        float z = va.size() == 3 ? va[2].as<float>() : 0.0f;
        rlTranslatef(va[0].as<float>(), va[1].as<float>(), z);
    };
}
}