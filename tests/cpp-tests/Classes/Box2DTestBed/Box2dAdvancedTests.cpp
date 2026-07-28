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

#include "Box2dAdvancedTests.h"

#include "renderer/CCRenderer.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

USING_NS_CC;

namespace
{
constexpr float PixelsPerMeter = 15.0f;
constexpr float TimeStep = 1.0f / 60.0f;
constexpr int SubStepCount = 4;
}

Box2dAdvancedTest* Box2dAdvancedTest::create(Box2dAdvancedScenario scenario)
{
    auto test = new (std::nothrow) Box2dAdvancedTest(scenario);
    if (test && test->init())
    {
        test->autorelease();
        return test;
    }

    CC_SAFE_DELETE(test);
    return nullptr;
}

Box2dAdvancedTest::Box2dAdvancedTest(Box2dAdvancedScenario scenario)
    : _scenario(scenario)
    , _world(b2_nullWorldId)
    , _debugDraw(PixelsPerMeter)
    , _debugLabel(nullptr)
    , _infoLabel(nullptr)
    , _debugEnabled(true)
    , _cameraX(0.0f)
    , _car()
    , _carStart{0.0f, 1.0f}
    , _carScale(1.3f)
    , _carSpeed(0.0f)
    , _human{}
    , _ragdollTense(true)
    , _ragdollDirection(1.0f)
    , _donut()
    , _softBodyHertz(5.0f)
{
}

Box2dAdvancedTest::~Box2dAdvancedTest()
{
    if (B2_IS_NON_NULL(_world))
    {
        b2DestroyWorld(_world);
    }
}

bool Box2dAdvancedTest::init()
{
    if (!TestCase::init())
    {
        return false;
    }

    createWorld();
    createHeader();

    _infoLabel = Label::createWithTTF("", "fonts/arial.ttf", 12.0f);
    _infoLabel->setPosition(VisibleRect::center().x + 45.0f, VisibleRect::top().y - 88.0f);
    addChild(_infoLabel, 10000);

    createDebugButton();
    createControls();
    scheduleUpdate();
    return true;
}

std::string Box2dAdvancedTest::title() const
{
    switch (_scenario)
    {
    case Box2dAdvancedScenario::Car:
        return "Box2D 3.1.1 - Car";
    case Box2dAdvancedScenario::Ragdoll:
        return "Box2D 3.1.1 - Ragdoll";
    case Box2dAdvancedScenario::SoftBody:
        return "Box2D 3.1.1 - Soft Body";
    }
    return "Box2D 3.1.1";
}

std::string Box2dAdvancedTest::subtitle() const
{
    switch (_scenario)
    {
    case Box2dAdvancedScenario::Car:
        return "Wheel-joint suspension, motors, terrain, and a following camera";
    case Box2dAdvancedScenario::Ragdoll:
        return "Eleven bones joined with limits, friction motors, and springs";
    case Box2dAdvancedScenario::SoftBody:
        return "Seven capsules joined by soft angular weld constraints";
    }
    return "";
}

void Box2dAdvancedTest::createWorld()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -10.0f};
    _world = b2CreateWorld(&worldDef);

    switch (_scenario)
    {
    case Box2dAdvancedScenario::Car:
        createCar();
        break;
    case Box2dAdvancedScenario::Ragdoll:
        createRagdoll();
        break;
    case Box2dAdvancedScenario::SoftBody:
        createSoftBody();
        break;
    }
}

void Box2dAdvancedTest::createHeader()
{
    const Rect visibleRect = VisibleRect::getVisibleRect();
    auto header = LayerColor::create(Color4B::BLACK, visibleRect.size.width, 105.0f);
    header->setPosition(visibleRect.origin.x, visibleRect.getMaxY() - 105.0f);
    addChild(header, 9998);
}

void Box2dAdvancedTest::createDebugButton()
{
    _debugLabel = Label::createWithTTF("Debug: ON", "fonts/arial.ttf", 16.0f);
    auto toggle = MenuItemLabel::create(_debugLabel, CC_CALLBACK_1(Box2dAdvancedTest::toggleDebug, this));
    auto menu = Menu::create(toggle, nullptr);
    menu->setPosition(VisibleRect::rightTop().x - 60.0f, VisibleRect::rightTop().y - 87.0f);
    addChild(menu, 10000);
}

void Box2dAdvancedTest::createControls()
{
    const float y = VisibleRect::top().y - 87.0f;
    const float left = VisibleRect::left().x;

    switch (_scenario)
    {
    case Box2dAdvancedScenario::Car:
    {
        addControl("Left", {left + 27.0f, y}, [this](Ref*) { setCarSpeed(24.0f); });
        addControl("Brake", {left + 77.0f, y}, [this](Ref*) { setCarSpeed(0.0f); });
        addControl("Right", {left + 132.0f, y}, [this](Ref*) { setCarSpeed(-24.0f); });
        addControl("Reset", {left + 187.0f, y},
                   [this](Ref* sender) { resetCar(sender); });

        auto listener = EventListenerKeyboard::create();
        listener->onKeyPressed = [this](EventKeyboard::KeyCode key, Event*) {
            if (key == EventKeyboard::KeyCode::KEY_LEFT_ARROW || key == EventKeyboard::KeyCode::KEY_A)
            {
                setCarSpeed(24.0f);
            }
            else if (key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW || key == EventKeyboard::KeyCode::KEY_D)
            {
                setCarSpeed(-24.0f);
            }
            else if (key == EventKeyboard::KeyCode::KEY_SPACE || key == EventKeyboard::KeyCode::KEY_S)
            {
                setCarSpeed(0.0f);
            }
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
        break;
    }
    case Box2dAdvancedScenario::Ragdoll:
        addControl("Toss", {left + 28.0f, y},
                   [this](Ref* sender) { tossRagdoll(sender); });
        addControl("Relax / tense", {left + 98.0f, y},
                   [this](Ref* sender) { toggleRagdollJoints(sender); });
        addControl("Respawn", {left + 183.0f, y},
                   [this](Ref* sender) { resetRagdoll(sender); });
        break;
    case Box2dAdvancedScenario::SoftBody:
        addControl("Kick", {left + 28.0f, y},
                   [this](Ref* sender) { kickSoftBody(sender); });
        addControl("Soft / firm", {left + 92.0f, y},
                   [this](Ref* sender) { toggleSoftBody(sender); });
        addControl("Reset", {left + 163.0f, y},
                   [this](Ref* sender) { resetSoftBody(sender); });
        break;
    }
}

void Box2dAdvancedTest::addControl(const std::string& text, const Vec2& position,
                                   const std::function<void(Ref*)>& callback)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 13.0f);
    auto item = MenuItemLabel::create(label, callback);
    auto menu = Menu::create(item, nullptr);
    menu->setPosition(position);
    addChild(menu, 10000);
}

b2BodyId Box2dAdvancedTest::createGroundSegment(b2Vec2 point1, b2Vec2 point2, float friction)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = friction;
    shapeDef.material.customColor = b2_colorLightSteelBlue;
    const b2Segment segment = {point1, point2};
    b2CreateSegmentShape(ground, &shapeDef, &segment);
    return ground;
}

void Box2dAdvancedTest::createCar()
{
    b2BodyDef groundDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &groundDef);
    const b2Vec2 points[] = {
        {90.0f, 1.0f}, {75.0f, 1.0f}, {65.0f, 3.0f}, {55.0f, 0.0f},
        {45.0f, 2.0f}, {35.0f, 1.0f}, {25.0f, 4.0f}, {15.0f, 1.0f},
        {5.0f, 2.0f}, {-5.0f, 0.0f}, {-15.0f, 2.0f}, {-28.0f, 0.0f},
        {-40.0f, 0.0f}, {-40.0f, -5.0f}, {90.0f, -5.0f},
    };
    b2SurfaceMaterial material = b2DefaultSurfaceMaterial();
    material.friction = 0.9f;
    material.customColor = b2_colorLightSteelBlue;
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.points = points;
    chainDef.count = static_cast<int>(sizeof(points) / sizeof(points[0]));
    chainDef.materials = &material;
    chainDef.materialCount = 1;
    chainDef.isLoop = true;
    b2CreateChain(ground, &chainDef);

    b2ShapeDef boxShapeDef = b2DefaultShapeDef();
    boxShapeDef.density = 0.5f;
    boxShapeDef.material.friction = 0.5f;
    boxShapeDef.material.customColor = b2_colorOrange;
    const b2Polygon box = b2MakeRoundedBox(0.65f, 0.65f, 0.08f);
    for (int i = 0; i < 3; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {8.0f + static_cast<float>(i) * 1.4f, 5.0f + static_cast<float>(i)};
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2CreatePolygonShape(body, &boxShapeDef, &box);
    }

    _car.Spawn(_world, _carStart, _carScale, 5.0f, 0.7f, 8.0f, nullptr);
    _cameraX = _carStart.x;
}

void Box2dAdvancedTest::setCarSpeed(float speed)
{
    _carSpeed = speed;
    if (b2Joint_IsValid(_car.m_rearAxleId) && b2Joint_IsValid(_car.m_frontAxleId))
    {
        _car.SetSpeed(speed);
    }
}

void Box2dAdvancedTest::resetCar(Ref*)
{
    setCarSpeed(0.0f);

    const b2Vec2 chassisPosition = b2Add(_carStart, {0.0f, 1.0f * _carScale});
    const b2Vec2 rearPosition = b2Add(_carStart, {-1.0f * _carScale, 0.35f * _carScale});
    const b2Vec2 frontPosition = b2Add(_carStart, {1.0f * _carScale, 0.4f * _carScale});
    const b2BodyId bodies[] = {_car.m_chassisId, _car.m_rearWheelId, _car.m_frontWheelId};
    const b2Vec2 positions[] = {chassisPosition, rearPosition, frontPosition};
    for (int i = 0; i < 3; ++i)
    {
        if (b2Body_IsValid(bodies[i]))
        {
            b2Body_SetTransform(bodies[i], positions[i], b2Rot_identity);
            b2Body_SetLinearVelocity(bodies[i], b2Vec2_zero);
            b2Body_SetAngularVelocity(bodies[i], 0.0f);
            b2Body_SetAwake(bodies[i], true);
        }
    }
    _cameraX = _carStart.x;
}

void Box2dAdvancedTest::createRagdoll()
{
    createGroundSegment({-18.0f, 0.0f}, {18.0f, 0.0f});
    createGroundSegment({-9.0f, 4.0f}, {0.0f, 2.0f});
    createGroundSegment({0.0f, 2.0f}, {9.0f, 4.0f});
    b2World_SetContactTuning(_world, 240.0f, 0.0f, 2.0f);
    resetRagdoll();
}

void Box2dAdvancedTest::resetRagdoll(Ref*)
{
    if (_human.isSpawned)
    {
        DestroyHuman(&_human);
    }

    _human = {};
    _ragdollTense = true;
    CreateHuman(&_human, _world, {0.0f, 8.0f}, 3.5f, 0.08f, 4.0f, 0.5f,
                1, nullptr, true);
}

void Box2dAdvancedTest::tossRagdoll(Ref*)
{
    if (!_human.isSpawned)
    {
        return;
    }

    const b2BodyId torso = _human.bones[bone_torso].bodyId;
    b2Body_ApplyLinearImpulseToCenter(torso, {_ragdollDirection * 20.0f, 8.0f}, true);
    b2Body_ApplyAngularImpulse(torso, _ragdollDirection * 3.0f, true);
    _ragdollDirection = -_ragdollDirection;
}

void Box2dAdvancedTest::toggleRagdollJoints(Ref*)
{
    if (!_human.isSpawned)
    {
        return;
    }

    _ragdollTense = !_ragdollTense;
    Human_SetJointFrictionTorque(&_human, _ragdollTense ? 0.08f : 0.0f);
    Human_SetJointSpringHertz(&_human, _ragdollTense ? 4.0f : 0.0f);
    Human_SetJointDampingRatio(&_human, _ragdollTense ? 0.5f : 0.0f);
}

void Box2dAdvancedTest::createSoftBody()
{
    createGroundSegment({-18.0f, 0.0f}, {18.0f, 0.0f});
    b2BodyDef obstacleDef = b2DefaultBodyDef();
    obstacleDef.position = {0.0f, 2.0f};
    b2BodyId obstacle = b2CreateBody(_world, &obstacleDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = 0.7f;
    shapeDef.material.customColor = b2_colorLightSteelBlue;
    const b2Polygon block = b2MakeRoundedBox(2.0f, 0.5f, 0.2f);
    b2CreatePolygonShape(obstacle, &shapeDef, &block);
    resetSoftBody();
}

void Box2dAdvancedTest::resetSoftBody(Ref*)
{
    if (_donut.m_isSpawned)
    {
        _donut.Destroy();
    }

    _softBodyHertz = 5.0f;
    _donut.Create(_world, {0.0f, 10.0f}, 2.2f, 1, false, nullptr);
    for (int i = 0; i < Donut::m_sides; ++i)
    {
        b2ShapeId shapeId;
        if (b2Body_GetShapes(_donut.m_bodyIds[i], &shapeId, 1) == 1)
        {
            b2SurfaceMaterial material = b2Shape_GetSurfaceMaterial(shapeId);
            material.customColor = i % 2 == 0 ? b2_colorCyan : b2_colorOrange;
            b2Shape_SetSurfaceMaterial(shapeId, material);
        }
    }
}

void Box2dAdvancedTest::kickSoftBody(Ref*)
{
    if (!_donut.m_isSpawned)
    {
        return;
    }

    b2Vec2 center = b2Vec2_zero;
    for (b2BodyId body : _donut.m_bodyIds)
    {
        center = b2Add(center, b2Body_GetPosition(body));
    }
    center = b2MulSV(1.0f / static_cast<float>(Donut::m_sides), center);

    for (int i = 0; i < Donut::m_sides; ++i)
    {
        const b2BodyId body = _donut.m_bodyIds[i];
        b2Vec2 direction = b2Normalize(b2Sub(b2Body_GetPosition(body), center));
        b2Vec2 impulse = b2MulSV(3.0f, direction);
        impulse.y += 5.0f;
        b2Body_ApplyLinearImpulseToCenter(body, impulse, true);
        b2Body_ApplyAngularImpulse(body, i % 2 == 0 ? 0.8f : -0.8f, true);
    }
}

void Box2dAdvancedTest::toggleSoftBody(Ref*)
{
    if (!_donut.m_isSpawned)
    {
        return;
    }

    _softBodyHertz = _softBodyHertz > 3.0f ? 1.0f : 8.0f;
    for (b2JointId joint : _donut.m_jointIds)
    {
        if (b2Joint_IsValid(joint))
        {
            b2WeldJoint_SetAngularHertz(joint, _softBodyHertz);
            b2WeldJoint_SetAngularDampingRatio(joint, 0.3f);
            b2Joint_WakeBodies(joint);
        }
    }
}

void Box2dAdvancedTest::update(float dt)
{
    b2World_Step(_world, TimeStep, SubStepCount);

    switch (_scenario)
    {
    case Box2dAdvancedScenario::Car:
        updateCar(dt);
        break;
    case Box2dAdvancedScenario::Ragdoll:
        updateRagdoll();
        break;
    case Box2dAdvancedScenario::SoftBody:
        updateSoftBody();
        break;
    }
}

void Box2dAdvancedTest::updateCar(float dt)
{
    if (!b2Body_IsValid(_car.m_chassisId))
    {
        return;
    }

    const b2Vec2 position = b2Body_GetPosition(_car.m_chassisId);
    const b2Vec2 velocity = b2Body_GetLinearVelocity(_car.m_chassisId);
    const float follow = std::min(1.0f, 4.0f * dt);
    _cameraX += follow * (position.x - _cameraX);
    _infoLabel->setString(StringUtils::format("x %.1f | %+.1f km/h | motor %+.0f",
                                               position.x, velocity.x * 3.6f, _carSpeed));

    if (position.y < -2.0f || position.x > 87.0f || position.x < -38.0f)
    {
        resetCar();
    }
}

void Box2dAdvancedTest::updateRagdoll()
{
    if (!_human.isSpawned)
    {
        return;
    }

    const float hipY = b2Body_GetPosition(_human.bones[bone_hip].bodyId).y;
    _infoLabel->setString(StringUtils::format("%s | hip %.1f | 11 / 10",
                                               _ragdollTense ? "tense" : "relaxed", hipY));
}

void Box2dAdvancedTest::updateSoftBody()
{
    if (!_donut.m_isSpawned)
    {
        return;
    }

    b2Vec2 center = b2Vec2_zero;
    for (b2BodyId body : _donut.m_bodyIds)
    {
        center = b2Add(center, b2Body_GetPosition(body));
    }
    center = b2MulSV(1.0f / static_cast<float>(Donut::m_sides), center);

    float minimumRadius = FLT_MAX;
    float maximumRadius = 0.0f;
    for (b2BodyId body : _donut.m_bodyIds)
    {
        const float radius = b2Distance(center, b2Body_GetPosition(body));
        minimumRadius = std::min(minimumRadius, radius);
        maximumRadius = std::max(maximumRadius, radius);
    }
    _infoLabel->setString(StringUtils::format("%.0f Hz | center y %.1f | flex %.2f",
                                               _softBodyHertz, center.y,
                                               maximumRadius - minimumRadius));

    if (center.y < -2.0f)
    {
        resetSoftBody();
    }
}

void Box2dAdvancedTest::toggleDebug(Ref*)
{
    _debugEnabled = !_debugEnabled;
    _debugDraw.setEnabled(_debugEnabled);
    _debugLabel->setString(_debugEnabled ? "Debug: ON" : "Debug: OFF");
}

void Box2dAdvancedTest::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    Scene::draw(renderer, transform, flags);
    if (!_debugEnabled)
    {
        return;
    }

    _debugCommand.init(_globalZOrder, transform, flags);
    _debugCommand.func = [this, transform]() { onDebugDraw(transform); };
    renderer->addCommand(&_debugCommand);
}

void Box2dAdvancedTest::onDebugDraw(Mat4 transform)
{
    transform.translate(VisibleRect::center().x - _cameraX * PixelsPerMeter,
                        VisibleRect::bottom().y + 55.0f, 0.0f);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    b2World_Draw(_world, _debugDraw.getDebugDraw());
    CHECK_GL_ERROR_DEBUG();
    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
