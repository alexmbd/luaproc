#pragma once

#include <array>
#include <print>
#include <string>

namespace LuaProc
{
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
constexpr std::array<const char *, static_cast<std::size_t>(MessageType::INVALID)> prefixes{
    "[LUAPROC WARNING (CPP)]", "[LUAPROC ERROR (CPP)]", "[LUAPROC WARNING (LUA)]", "[LUAPROC ERROR (LUA)]"};

constexpr std::array<const std::string_view, static_cast<std::size_t>(Message::INVALID)> templates{
    "'{}' function not found", "'{}' expects {} arguments but got {}", "'{}' expects arguments of type '{}'", "{}"};
}

// This function will log the message and will exit the program if the MessageType is an *_ERROR
template <typename... T> inline void conditionalExit(MessageType msgType, Message msg, T &&...msgArgs)
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
}