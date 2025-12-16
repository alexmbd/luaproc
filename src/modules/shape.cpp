#include "shape.hpp"
#include "core/lua.hpp"
#include "core/msghandler.hpp"

#include "rlgl.h"

namespace LuaProc
{
namespace Shape
{
void DrawRectangle3D(const Rectangle &rec, float z, const Color &color)
{
    DrawCubeV(Vector3{rec.x + rec.width * 0.5f, rec.y + rec.height * 0.5f, z}, Vector3{rec.width, rec.height, 0.01f}, color);
}

void DrawRectangleLines3D(const Rectangle &rec, float z, float borderThickness, const Color &color)
{
    Vector3 center = {rec.x + rec.width * 0.5f, rec.y + rec.height * 0.5f, z};
    if (borderThickness <= 0) { return; }

    float halfThickness = borderThickness * 0.5f;
    // Top
    DrawCubeV(Vector3{center.x, rec.y + halfThickness, z + 0.001f}, Vector3{rec.width, borderThickness, 0.02f}, color);
    // Bottom
    DrawCubeV(Vector3{center.x, rec.y + rec.height - halfThickness, z + 0.001f}, Vector3{rec.width, borderThickness, 0.02f}, color);
    // Left
    DrawCubeV(Vector3{rec.x + halfThickness, center.y, z + 0.001f}, Vector3{borderThickness, rec.height, 0.02f}, color);
    // Right
    DrawCubeV(Vector3{rec.x + rec.width - halfThickness, center.y, z + 0.001f}, Vector3{borderThickness, rec.height, 0.02f}, color);
}

// ---------- SHAPE ----------
void setupShape(std::shared_ptr<Lua> luaptr)
{
    sol::state &lua = luaptr->lua;

    // 2D Primitives

    lua["line"] = [luaptr](sol::variadic_args va) {
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

    lua["rect"] = [luaptr](sol::variadic_args va) {
        // TODO: Not implemented yet
        // rect(a, b, c, d, r)
        // rect(a, b, c, d, tl, tr, br, bl)
        checkArgSize("rect", 4, va.size());
        checkArgType("rect", va, sol::type::number);
        auto rect = Rectangle{va[0].as<float>(), va[1].as<float>(), va[2].as<float>(), va[3].as<float>()};
        if (luaptr->canvas.renderer == Canvas::Renderer::P2D)
        {
            if (!luaptr->canvas.noFill) { DrawRectangleRec(rect, luaptr->canvas.fill); }
            if (!luaptr->canvas.noStroke) { DrawRectangleLinesEx(rect, 1.0f, luaptr->canvas.stroke); }
        }
        else
        {
            if (!luaptr->canvas.noFill) { DrawRectangle3D(rect, luaptr->canvas.zOrder, luaptr->canvas.fill); }
            if (!luaptr->canvas.noStroke) { DrawRectangleLines3D(rect, luaptr->canvas.zOrder, 1.0f, luaptr->canvas.stroke); }
            luaptr->canvas.zOrder += 0.1f;
        }
    };

    // 3D Primitives

    lua["box"] = [luaptr](sol::variadic_args va) {
        if ((va.size() != 1) && (va.size() != 3))
        {
            conditionalExit(MessageType::LUA_ERROR, Message::UNEXPECTED_ARG_COUNT, "box", "1 or 3", va.size());
        }
        checkArgType("box", va, sol::type::number);
        Vector3 size = {va[0].as<float>(), va[0].as<float>(), va[0].as<float>()};
        if (va.size() == 3) { size = {va[0].as<float>(), va[1].as<float>(), va[2].as<float>()}; }
        if (!luaptr->canvas.noFill) { DrawCubeV(Vector3{0.0f, 0.0f, 0.0f}, size, luaptr->canvas.fill); }
        if (!luaptr->canvas.noStroke) { DrawCubeWiresV(Vector3{0.0f, 0.0f, 0.0f}, size, luaptr->canvas.stroke); }
    };

    lua["sphere"] = [luaptr](sol::variadic_args va) {
        checkArgSize("sphere", 1, va.size());
        checkArgType("sphere", va, sol::type::number);
        DrawSphere(Vector3{0.0f, 0.0f, 0.0f}, va[0].as<double>(), luaptr->canvas.fill);
    };
}
}
}