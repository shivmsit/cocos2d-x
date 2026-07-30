/*
 * Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
 *
 * iPhone port by Simon Oliver - http://www.simonoliver.com - http://www.handcircus.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Box2D 3.x callback adaptation of the Cocos2d-x GLES testbed renderer.
 */

#include "GLES-Render.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>

USING_NS_CC;

namespace
{
constexpr int CircleSegments = 24;
constexpr int CornerSegments = 4;
constexpr int MaxDebugVertices = B2_MAX_POLYGON_VERTICES * (CornerSegments + 1);
}

GLESDebugDraw::GLESDebugDraw(float ratio)
    : _ratio(ratio)
    , _drawNode(nullptr)
    , _debugDraw(b2DefaultDebugDraw())
    , _enabled(true)
{
    _debugDraw.context = this;
    _debugDraw.DrawPolygonFcn = DrawPolygonCallback;
    _debugDraw.DrawSolidPolygonFcn = DrawSolidPolygonCallback;
    _debugDraw.DrawCircleFcn = DrawCircleCallback;
    _debugDraw.DrawSolidCircleFcn = DrawSolidCircleCallback;
    _debugDraw.DrawSolidCapsuleFcn = DrawSolidCapsuleCallback;
    _debugDraw.DrawSegmentFcn = DrawSegmentCallback;
    _debugDraw.DrawTransformFcn = DrawTransformCallback;
    _debugDraw.DrawPointFcn = DrawPointCallback;
    _debugDraw.DrawStringFcn = DrawStringCallback;
    _debugDraw.drawShapes = true;
    _debugDraw.drawJoints = true;
}

void GLESDebugDraw::drawWorld(b2WorldId world)
{
    if (!_drawNode)
    {
        return;
    }

    _drawNode->clear();
    if (_enabled)
    {
        b2World_Draw(world, &_debugDraw);
    }
}

Color4F GLESDebugDraw::makeColor(b2HexColor color, float alpha, float scale) const
{
    const uint32_t value = static_cast<uint32_t>(color);
    const float red = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
    const float green = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
    const float blue = static_cast<float>(value & 0xFF) / 255.0f;
    return {
        std::min(red * scale, 1.0f),
        std::min(green * scale, 1.0f),
        std::min(blue * scale, 1.0f),
        alpha,
    };
}

Vec2 GLESDebugDraw::makePoint(b2Vec2 point) const
{
    return {point.x * _ratio, point.y * _ratio};
}

void GLESDebugDraw::drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, bool solid)
{
    CCASSERT(vertexCount <= MaxDebugVertices, "Box2D debug polygon exceeds renderer capacity");
    std::array<Vec2, MaxDebugVertices> points;
    for (int i = 0; i < vertexCount; ++i)
    {
        points[i] = makePoint(vertices[i]);
    }

    const Color4F fill = solid ? makeColor(color, 0.35f, 0.5f) : Color4F(0.0f, 0.0f, 0.0f, 0.0f);
    _drawNode->drawPolygon(points.data(), vertexCount, fill, 2.0f, makeColor(color, 1.0f, 1.35f));
}

void GLESDebugDraw::drawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount,
                                    float radius, b2HexColor color)
{
    std::array<b2Vec2, MaxDebugVertices> transformed;
    int transformedCount = 0;

    if (radius <= 0.0f)
    {
        for (int i = 0; i < vertexCount; ++i)
        {
            transformed[transformedCount++] = b2TransformPoint(transform, vertices[i]);
        }
        drawPolygon(transformed.data(), transformedCount, color, true);
        return;
    }

    for (int i = 0; i < vertexCount; ++i)
    {
        const b2Vec2 previous = vertices[(i + vertexCount - 1) % vertexCount];
        const b2Vec2 current = vertices[i];
        const b2Vec2 next = vertices[(i + 1) % vertexCount];
        const b2Vec2 previousEdge = b2Normalize(b2Sub(current, previous));
        const b2Vec2 nextEdge = b2Normalize(b2Sub(next, current));
        const b2Vec2 previousNormal = {previousEdge.y, -previousEdge.x};
        const b2Vec2 nextNormal = {nextEdge.y, -nextEdge.x};

        float startAngle = std::atan2(previousNormal.y, previousNormal.x);
        float endAngle = std::atan2(nextNormal.y, nextNormal.x);
        while (endAngle < startAngle)
        {
            endAngle += 2.0f * B2_PI;
        }

        for (int segment = 0; segment <= CornerSegments; ++segment)
        {
            const float fraction = static_cast<float>(segment) / static_cast<float>(CornerSegments);
            const float angle = startAngle + fraction * (endAngle - startAngle);
            const b2Vec2 roundedVertex = {
                current.x + radius * std::cos(angle),
                current.y + radius * std::sin(angle),
            };
            transformed[transformedCount++] = b2TransformPoint(transform, roundedVertex);
        }
    }
    drawPolygon(transformed.data(), transformedCount, color, true);
}

void GLESDebugDraw::drawCircle(b2Vec2 center, float radius, b2HexColor color, bool solid)
{
    b2Vec2 vertices[CircleSegments];
    for (int i = 0; i < CircleSegments; ++i)
    {
        const float angle = 2.0f * B2_PI * static_cast<float>(i) / static_cast<float>(CircleSegments);
        vertices[i] = {center.x + radius * std::cos(angle), center.y + radius * std::sin(angle)};
    }
    drawPolygon(vertices, CircleSegments, color, solid);
}

void GLESDebugDraw::drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color)
{
    const b2Vec2 delta = b2Sub(p2, p1);
    const float lengthSquared = b2LengthSquared(delta);
    if (lengthSquared <= FLT_EPSILON)
    {
        drawCircle(p1, radius, color, true);
        return;
    }

    const b2Vec2 axis = b2MulSV(1.0f / std::sqrt(lengthSquared), delta);
    const b2Vec2 normal = {-axis.y, axis.x};
    const float normalAngle = std::atan2(normal.y, normal.x);

    constexpr int CapSegments = CircleSegments / 2;
    std::array<b2Vec2, 2 * (CapSegments + 1)> vertices;
    int vertexCount = 0;

    // Counter-clockwise outline: lower side, far cap, upper side, near cap.
    for (int segment = 0; segment <= CapSegments; ++segment)
    {
        const float angle = normalAngle - B2_PI +
                            B2_PI * static_cast<float>(segment) / static_cast<float>(CapSegments);
        vertices[vertexCount++] = {p2.x + radius * std::cos(angle), p2.y + radius * std::sin(angle)};
    }
    for (int segment = 0; segment <= CapSegments; ++segment)
    {
        const float angle = normalAngle +
                            B2_PI * static_cast<float>(segment) / static_cast<float>(CapSegments);
        vertices[vertexCount++] = {p1.x + radius * std::cos(angle), p1.y + radius * std::sin(angle)};
    }
    drawPolygon(vertices.data(), vertexCount, color, true);
}

void GLESDebugDraw::drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color)
{
    _drawNode->drawSegment(makePoint(p1), makePoint(p2), 1.0f, makeColor(color, 1.0f, 1.35f));
}

void GLESDebugDraw::drawPoint(b2Vec2 point, float size, b2HexColor color)
{
    _drawNode->drawDot(makePoint(point), std::max(1.0f, size * 0.5f), makeColor(color));
}

void GLESDebugDraw::DrawPolygonCallback(const b2Vec2* v, int n, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawPolygon(v, n, c, false);
}

void GLESDebugDraw::DrawSolidPolygonCallback(b2Transform t, const b2Vec2* v, int n, float radius,
                                             b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawSolidPolygon(t, v, n, radius, c);
}

void GLESDebugDraw::DrawCircleCallback(b2Vec2 p, float r, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawCircle(p, r, c, false);
}

void GLESDebugDraw::DrawSolidCircleCallback(b2Transform t, float r, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawCircle(t.p, r, c, true);
    static_cast<GLESDebugDraw*>(x)->drawSegment(t.p, b2MulAdd(t.p, r, b2Rot_GetXAxis(t.q)), c);
}

void GLESDebugDraw::DrawSolidCapsuleCallback(b2Vec2 p1, b2Vec2 p2, float r, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawCapsule(p1, p2, r, c);
}

void GLESDebugDraw::DrawSegmentCallback(b2Vec2 p1, b2Vec2 p2, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawSegment(p1, p2, c);
}

void GLESDebugDraw::DrawTransformCallback(b2Transform t, void* x)
{
    auto* draw = static_cast<GLESDebugDraw*>(x);
    draw->drawSegment(t.p, b2MulAdd(t.p, 0.4f, b2Rot_GetXAxis(t.q)), b2_colorRed);
    draw->drawSegment(t.p, b2MulAdd(t.p, 0.4f, b2Rot_GetYAxis(t.q)), b2_colorGreen);
}

void GLESDebugDraw::DrawPointCallback(b2Vec2 p, float size, b2HexColor c, void* x)
{
    static_cast<GLESDebugDraw*>(x)->drawPoint(p, size, c);
}

void GLESDebugDraw::DrawStringCallback(b2Vec2, const char*, b2HexColor, void*)
{
}
