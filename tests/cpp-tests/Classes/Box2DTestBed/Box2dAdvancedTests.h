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

#ifndef _BOX2D_ADVANCED_TESTS_H_
#define _BOX2D_ADVANCED_TESTS_H_

#include "../BaseTest.h"
#include "GLES-Render.h"
#include "renderer/CCCustomCommand.h"

#include "car.h"
#include "donut.h"
#include "human.h"

#include <functional>

enum class Box2dAdvancedScenario
{
    Car,
    Ragdoll,
    SoftBody,
};

class Box2dAdvancedTest : public TestCase
{
public:
    static Box2dAdvancedTest* create(Box2dAdvancedScenario scenario);

    explicit Box2dAdvancedTest(Box2dAdvancedScenario scenario);
    ~Box2dAdvancedTest() override;

    bool init() override;
    std::string title() const override;
    std::string subtitle() const override;
    void update(float dt) override;
    void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;

private:
    void createWorld();
    void createHeader();
    void createControls();
    void createDebugButton();
    void addControl(const std::string& text, const cocos2d::Vec2& position,
                    const std::function<void(cocos2d::Ref*)>& callback);
    b2BodyId createGroundSegment(b2Vec2 point1, b2Vec2 point2, float friction = 0.7f);

    void createCar();
    void createRagdoll();
    void createSoftBody();

    void setCarSpeed(float speed);
    void resetCar(cocos2d::Ref* = nullptr);
    void tossRagdoll(cocos2d::Ref* = nullptr);
    void toggleRagdollJoints(cocos2d::Ref* = nullptr);
    void resetRagdoll(cocos2d::Ref* = nullptr);
    void kickSoftBody(cocos2d::Ref* = nullptr);
    void toggleSoftBody(cocos2d::Ref* = nullptr);
    void resetSoftBody(cocos2d::Ref* = nullptr);

    void updateCar(float dt);
    void updateRagdoll();
    void updateSoftBody();
    void toggleDebug(cocos2d::Ref*);
    void onDebugDraw(cocos2d::Mat4 transform);

    Box2dAdvancedScenario _scenario;
    b2WorldId _world;
    GLESDebugDraw _debugDraw;
    cocos2d::Label* _debugLabel;
    cocos2d::Label* _infoLabel;
    cocos2d::CustomCommand _debugCommand;
    bool _debugEnabled;
    float _cameraX;

    Car _car;
    b2Vec2 _carStart;
    float _carScale;
    float _carSpeed;

    Human _human;
    bool _ragdollTense;
    float _ragdollDirection;

    Donut _donut;
    float _softBodyHertz;
};

#endif
