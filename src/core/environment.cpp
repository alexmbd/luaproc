#include "environment.hpp"
#include "luaproc.hpp"
#include "utils.hpp"

#include "raylib.h"

namespace LuaProc
{
namespace Environment
{
void cursor(const std::vector<sol::object> &va)
{
    if (va.size() == 0) { return ShowCursor(); }

    checkVASize(va, "cursor", 1);
    checkType(va[0], "cursor", sol::type::number);

    int cursorType = va[0].as<int>();

    ShowCursor();
    if (cursorType > Window::MouseCursor::NOT_ALLOWED || cursorType < 0)
    {
        // Just set to the default cursor if an invalid number was passed (no error thrown)
        SetMouseCursor(Window::MouseCursor::DEFAULT);
        std::println("[LUAPROC WARNING] '{}' was passed as an invalid argument to '{}'. Using default cursor", cursorType, "cursor");
    }
    else {
        SetMouseCursor(cursorType);
    }
}

int displayHeight(const std::vector<sol::object> &va)
{
    checkVASize(va, "displayHeight", 0);

    return GetMonitorHeight(0);
};

int displayWidth(const std::vector<sol::object> &va)
{
    checkVASize(va, "displayWidth", 0);

    return GetMonitorWidth(0);
};

bool focused(const std::vector<sol::object> &va)
{
    checkVASize(va, "focused", 0);

    return IsWindowFocused();
};

void fullScreen(const std::vector<sol::object> &va, Window &window)
{
    checkVASize(va, "fullScreen", 0);

    window.flags |= FLAG_FULLSCREEN_MODE;
};

int frameRate(const std::vector<sol::object> &va, Window &window)
{
    if (va.size() == 0) { return window.frameRate; }

    checkVASize(va, "frameRate", 1);
    checkType(va[0], "frameRate", sol::type::number);

    window.frameRate = va[0].as<int>();
    return window.frameRate;
};

int height(const std::vector<sol::object> &va)
{
    checkVASize(va, "height", 0);

    return GetScreenHeight();
};

void noCursor(const std::vector<sol::object> &va)
{
    checkVASize(va, "noCursor", 0);

    HideCursor();
}

void size(const std::vector<sol::object> &va, Window &window)
{
    checkVASize(va, "size", 2);
    checkType(va[0], "size", sol::type::number);
    checkType(va[1], "size", sol::type::number);

    window.width  = va[0].as<int>();
    window.height = va[1].as<int>();
};

int width(const std::vector<sol::object> &va)
{
    checkVASize(va, "width", 0);

    return GetScreenWidth();
};

void windowMove(const std::vector<sol::object> &va)
{
    checkVASize(va, "windowMove", 2);
    checkType(va[0], "windowMove", sol::type::number);
    checkType(va[1], "windowMove", sol::type::number);

    SetWindowPosition(va[0].as<int>(), va[1].as<int>());
};

void windowResizable(const std::vector<sol::object> &va, Window &window)
{
    checkVASize(va, "windowResizable", 1);
    checkType(va[0], "windowResizable", sol::type::boolean);

    if (va[0].as<bool>()) { window.flags |= FLAG_WINDOW_RESIZABLE; }
};

void windowResize(const std::vector<sol::object> &va)
{
    checkVASize(va, "windowResize", 2);
    checkType(va[0], "windowResize", sol::type::number);
    checkType(va[1], "windowResize", sol::type::number);

    SetWindowSize(va[0].as<int>(), va[1].as<int>());
};

void windowTitle(const std::vector<sol::object> &va, Window &window)
{
    checkVASize(va, "windowTitle", 1);
    checkType(va[0], "windowTitle", sol::type::string);

    window.title = va[0].as<std::string>();
};
}
}