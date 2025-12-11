#include "color.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

namespace LuaProc
{
namespace ColorNS
{
void checkColorBound(const std::string &name, const double value, Canvas::ColorMode colorMode)
{
    if (colorMode == Canvas::ColorMode::RGB)
    {
        if ((value >= 0) && (value <= 255)) { return; }
        conditionalExit(
            MessageType::LUA_ERROR, Message::GENERIC,
            std::format("'{}' arguments can only be numbers from 0 to 255 in RGB mode (base 10) or in hexadecimal (base 16)", name));
    }
    else
    {
        if ((value >= 0) && (value <= 1)) { return; }
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC,
                        std::format("'{}' arguments can only be numbers from 0 to 1 in HSB mode", name));
    }
}

void checkColorArg(const std::string &name, const sol::variadic_args &va, Canvas::ColorMode colorMode)
{
    for (const sol::stack_proxy &arg : va)
    {
        if (arg.get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, name, solTypeToString(sol::type::number));
        }
        if ((va.size() == 1) && (colorMode == Canvas::ColorMode::RGB)) { continue; }
        checkColorBound(name, arg.as<double>(), colorMode);
    }
}

Color parseColorMode(Canvas::ColorMode colorMode, double a, double b, double c, double alpha)
{
    if (colorMode == Canvas::ColorMode::RGB)
    {
        // RGB
        // a, b, c -> [0, 255]
        return Color{static_cast<unsigned char>(a), static_cast<unsigned char>(b), static_cast<unsigned char>(c),
                     static_cast<unsigned char>(alpha)};
    }
    else
    {
        // HSB
        // a -> [0, 1] * 360
        // b, c -> [0, 1]
        Color color = ColorFromHSV(a * 360, b, c);
        color.a     = static_cast<unsigned char>(alpha * 255);
        return color;
    }
}

Color parseColor(const sol::variadic_args &va, Canvas::ColorMode colorMode)
{
    if (va.size() == 1)
    {
        auto gray = va[0].as<double>();
        if (gray > 255)
        {
            // Treat input as hex 0x
            unsigned int hexValue = static_cast<unsigned int>(gray);
            unsigned char r       = (hexValue >> 16) & 0xFF;
            unsigned char g       = (hexValue >> 8) & 0xFF;
            unsigned char b       = (hexValue) & 0xFF;
            return parseColorMode(colorMode, r, g, b, 255.0);
        }
        else
        {
            double value = colorMode == Canvas::ColorMode::HSB ? 0.0 : gray;
            double alpha = colorMode == Canvas::ColorMode::HSB ? 1.0 : 255.0;
            return parseColorMode(colorMode, value, value, gray, alpha);
        }
    }
    else if (va.size() == 2)
    {
        auto gray    = va[0].as<double>();
        double value = colorMode == Canvas::ColorMode::HSB ? 0.0 : gray;
        return parseColorMode(colorMode, value, value, gray, va[1].as<double>());
    }
    else if (va.size() == 3)
    {
        double alpha = colorMode == Canvas::ColorMode::HSB ? 1.0 : 255.0;
        return parseColorMode(colorMode, va[0].as<double>(), va[1].as<double>(), va[2].as<double>(), alpha);
    }
    else
    {
        return parseColorMode(colorMode, va[0].as<double>(), va[1].as<double>(), va[2].as<double>(), va[3].as<double>());
    }
};

// ---------- COLOR ----------
void setupColor(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua.new_usertype<Color>("Color",
                            sol::constructors<Color(), Color(unsigned char), Color(unsigned char, unsigned char),
                                              Color(unsigned char, unsigned char, unsigned char),
                                              Color(unsigned char, unsigned char, unsigned char, unsigned char)>(),
                            "r", &Color::r, "g", &Color::g, "b", &Color::b, "a", &Color::a);

    lua["RGB"]        = static_cast<int>(Canvas::ColorMode::RGB);
    lua["HSB"]        = static_cast<int>(Canvas::ColorMode::HSB);

    lua["background"] = [luaptr](sol::variadic_args va) {
        // background(gray)
        // background(colorObject)
        // background(gray, a)
        // background(r, g, b)
        // background(r, g, b, a)

        if ((va.size() == 1) && (va[0].is<Color>()))
        {
            luaptr->canvas.background = va[0].as<Color>();
            return;
        }

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "background", "1 to 4", va.size());
        }
        checkColorArg("background", va, luaptr->canvas.colorMode);
        luaptr->canvas.background = parseColor(va, luaptr->canvas.colorMode);
    };

    lua["color"] = [luaptr](sol::variadic_args va) {
        // color(gray)
        // color(gray, a)
        // color(r, g, b)
        // color(r, g, b, a)

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "color", "1 to 4", va.size());
        }
        checkColorArg("color", va, luaptr->canvas.colorMode);
        return parseColor(va, luaptr->canvas.colorMode);
    };

    lua["colorMode"] = [luaptr](sol::variadic_args va) {
        // TODO: Not implemented yet
        // colorMode(mode, max)
        // colorMode(mode, max1, max2, max3)
        // colorMode(mode, max1, max2, max3, maxA)
        checkArgSize("colorMode", 1, va.size());
        checkArgType("colorMode", va, sol::type::number);
        luaptr->canvas.colorMode = static_cast<Canvas::ColorMode>(va[0].as<int>());
    };

    lua["fill"] = [luaptr](sol::variadic_args va) {
        // fill(gray)
        // fill(colorObject)
        // fill(gray, a)
        // fill(r, g, b)
        // fill(r, g, b, a)

        if ((va.size() == 1) && (va[0].is<Color>()))
        {
            luaptr->canvas.fill = va[0].as<Color>();
            return;
        }

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "fill", "1 to 4", va.size());
        }
        checkColorArg("fill", va, luaptr->canvas.colorMode);
        luaptr->canvas.fill   = parseColor(va, luaptr->canvas.colorMode);
        luaptr->canvas.noFill = false;
    };

    lua["lerpColor"] = [luaptr](sol::variadic_args va) {
        // lerpColor(colorObject)
        // lerpColor(hexCode)
        checkArgSize("lerpColor", 3, va.size());
        Color from;
        if (va[0].is<Color>()) { from = va[0].as<Color>(); }
        else
        {
            if (va[0].get_type() != sol::type::number)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "lerpColor", "number or Color");
            }
            unsigned int fromValue = va[0].as<unsigned int>();
            unsigned char r        = (fromValue >> 16) & 0xFF;
            unsigned char g        = (fromValue >> 8) & 0xFF;
            unsigned char b        = (fromValue) & 0xFF;
            from                   = parseColorMode(luaptr->canvas.colorMode, r, g, b, 255.0);
        }

        Color to;
        if (va[1].is<Color>()) { to = va[1].as<Color>(); }
        else
        {
            if (va[1].get_type() != sol::type::number)
            {
                conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, "lerpColor", "number or Color");
            }
            unsigned int toValue = va[1].as<unsigned int>();
            unsigned char r      = (toValue >> 16) & 0xFF;
            unsigned char g      = (toValue >> 8) & 0xFF;
            unsigned char b      = (toValue) & 0xFF;
            to                   = parseColorMode(luaptr->canvas.colorMode, r, g, b, 255.0);
        }
        return ColorLerp(from, to, va[2].as<float>());
    };

    lua["noFill"] = [luaptr](sol::variadic_args va) {
        checkArgSize("noFill", 0, va.size());
        luaptr->canvas.noFill = true;
    };

    lua["noStroke"] = [luaptr](sol::variadic_args va) {
        checkArgSize("noStroke", 0, va.size());
        luaptr->canvas.noStroke = true;
    };

    lua["stroke"] = [luaptr](sol::variadic_args va) {
        // stroke(gray)
        // stroke(colorObject)
        // stroke(gray, a)
        // stroke(r, g, b)
        // stroke(r, g, b, a)

        if ((va.size() == 1) && (va[0].is<Color>()))
        {
            luaptr->canvas.stroke = va[0].as<Color>();
            return;
        }

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "stroke", "1 to 4", va.size());
        }
        checkColorArg("stroke", va, luaptr->canvas.colorMode);
        luaptr->canvas.stroke   = parseColor(va, luaptr->canvas.colorMode);
        luaptr->canvas.noStroke = false;
    };
}
}
}