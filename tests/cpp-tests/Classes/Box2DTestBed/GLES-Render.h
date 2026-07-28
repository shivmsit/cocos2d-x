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
 */

#ifndef RENDER_H
#define RENDER_H

#include "box2d/box2d.h"
#include "cocos2d.h"

// Cocos OpenGL renderer for Box2D 3.x's callback-based debug draw API.
class GLESDebugDraw
{
public:
    explicit GLESDebugDraw(float ratio = 1.0f);

    b2DebugDraw* getDebugDraw() { return &_debugDraw; }
    void setEnabled(bool enabled)
    {
        _debugDraw.drawShapes = enabled;
        _debugDraw.drawJoints = enabled;
    }

private:
    static void DrawPolygonCallback(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context);
    static void DrawSolidPolygonCallback(b2Transform transform, const b2Vec2* vertices, int vertexCount,
                                         float radius, b2HexColor color, void* context);
    static void DrawCircleCallback(b2Vec2 center, float radius, b2HexColor color, void* context);
    static void DrawSolidCircleCallback(b2Transform transform, float radius, b2HexColor color, void* context);
    static void DrawSolidCapsuleCallback(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context);
    static void DrawSegmentCallback(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context);
    static void DrawTransformCallback(b2Transform transform, void* context);
    static void DrawPointCallback(b2Vec2 point, float size, b2HexColor color, void* context);
    static void DrawStringCallback(b2Vec2 point, const char* string, b2HexColor color, void* context);

    void drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, bool solid);
    void drawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount,
                          float radius, b2HexColor color);
    void drawCircle(b2Vec2 center, float radius, b2HexColor color, bool solid);
    void drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color);
    void drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color);
    void drawPoint(b2Vec2 point, float size, b2HexColor color);
    void setColor(b2HexColor color, float alpha = 1.0f, float scale = 1.0f);
    void prepare();

    float _ratio;
    cocos2d::GLProgram* _shaderProgram;
    GLint _colorLocation;
    b2DebugDraw _debugDraw;
};

#endif
