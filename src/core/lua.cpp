#include "lua.hpp"
#include "constants.hpp"
#include "msghandler.hpp"

#include "rlgl.h"

#include <cmath>

namespace LuaProc
{
namespace ColorNS
{
void checkColorArg(const std::string &name, const sol::variadic_args &va, Canvas::ColorMode colorMode)
{
    for (const sol::stack_proxy &arg : va)
    {
        if (arg.get_type() != sol::type::number)
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_TYPE, name, solTypeToString(sol::type::number));
        }
        if (colorMode == Canvas::ColorMode::RGB)
        {
            if ((arg.as<double>() < 0) || (arg.as<double>() > 255))
            {
                conditionalExit(MessageType::LUA_ERROR, Message::GENERIC,
                                std::format("'{}' arguments can only be numbers from 0 to 255 in RGB mode", name));
            }
        }
        else if (colorMode == Canvas::ColorMode::HSB)
        {
            if ((arg.as<double>() < 0) || (arg.as<double>() > 1))
            {
                conditionalExit(MessageType::LUA_ERROR, Message::GENERIC,
                                std::format("'{}' arguments can only be numbers from 0 to 1 in HSB mode", name));
            }
        }
    }
}

Color toColor(Canvas::ColorMode colorMode, double a, double b, double c, double alpha)
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
        // a -> [0, 255]
        // b, c -> [0, 1]
        Color color = ColorFromHSV(a * 255, b, c);
        color.a     = static_cast<unsigned char>(alpha * 255);
        return color;
    }
}

void setColor(const sol::variadic_args &va, Color &color, Canvas::ColorMode colorMode)
{
    if (va.size() == 1)
    {
        auto gray = va[0].as<double>();
        if (colorMode == Canvas::ColorMode::HSB) { color = toColor(colorMode, 0.0, 0.0, gray, 1.0); }
        else
        {
            color = toColor(colorMode, gray, gray, gray, 255.0);
        }
    }
    else if (va.size() == 2)
    {
        auto gray = va[0].as<double>();
        if (colorMode == Canvas::ColorMode::HSB) { color = toColor(colorMode, 0.0, 0.0, gray, va[1].as<double>()); }
        else
        {
            color = toColor(colorMode, gray, gray, gray, va[1].as<double>());
        }
    }
    else if (va.size() == 3)
    {
        double alpha = colorMode == Canvas::ColorMode::HSB ? 1.0 : 255.0;
        color        = toColor(colorMode, va[0].as<double>(), va[1].as<double>(), va[2].as<double>(), alpha);
    }
    else
    {
        color = toColor(colorMode, va[0].as<double>(), va[1].as<double>(), va[2].as<double>(), va[3].as<double>());
    }
}

// ---------- COLOR ----------
void setupColor(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua   = luaptr->lua;
    lua["RGB"]        = static_cast<int>(Canvas::ColorMode::RGB);
    lua["HSB"]        = static_cast<int>(Canvas::ColorMode::HSB);

    lua["background"] = [luaptr](sol::variadic_args va) {
        // (gray)
        // (gray, a)
        // (r, g, b)
        // (r, g, b, a)

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "background", "1 to 4", va.size());
        }
        checkColorArg("background", va, luaptr->canvas.colorMode);
        setColor(va, luaptr->canvas.background, luaptr->canvas.colorMode);
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
        // (gray)
        // (gray, a)
        // (r, g, b)
        // (r, g, b, a)
        // TODO: No color value in hexadecimal notation yet

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "fill", "1 to 4", va.size());
        }
        checkColorArg("fill", va, luaptr->canvas.colorMode);
        setColor(va, luaptr->canvas.fill, luaptr->canvas.colorMode);
        luaptr->canvas.noFill = false;
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
        // (gray)
        // (gray, a)
        // (r, g, b)
        // (r, g, b, a)
        // TODO: No color value in hexadecimal notation yet

        if ((va.size() == 0) || (va.size() > 4))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "stroke", "1 to 4", va.size());
        }
        checkColorArg("stroke", va, luaptr->canvas.colorMode);
        setColor(va, luaptr->canvas.stroke, luaptr->canvas.colorMode);
        luaptr->canvas.noStroke = false;
    };
}
}

namespace Environment
{
void cursor(const std::vector<sol::object> &va)
{
    if (va.size() == 0) { return ShowCursor(); }

    checkArgSize("cursor", 1, va.size());
    checkArgType("cursor", va, sol::type::number);

    int cursorType = va[0].as<int>();

    ShowCursor();
    if (cursorType > MOUSE_CURSOR_NOT_ALLOWED || cursorType < 0)
    {
        // Just set to the default cursor if an invalid number was passed (no error thrown)
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        std::println("[LUAPROC WARNING] '{}' was passed as an invalid argument to '{}'. Using default cursor", cursorType, "cursor");
    }
    else
    {
        SetMouseCursor(cursorType);
    }
}

void noCursor(const std::vector<sol::object> &va)
{
    checkArgSize("noCursor", 0, va.size());
    HideCursor();
};

// ---------- ENVIRONMENT ----------
// ENVIRONMENT (Not implemented)
// delay, displayDensity
// frameCount
// noSmooth
// pixelDensity, pixelHeight, pixelWidth
// settings, smooth
// windowMoved, windowRatio, windowResized

// ENVIRONMENT (API changes)
// frameRate variable and function merged

void setupEnvironment(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua      = luaptr->lua;
    lua["DEFAULT"]       = MOUSE_CURSOR_DEFAULT;
    lua["ARROW"]         = MOUSE_CURSOR_ARROW;
    lua["IBEAM"]         = MOUSE_CURSOR_IBEAM;
    lua["CROSSHAIR"]     = MOUSE_CURSOR_CROSSHAIR;
    lua["POINTING_HAND"] = MOUSE_CURSOR_POINTING_HAND;
    lua["RESIZE_EW"]     = MOUSE_CURSOR_RESIZE_EW;
    lua["RESIZE_NS"]     = MOUSE_CURSOR_RESIZE_NS;
    lua["RESIZE_NWSE"]   = MOUSE_CURSOR_RESIZE_NWSE;
    lua["RESIZE_NESW"]   = MOUSE_CURSOR_RESIZE_NESW;
    lua["RESIZE_ALL"]    = MOUSE_CURSOR_RESIZE_ALL;
    lua["NOT_ALLOWED"]   = MOUSE_CURSOR_NOT_ALLOWED;

    lua["P2D"]           = static_cast<int>(Canvas::Renderer::P2D);
    lua["P3D"]           = static_cast<int>(Canvas::Renderer::P3D);

    lua["cursor"]        = [luaptr](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (luaptr->state == Lua::State::Setup)
        {
            luaptr->postSetupFuncs.emplace_back(Environment::cursor, vec);
            return;
        }
        Environment::cursor(vec);
    };

    lua["displayHeight"] = [](sol::variadic_args va) {
        checkArgSize("displayHeight", 0, va.size());
        return GetMonitorHeight(0);
    };

    lua["displayWidth"] = [](sol::variadic_args va) {
        checkArgSize("displayWidth", 0, va.size());
        return GetMonitorWidth(0);
    };

    lua["focused"] = [](sol::variadic_args va) {
        checkArgSize("focused", 0, va.size());
        return IsWindowFocused();
    };

    lua["fullScreen"] = [luaptr](sol::variadic_args va) {
        checkArgSize("fullScreen", 0, va.size());
        luaptr->window.flags |= FLAG_FULLSCREEN_MODE;
    };

    lua["frameRate"] = [luaptr](sol::variadic_args va) {
        if (va.size() > 1) { conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "frameRate", "0 to 1", va.size()); }
        if (va.size() == 0) { return luaptr->window.frameRate; }
        checkArgType("frameRate", va, sol::type::number);
        luaptr->window.frameRate = va[0].as<int>();
        return luaptr->window.frameRate;
    };

    lua["height"] = [](sol::variadic_args va) {
        checkArgSize("height", 0, va.size());
        return GetScreenHeight();
    };

    lua["noCursor"] = [luaptr](sol::variadic_args va) {
        std::vector<sol::object> vec(va.begin(), va.end());
        if (luaptr->state == Lua::State::Setup)
        {
            luaptr->postSetupFuncs.emplace_back(Environment::noCursor, vec);
            return;
        }
        Environment::noCursor(vec);
    };

    lua["size"] = [luaptr](sol::variadic_args va) {
        if ((va.size() < 2) || (va.size() > 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "size", "2 to 3", va.size());
        }
        checkArgType("size", va, sol::type::number);
        luaptr->window.width    = va[0].as<int>();
        luaptr->window.height   = va[1].as<int>();
        luaptr->canvas.renderer = va.size() == 2 ? Canvas::Renderer::P2D : static_cast<Canvas::Renderer>(va[2].as<int>());

        if (luaptr->canvas.renderer == Canvas::Renderer::P2D)
        {
            // TODO: Recheck position and target values
            luaptr->canvas.camera.position   = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, -1.0f};
            luaptr->canvas.camera.target     = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, 0.0f};
            luaptr->canvas.camera.up         = Vector3{0.0f, -1.0f, 0.0f};
            luaptr->canvas.camera.fovy       = static_cast<float>(luaptr->window.height);
            luaptr->canvas.camera.projection = CAMERA_ORTHOGRAPHIC;
            rlSetClipPlanes(-1000, 1000);
        }
        else if (luaptr->canvas.renderer == Canvas::Renderer::P3D)
        {
            luaptr->canvas.camera.position   = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, -1.0f};
            luaptr->canvas.camera.target     = Vector3{luaptr->window.width / 2.0f, luaptr->window.height / 2.0f, 0.0f};
            luaptr->canvas.camera.up         = Vector3{0.0f, -1.0f, 0.0f};
            luaptr->canvas.camera.fovy       = static_cast<float>(luaptr->window.height);
            luaptr->canvas.camera.projection = CAMERA_PERSPECTIVE;
            rlSetClipPlanes(-1000, 1000);
        }
    };

    lua["width"] = [](sol::variadic_args va) {
        checkArgSize("width", 0, va.size());
        return GetScreenWidth();
    };

    lua["windowMove"] = [](sol::variadic_args va) {
        checkArgSize("windowMove", 2, va.size());
        checkArgType("windowMove", va, sol::type::number);
        SetWindowPosition(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowResizable"] = [luaptr](sol::variadic_args va) {
        checkArgSize("windowResizable", 1, va.size());
        checkArgType("windowResizable", va, sol::type::boolean);
        if (va[0].as<bool>()) { luaptr->window.flags |= FLAG_WINDOW_RESIZABLE; }
    };

    lua["windowResize"] = [](sol::variadic_args va) {
        checkArgSize("windowResize", 2, va.size());
        checkArgType("windowResize", va, sol::type::number);
        SetWindowSize(va[0].as<int>(), va[1].as<int>());
    };

    lua["windowTitle"] = [luaptr](sol::variadic_args va) {
        checkArgSize("windowTitle", 1, va.size());
        checkArgType("windowTitle", va, sol::type::string);
        luaptr->window.title = va[0].as<std::string>();
    };
}
}

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

    lua["cos"]        = [](sol::variadic_args va) {
        checkArgSize("cos", 1, va.size());
        checkArgType("cos", va, sol::type::number);
        return std::cos(va[0].as<double>());
    };

    lua["degrees"] = [](sol::variadic_args va) {
        checkArgSize("degrees", 1, va.size());
        checkArgType("degrees", va, sol::type::number);
        return va[0].as<double>() * (180 / Math::PI_);
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
}
}

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

namespace LightsCamera
{
// ---------- LIGHTS CAMERA ----------
void setupLightsCamera(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua["ortho"]    = [luaptr](sol::variadic_args va) {
        // TODO: Not implemented yet
        // ortho(left, right, bottom, top)
        // ortho(left, right, bottom, top, near, far)
        checkArgSize("ortho", 0, va.size());
        luaptr->canvas.camera.projection = CAMERA_ORTHOGRAPHIC;
    };
}
}

namespace Shape
{
// ---------- SHAPE ----------
void setupShape(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    lua["line"]     = [luaptr](sol::variadic_args va) {
        if ((va.size() != 4) && (va.size() != 6))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "line", "4 or 6", va.size());
        }
        checkArgType("line", va, sol::type::number);
        if (va.size() == 4)
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), 0.0f}, Vector3{va[2].as<float>(), va[3].as<float>(), 0.0f},
                           luaptr->canvas.stroke);
        }
        else
        {
            DrawLine3D(Vector3{va[0].as<float>(), va[1].as<float>(), va[2].as<float>()},
                           Vector3{va[3].as<float>(), va[4].as<float>(), va[5].as<float>()}, luaptr->canvas.stroke);
        }
    };

    lua["sphere"] = [luaptr](sol::variadic_args va) {
        checkArgSize("sphere", 1, va.size());
        checkArgType("sphere", va, sol::type::number);
        DrawSphere(Vector3{0.0f, 0.0f, 0.0f}, va[0].as<double>(), luaptr->canvas.fill);
    };
}
}

namespace TransformNS
{
// ---------- TRANSFORM ----------
void setupTransform(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    // NOTE: All angles in lua are in radians

    lua["popMatrix"] = [](sol::variadic_args va) {
        checkArgSize("popMatrix", 0, va.size());
        rlPopMatrix();
    };

    lua["pushMatrix"] = [](sol::variadic_args va) {
        checkArgSize("pushMatrix", 0, va.size());
        rlPushMatrix();
    };

    lua["rotateX"] = [](sol::variadic_args va) {
        checkArgSize("rotateX", 1, va.size());
        checkArgType("rotateX", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 0.0f, 0.0f);
    };

    lua["rotateY"] = [](sol::variadic_args va) {
        checkArgSize("rotateY", 1, va.size());
        checkArgType("rotateY", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 1.0f, 0.0f);
    };

    lua["rotateZ"] = [](sol::variadic_args va) {
        checkArgSize("rotateZ", 1, va.size());
        checkArgType("rotateZ", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 0.0f, 0.0f, 1.0f);
    };

    lua["rotate"] = [](sol::variadic_args va) {
        checkArgSize("rotate", 1, va.size());
        checkArgType("rotate", va, sol::type::number);
        rlRotatef(va[0].as<double>() * (180 / Math::PI_), 1.0f, 1.0f, 1.0f);
    };

    lua["translate"] = [](sol::variadic_args va) {
        if ((va.size() != 2) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "translate", "2 to 3", va.size());
        }
        checkArgType("translate", va, sol::type::number);

        float z = va.size() == 3 ? va[2].as<double>() : 0.0f;
        rlTranslatef(va[0].as<double>(), va[1].as<double>(), z);
    };
}
}

// ---------- LUA ----------
void setupScript(std::shared_ptr<Lua> luaptr, const std::string &filename)
{
    sol::state &lua = luaptr->lua;

    // Error Handlers
    lua["__MSG_HANDLER__"] = [](const std::string &msg) { conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, msg); };
    sol::protected_function::set_default_handler(lua["__MSG_HANDLER__"]);

    lua.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> exception, sol::string_view description) {
        // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
        conditionalExit(MessageType::CPP_ERROR, Message::GENERIC, description.data());
        return sol::stack::push(L, description);
    });

    // Start setup
    luaptr->state = Lua::State::Setup;
    ColorNS::setupColor(luaptr);
    Environment::setupEnvironment(luaptr);
    Math::setupMath(luaptr);
    Output::setupOutput(luaptr);
    LightsCamera::setupLightsCamera(luaptr);
    Shape::setupShape(luaptr);
    TransformNS::setupTransform(luaptr);

    lua.safe_script_file(filename, [](lua_State *L, sol::protected_function_result pfr) {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, pfr.get<std::string>());
        return pfr;
    });
    sol::protected_function setupLua = lua["setup"];
    if (!setupLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "setup"); }
    setupLua();

    if ((luaptr->window.width <= 0) || (luaptr->window.height <= 0))
    {
        conditionalExit(MessageType::LUA_ERROR, Message::GENERIC, "window size not valid must be greater than 0");
    }
    SetTargetFPS(luaptr->window.frameRate);
    SetConfigFlags(luaptr->window.flags);
    InitWindow(luaptr->window.width, luaptr->window.height, luaptr->window.title.c_str());

    // Start postSetup
    luaptr->state = Lua::State::PostSetup;
    for (const auto &psFunc : luaptr->postSetupFuncs) { psFunc.function(psFunc.args); }

    // Start draw
    luaptr->state = Lua::State::Draw;
}

void Lua::update()
{
    if (IsWindowResized())
    {
        if (canvas.renderer == Canvas::Renderer::P2D)
        {
            float width            = static_cast<float>(GetScreenWidth());
            float height           = static_cast<float>(GetScreenHeight());
            canvas.camera.position = Vector3{width / 2.0f, height / 2.0f, -1.0f};
            canvas.camera.target   = Vector3{width / 2.0f, height / 2.0f, 0.0f};
            canvas.camera.fovy     = height;
        }
    }
}

void Lua::draw()
{
    sol::protected_function drawLua = lua["draw"];
    if (!drawLua.valid()) { conditionalExit(MessageType::LUA_ERROR, Message::FUNC_NOT_FOUND, "draw"); }
    drawLua();
}
}