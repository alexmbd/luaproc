#pragma once

#include "safesol.hpp"

#include <array>
#include <print>
#include <string>
#include <vector>

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

enum class MessageType
{
    CPP_WARNING,
    CPP_ERROR,
    LUA_WARNING,
    LUA_ERROR,
    INVALID
};

enum class Message
{
    FUNC_NOT_FOUND,
    UNEXPECTED_ARG_COUNT,
    UNEXPECTED_ARG_TYPE,
    GENERIC,
    INVALID
};

namespace Messages
{
inline constexpr std::array<const char *, static_cast<std::size_t>(MessageType::INVALID)> prefixes{
    "[LUAPROC WARNING (CPP)]", "[LUAPROC ERROR (CPP)]", "[LUAPROC WARNING (LUA)]", "[LUAPROC ERROR (LUA)]"};

inline constexpr std::array<const std::string_view, static_cast<std::size_t>(Message::INVALID)> templates{
    "'{}' function not found", "'{}' expects {} arguments but got {}", "'{}' expects arguments of type '{}'", "{}"};
}

// This function will log the message and will exit the program if the MessageType is an *_ERROR
template <typename... T>
inline void conditionalExit(MessageType msgType, Message msg, T &&...msgArgs)
{
    std::println("{} {}", Messages::prefixes[static_cast<std::size_t>(msgType)],
                 std::vformat(Messages::templates[static_cast<std::size_t>(msg)], std::make_format_args(msgArgs...)));

    switch (msgType)
    {
    case MessageType::CPP_WARNING:
    case MessageType::LUA_WARNING:
        break;

    case MessageType::CPP_ERROR:
    case MessageType::LUA_ERROR:
        std::exit(-1);

    default:
        std::println("{} Invalid MessageType", Messages::prefixes[static_cast<std::size_t>(MessageType::CPP_ERROR)]);
        std::exit(-1);
    }
}

inline void checkArgSize(const std::string &name, int expectedSize, int size)
{
    if (expectedSize == size) { return; }
    conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, name, expectedSize, size);
}

inline void checkArgType(const std::string &name, const sol::variadic_args &va, sol::type type)
{
    for (const sol::stack_proxy &arg : va)
    {
        if (arg.get_type() == type) { continue; }
        conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, name, solTypeToString(type));
    }
}

inline void checkArgType(const std::string &name, const std::vector<sol::object> &va, sol::type type)
{
    for (const sol::object &arg : va)
    {
        if (arg.get_type() == type) { continue; }
        conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, name, solTypeToString(type));
    }
}
}