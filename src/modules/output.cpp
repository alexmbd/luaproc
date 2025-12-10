#include "output.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

namespace LuaProc
{
namespace Output
{
void print(sol::variadic_args va, bool newline)
{
    // Get Type (https://github.com/ThePhD/sol2/issues/850#issuecomment-515720422)
    std::string text = "";
    for (int i = 0; i < va.size(); i++)
    {
        if (va[i].get_type() == sol::type::nil) { text += "nil"; }
        else if (va[i].get_type() == sol::type::boolean) { text += va[i].as<bool>() ? "true" : "false"; }
        else if (va[i].get_type() == sol::type::number) { text += std::format("{}", va[i].as<double>()); }
        else
        {
            text += va[i].as<std::string>();
        }

        if (i != va.size() - 1) { text += " "; }
    }
    if (newline) { std::println("{}", text); }
    else
    {
        std::print("{}", text);
    }
}

// ---------- OUTPUT ----------
void setupOutput(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua["print"]    = [](sol::variadic_args va) { Output::print(va, false); };
    lua["println"]  = [](sol::variadic_args va) { Output::print(va, true); };
}
}
}