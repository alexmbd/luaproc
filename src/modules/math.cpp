#include "math.hpp"
#include "core/constants.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

#include <cmath>

namespace LuaProc
{
namespace Math
{
// ---------- MATH ----------
void setupMath(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    // NOTE: All angles in lua are in radians

    lua["HALF_PI"]    = Math::HALF_PI;
    lua["PI"]         = Math::PI_;
    lua["QUARTER_PI"] = Math::QUARTER_PI;
    lua["TWO_PI"]     = Math::TWO_PI;
    lua["TAU"]        = Math::TWO_PI;

    lua["abs"]        = [](sol::variadic_args va) {
        checkArgSize("abs", 1, va.size());
        checkArgType("abs", va, sol::type::number);
        return std::abs(va[0].as<double>());
    };

    lua["cos"] = [](sol::variadic_args va) {
        checkArgSize("cos", 1, va.size());
        checkArgType("cos", va, sol::type::number);
        return std::cos(va[0].as<double>());
    };

    lua["degrees"] = [](sol::variadic_args va) {
        checkArgSize("degrees", 1, va.size());
        checkArgType("degrees", va, sol::type::number);
        return va[0].as<double>() * (180 / Math::PI_);
    };

    lua["max"] = [](sol::variadic_args va) {
        // TODO: Not implemented yet
        // max(array)
        checkArgType("max", va, sol::type::number);
        double value = 0.0;
        for (const sol::stack_proxy &arg : va) { value = std::max(value, arg.as<double>()); }
        return value;
    };

    lua["min"] = [](sol::variadic_args va) {
        // TODO: Not implemented yet
        // min(array)
        checkArgType("min", va, sol::type::number);
        double value = 0.0;
        for (const sol::stack_proxy &arg : va) { value = std::min(value, arg.as<double>()); }
        return value;
    };

    lua["radians"] = [](sol::variadic_args va) {
        checkArgSize("radians", 1, va.size());
        checkArgType("radians", va, sol::type::number);
        return va[0].as<double>() * (Math::PI_ / 180);
    };

    lua["sin"] = [](sol::variadic_args va) {
        checkArgSize("sin", 1, va.size());
        checkArgType("sin", va, sol::type::number);
        return std::sin(va[0].as<double>());
    };

    lua["sqrt"] = [](sol::variadic_args va) {
        checkArgSize("sqrt", 1, va.size());
        checkArgType("sqrt", va, sol::type::number);
        auto value = va[0].as<double>();
        if (value < 0) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "'sqrt' argument should be non-negative"); }
        return std::sqrt(value);
    };
}
}
}