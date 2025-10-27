#pragma once

#include "sol.hpp"

#include <functional>
#include <variant>
#include <vector>

namespace LuaProc
{
// ENVIRONMENT

using FuncVoid         = std::function<void(const std::vector<sol::object> &)>;
using PostSetupVariant = std::variant<FuncVoid>;

// HANDLER

template <typename VariantType> struct Function
{
    VariantType function;
    std::vector<sol::object> args;
};

template <typename VariantType> using FunctionHandler = std::vector<Function<VariantType>>;
}