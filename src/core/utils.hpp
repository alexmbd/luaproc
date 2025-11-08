#pragma once

#include "sol.hpp"

#include <print>

namespace LuaProc
{
inline std::string solTypeToString(sol::type type)
{
    switch (type)
    {
    case sol::type::boolean:
        return "boolean";

    case sol::type::function:
        return "function";

    case sol::type::lightuserdata:
        return "lightuserdata";

    case sol::type::nil:
        return "nil";

    case sol::type::number:
        return "number";

    case sol::type::string:
        return "string";

    case sol::type::table:
        return "table";

    case sol::type::thread:
        return "thread";

    case sol::type::userdata:
        return "userdata";

    default:
        return "unknown type";
    }
}

inline void checkVASize(const std::vector<sol::object> &va, const std::string &name, int expectedSize)
{
    if (va.size() == expectedSize) { return; }
    std::println("[LUAPROC ERROR] '{}' expects {} arguments but got {}", name, expectedSize, va.size());
    std::exit(-1);
}

inline void checkVASize(const sol::variadic_args &va, const std::string &name, int expectedSize)
{
    if (va.size() == expectedSize) { return; }
    std::println("[LUAPROC ERROR] '{}' expects {} arguments but got {}", name, expectedSize, va.size());
    std::exit(-1);
}

inline void checkType(const sol::object &var, const std::string &name, sol::type type)
{
    if (var.get_type() == type) { return; }
    std::println("[LUAPROC ERROR] '{}' expects arguments of type '{}'", name, solTypeToString(type));
    std::exit(-1);
}
}