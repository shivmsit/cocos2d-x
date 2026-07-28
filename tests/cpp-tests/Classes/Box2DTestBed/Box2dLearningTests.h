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

#ifndef _BOX2D_LEARNING_TESTS_H_
#define _BOX2D_LEARNING_TESTS_H_

#include "../BaseTest.h"
#include "GLES-Render.h"
#include "renderer/CCCustomCommand.h"

#include <array>

enum class Box2dLearningScenario
{
    ContinuousCollision,
    Casts,
    SensorEvents,
    JointsGallery,
    OneWayPlatform,
    CollisionFiltering,
    CharacterPlatform,
};

class Box2dLearningTest : public TestCase
{
public:
    static Box2dLearningTest* create(Box2dLearningScenario scenario);

    explicit Box2dLearningTest(Box2dLearningScenario scenario);
    ~Box2dLearningTest() override;

    bool init() override;
    std::string title() const override;
    std::string subtitle() const override;
    void update(float dt) override;
    void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;

private:
    void createWorld();
    void createContinuousCollision();
    void createCasts();
    void createSensorEvents();
    void createJointsGallery();
    void createOneWayPlatform();
    void createCollisionFiltering();
    void createCharacterPlatform();

    void createHeader();
    void createDebugButton();
    void createScenarioControls();
    void addControl(const std::string& text, const cocos2d::Vec2& position,
                    const std::function<void(cocos2d::Ref*)>& callback);
    void addWorldLabel(const std::string& text, b2Vec2 position);
    void createGroundSegment(b2Vec2 point1, b2Vec2 point2, float friction = 0.6f);

    void toggleDebug(cocos2d::Ref*);
    void toggleContinuous(cocos2d::Ref*);
    void toggleCastMode(cocos2d::Ref*);
    void fireProjectiles(cocos2d::Ref* = nullptr);
    void launchOneWayPlayer(cocos2d::Ref* = nullptr);
    void moveCharacter(float horizontalImpulse, float verticalImpulse);

    void updateContinuous(float dt);
    void updateCasts();
    void updateSensors();
    void updateOneWayPlatform(float dt);
    void updateCharacterPlatform();

    void installCastTouchListener();
    cocos2d::Vec2 worldToScreen(b2Vec2 position) const;
    b2Vec2 screenToWorld(const cocos2d::Vec2& position) const;
    void onDebugDraw(cocos2d::Mat4 transform);

    static bool oneWayPreSolve(b2ShapeId shapeIdA, b2ShapeId shapeIdB,
                               b2Manifold* manifold, void* context);
    bool shouldEnableOneWayContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB,
                                   const b2Manifold& manifold) const;

    Box2dLearningScenario _scenario;
    b2WorldId _world;
    GLESDebugDraw _debugDraw;
    cocos2d::Label* _debugLabel;
    cocos2d::Label* _infoLabel;
    cocos2d::Label* _continuousLabel;
    cocos2d::Label* _castModeLabel;
    cocos2d::DrawNode* _overlay;
    cocos2d::CustomCommand _debugCommand;
    bool _debugEnabled;
    bool _continuousEnabled;
    bool _useShapeCast;
    float _elapsed;

    std::array<b2BodyId, 4> _continuousBodies;
    std::array<b2BodyId, 3> _sensorVisitors;
    int _sensorBeginCount;
    int _sensorEndCount;
    int _sensorInsideCount;

    b2Vec2 _castOrigin;
    b2Vec2 _castTarget;

    b2BodyId _oneWayPlayer;
    b2ShapeId _oneWayPlayerShape;
    float _oneWayRadius;

    b2BodyId _movingPlatform;
    b2BodyId _character;
};

#endif
