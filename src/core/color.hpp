#pragma once

#include "luaproc.hpp"
#include "msghandler.hpp"

#include "raylib.h"

#include <cmath>

namespace LuaProc
{
// COLOR

inline void setupColor(Application &app)
{
    sol::state &lua   = app.lua();

    lua["background"] = [&app](sol::variadic_args va) {
        // (gray)
        // (gray, a)
        // (r, g, b)
        // (r, g, b, a)
        // No support for HSV yet

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "background", "1 to 4", va.size());
        }

        for (const sol::stack_proxy &arg : va)
        {
            if (arg.get_type() != sol::type::number)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "background", solTypeToString(sol::type::number));
            }
            if (arg.as<float>() < 0)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "'background' arguments can only be numbers from 0 to 255");
            }
        }

        Window &window = app.window();
        if (va.size() == 1)
        {
            auto gray         = static_cast<unsigned char>(std::round(va[0].as<float>()));
            window.background = Color{gray, gray, gray, 255};
        }
        else if (va.size() == 2)
        {
            auto gray         = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto alpha        = static_cast<unsigned char>(std::round(va[1].as<float>()));
            window.background = Color{gray, gray, gray, alpha};
        }
        else if (va.size() == 3)
        {
            auto r            = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto g            = static_cast<unsigned char>(std::round(va[1].as<float>()));
            auto b            = static_cast<unsigned char>(std::round(va[2].as<float>()));
            window.background = Color{r, g, b, 255};
        }
        else {
            auto r            = static_cast<unsigned char>(std::round(va[0].as<float>()));
            auto g            = static_cast<unsigned char>(std::round(va[1].as<float>()));
            auto b            = static_cast<unsigned char>(std::round(va[2].as<float>()));
            auto a            = static_cast<unsigned char>(std::round(va[3].as<float>()));
            window.background = Color{r, g, b, a};
        }
    };
}
}