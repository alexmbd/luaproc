#pragma once

#include "constants.hpp"
#include "luaproc.hpp"
#include "msghandler.hpp"

namespace LuaProc
{
// MATH

inline void setupMath(Application &app)
{
    sol::state &lua   = app.lua();

    lua["HALF_PI"]    = Math::HALF_PI;
    lua["PI"]         = Math::PI_;
    lua["QUARTER_PI"] = Math::QUARTER_PI;
    lua["TWO_PI"]     = Math::TWO_PI;
    lua["TAU"]        = Math::TWO_PI;

    lua["degrees"]    = [](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "degrees", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "degrees", solTypeToString(sol::type::number));
        }
        return va[0].as<double>() * (180 / Math::PI_);
    };

    lua["radians"] = [](sol::variadic_args va) {
        if (va.size() != 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "radians", 1, va.size()); }
        if (va[0].get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "radians", solTypeToString(sol::type::number));
        }
        return va[0].as<double>() * (Math::PI_ / 180);
    };
}
}