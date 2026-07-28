/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef _BOX2D_TEST_H_
#define _BOX2D_TEST_H_

#include "cocos2d.h"
#include "box2d/box2d.h"
#include "../BaseTest.h"
#include "../Box2DTestBed/GLES-Render.h"

DEFINE_TEST_SUITE(Box2DTests);

class Box2DTest : public TestCase
{
public:
    CREATE_FUNC(Box2DTest);

    Box2DTest();
    ~Box2DTest() override;

    bool init() override;
    void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    void update(float dt) override;

private:
    void initPhysics();
    void createResetButton();
    void createDebugButton();
    void toggleDebugCallback(cocos2d::Ref* sender);
    void addNewSpriteAtPosition(cocos2d::Vec2 position);
    void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
    void onDebugDraw(const cocos2d::Mat4& transform);

    cocos2d::Texture2D* _spriteTexture;
    cocos2d::Label* _debugLabel;
    cocos2d::CustomCommand _debugCommand;
    GLESDebugDraw _debugDraw;
    bool _debugEnabled;
    b2WorldId _world;
};

#endif
