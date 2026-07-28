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

#ifndef _BOX2D_GAMEPLAY_TESTS_H_
#define _BOX2D_GAMEPLAY_TESTS_H_

#include "../BaseTest.h"
#include "GLES-Render.h"
#include "renderer/CCCustomCommand.h"

#include <array>

enum class Box2dGameplayScenario
{
    ContactAndMoveEvents,
    ChainTerrain,
    ConveyorBelt,
    BreakableJoint,
    Explosion,
};

class Box2dGameplayTest : public TestCase
{
public:
    static Box2dGameplayTest* create(Box2dGameplayScenario scenario);

    explicit Box2dGameplayTest(Box2dGameplayScenario scenario);
    ~Box2dGameplayTest() override;

    bool init() override;
    std::string title() const override;
    std::string subtitle() const override;
    void update(float dt) override;
    void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;

private:
    void createWorld();
    void createHeader();
    void createDebugButton();
    void createScenarioControls();
    void addControl(const std::string& text, const cocos2d::Vec2& position,
                    const std::function<void(cocos2d::Ref*)>& callback);
    void addWorldLabel(const std::string& text, b2Vec2 position);
    b2BodyId createGroundSegment(b2Vec2 point1, b2Vec2 point2);

    void createContactAndMoveEvents();
    void createChainTerrain();
    void createConveyorBelt();
    void createBreakableJoint();
    void createExplosion();

    void resetEventBodies(cocos2d::Ref* = nullptr);
    void launchChainRunner(cocos2d::Ref* = nullptr);
    void resetConveyorBodies(cocos2d::Ref* = nullptr);
    void reverseConveyor(cocos2d::Ref* = nullptr);
    void resetBreakableChain(cocos2d::Ref* = nullptr);
    void stressBreakableChain(cocos2d::Ref* = nullptr);
    void resetExplosionBodies(cocos2d::Ref* = nullptr);
    void explode(float impulse);

    void toggleDebug(cocos2d::Ref*);
    void updateContactAndMoveEvents(float dt);
    void updateChainTerrain();
    void updateConveyorBelt();
    void updateBreakableJoint();
    void updateExplosion();

    cocos2d::Vec2 worldToScreen(b2Vec2 position) const;
    void onDebugDraw(cocos2d::Mat4 transform);

    Box2dGameplayScenario _scenario;
    b2WorldId _world;
    GLESDebugDraw _debugDraw;
    cocos2d::Label* _debugLabel;
    cocos2d::Label* _infoLabel;
    cocos2d::DrawNode* _overlay;
    cocos2d::CustomCommand _debugCommand;
    bool _debugEnabled;

    std::array<b2BodyId, 8> _eventBodies;
    int _contactBeginCount;
    int _contactEndCount;
    int _contactHitCount;
    int _bodyMoveCount;
    int _fellAsleepCount;
    b2Vec2 _lastHitPoint;
    b2Vec2 _lastHitNormal;
    float _lastHitSpeed;
    float _hitMarkerTime;

    b2ChainId _terrainChain;
    b2BodyId _chainRunner;

    b2ShapeId _conveyorShape;
    std::array<b2BodyId, 6> _conveyorBodies;
    float _conveyorSpeed;

    b2BodyId _breakAnchor;
    std::array<b2BodyId, 5> _breakBodies;
    std::array<b2JointId, 5> _breakJoints;
    float _breakForce;
    bool _breakPending;

    std::array<b2BodyId, 18> _explosionBodies;
    float _explosionRadius;
    float _explosionFalloff;
    float _lastExplosionImpulse;
};

#endif
