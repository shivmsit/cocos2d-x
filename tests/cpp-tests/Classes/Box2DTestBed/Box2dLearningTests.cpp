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

#include "Box2dLearningTests.h"

#include "renderer/CCRenderer.h"

#include <algorithm>

USING_NS_CC;

namespace
{
constexpr float PixelsPerMeter = 15.0f;
constexpr float TimeStep = 1.0f / 60.0f;
constexpr int SubStepCount = 4;

Color4F colorForHex(b2HexColor color, float alpha = 1.0f)
{
    const uint32_t value = static_cast<uint32_t>(color);
    return {
        static_cast<float>((value >> 16) & 0xFF) / 255.0f,
        static_cast<float>((value >> 8) & 0xFF) / 255.0f,
        static_cast<float>(value & 0xFF) / 255.0f,
        alpha,
    };
}
}

Box2dLearningTest* Box2dLearningTest::create(Box2dLearningScenario scenario)
{
    auto test = new (std::nothrow) Box2dLearningTest(scenario);
    if (test && test->init())
    {
        test->autorelease();
        return test;
    }

    CC_SAFE_DELETE(test);
    return nullptr;
}

Box2dLearningTest::Box2dLearningTest(Box2dLearningScenario scenario)
    : _scenario(scenario)
    , _world(b2_nullWorldId)
    , _debugDraw(PixelsPerMeter)
    , _debugLabel(nullptr)
    , _infoLabel(nullptr)
    , _continuousLabel(nullptr)
    , _castModeLabel(nullptr)
    , _overlay(nullptr)
    , _debugEnabled(true)
    , _continuousEnabled(true)
    , _useShapeCast(false)
    , _elapsed(0.0f)
    , _continuousBodies{b2_nullBodyId, b2_nullBodyId, b2_nullBodyId, b2_nullBodyId}
    , _sensorVisitors{b2_nullBodyId, b2_nullBodyId, b2_nullBodyId}
    , _sensorBeginCount(0)
    , _sensorEndCount(0)
    , _sensorInsideCount(0)
    , _castOrigin{-13.0f, 8.0f}
    , _castTarget{13.0f, 10.0f}
    , _oneWayPlayer(b2_nullBodyId)
    , _oneWayPlayerShape(b2_nullShapeId)
    , _oneWayRadius(0.4f)
    , _movingPlatform(b2_nullBodyId)
    , _character(b2_nullBodyId)
{
}

Box2dLearningTest::~Box2dLearningTest()
{
    if (B2_IS_NON_NULL(_world))
    {
        b2DestroyWorld(_world);
    }
}

bool Box2dLearningTest::init()
{
    if (!TestCase::init())
    {
        return false;
    }

    createWorld();
    createHeader();

    _overlay = DrawNode::create();
    addChild(_overlay, 9997);

    _infoLabel = Label::createWithTTF("", "fonts/arial.ttf", 13.0f);
    _infoLabel->setPosition(VisibleRect::center().x, VisibleRect::top().y - 88.0f);
    addChild(_infoLabel, 10000);

    createDebugButton();
    createScenarioControls();
    if (_scenario == Box2dLearningScenario::Casts)
    {
        installCastTouchListener();
    }

    scheduleUpdate();
    return true;
}

std::string Box2dLearningTest::title() const
{
    switch (_scenario)
    {
    case Box2dLearningScenario::ContinuousCollision:
        return "Box2D 3.1.1 - Continuous Collision";
    case Box2dLearningScenario::Casts:
        return "Box2D 3.1.1 - Ray and Shape Cast";
    case Box2dLearningScenario::SensorEvents:
        return "Box2D 3.1.1 - Sensor Events";
    case Box2dLearningScenario::JointsGallery:
        return "Box2D 3.1.1 - Joints Gallery";
    case Box2dLearningScenario::OneWayPlatform:
        return "Box2D 3.1.1 - One-Way Platform";
    case Box2dLearningScenario::CollisionFiltering:
        return "Box2D 3.1.1 - Collision Filtering";
    case Box2dLearningScenario::CharacterPlatform:
        return "Box2D 3.1.1 - Character and Platform";
    }
    return "Box2D 3.1.1";
}

std::string Box2dLearningTest::subtitle() const
{
    switch (_scenario)
    {
    case Box2dLearningScenario::ContinuousCollision:
        return "Top: ordinary dynamic body. Bottom: bullet body. Toggle CCD.";
    case Box2dLearningScenario::Casts:
        return "Drag in the scene to aim; switch between a ray and a swept circle";
    case Box2dLearningScenario::SensorEvents:
        return "Three kinematic visitors generate begin/end overlap events";
    case Box2dLearningScenario::JointsGallery:
        return "Revolute, distance, prismatic, wheel, weld and motor joints";
    case Box2dLearningScenario::OneWayPlatform:
        return "Passes upward; lands only while moving downward";
    case Box2dLearningScenario::CollisionFiltering:
        return "Left collides; center mask and right group -1 pass through";
    case Box2dLearningScenario::CharacterPlatform:
        return "Fixed-rotation capsule on a moving kinematic platform";
    }
    return "";
}

void Box2dLearningTest::createWorld()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    if (_scenario == Box2dLearningScenario::ContinuousCollision ||
        _scenario == Box2dLearningScenario::Casts ||
        _scenario == Box2dLearningScenario::SensorEvents)
    {
        worldDef.gravity = {0.0f, 0.0f};
    }
    else
    {
        worldDef.gravity = {0.0f, -10.0f};
    }

    _world = b2CreateWorld(&worldDef);
    b2World_EnableSleeping(_world, true);
    b2World_EnableContinuous(_world, true);

    switch (_scenario)
    {
    case Box2dLearningScenario::ContinuousCollision:
        createContinuousCollision();
        break;
    case Box2dLearningScenario::Casts:
        createCasts();
        break;
    case Box2dLearningScenario::SensorEvents:
        createSensorEvents();
        break;
    case Box2dLearningScenario::JointsGallery:
        createJointsGallery();
        break;
    case Box2dLearningScenario::OneWayPlatform:
        createOneWayPlatform();
        break;
    case Box2dLearningScenario::CollisionFiltering:
        createCollisionFiltering();
        break;
    case Box2dLearningScenario::CharacterPlatform:
        createCharacterPlatform();
        break;
    }
}

void Box2dLearningTest::createHeader()
{
    const Rect visibleRect = VisibleRect::getVisibleRect();
    auto header = LayerColor::create(Color4B::BLACK, visibleRect.size.width, 105.0f);
    header->setPosition(visibleRect.origin.x, visibleRect.getMaxY() - 105.0f);
    addChild(header, 9998);
}

void Box2dLearningTest::createDebugButton()
{
    _debugLabel = Label::createWithTTF("Debug: ON", "fonts/arial.ttf", 16.0f);
    auto toggle = MenuItemLabel::create(_debugLabel, CC_CALLBACK_1(Box2dLearningTest::toggleDebug, this));
    auto menu = Menu::create(toggle, nullptr);
    menu->setPosition(VisibleRect::rightTop().x - 60.0f, VisibleRect::rightTop().y - 87.0f);
    addChild(menu, 10000);
}

void Box2dLearningTest::createScenarioControls()
{
    const float y = VisibleRect::top().y - 87.0f;
    const float left = VisibleRect::left().x;

    switch (_scenario)
    {
    case Box2dLearningScenario::ContinuousCollision:
    {
        _continuousLabel = Label::createWithTTF("CCD: ON", "fonts/arial.ttf", 14.0f);
        auto toggle = MenuItemLabel::create(_continuousLabel,
                                             CC_CALLBACK_1(Box2dLearningTest::toggleContinuous, this));
        auto menu = Menu::create(toggle, nullptr);
        menu->setPosition(left + 55.0f, y);
        addChild(menu, 10000);
        addControl("Fire", {left + 130.0f, y}, [this](Ref* sender) { fireProjectiles(sender); });
        break;
    }
    case Box2dLearningScenario::Casts:
    {
        _castModeLabel = Label::createWithTTF("Mode: Ray", "fonts/arial.ttf", 14.0f);
        auto toggle = MenuItemLabel::create(_castModeLabel, CC_CALLBACK_1(Box2dLearningTest::toggleCastMode, this));
        auto menu = Menu::create(toggle, nullptr);
        menu->setPosition(left + 70.0f, y);
        addChild(menu, 10000);
        break;
    }
    case Box2dLearningScenario::OneWayPlatform:
        addControl("Jump through", {left + 75.0f, y},
                   [this](Ref* sender) { launchOneWayPlayer(sender); });
        break;
    case Box2dLearningScenario::CharacterPlatform:
        addControl("Left", {left + 35.0f, y},
                   [this](Ref*) { moveCharacter(-3.0f, 0.0f); });
        addControl("Jump", {left + 90.0f, y},
                   [this](Ref*) { moveCharacter(0.0f, 6.0f); });
        addControl("Right", {left + 150.0f, y},
                   [this](Ref*) { moveCharacter(3.0f, 0.0f); });
        break;
    default:
        break;
    }
}

void Box2dLearningTest::addControl(const std::string& text, const Vec2& position,
                                   const std::function<void(Ref*)>& callback)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 14.0f);
    auto item = MenuItemLabel::create(label, callback);
    auto menu = Menu::create(item, nullptr);
    menu->setPosition(position);
    addChild(menu, 10000);
}

void Box2dLearningTest::addWorldLabel(const std::string& text, b2Vec2 position)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 12.0f);
    label->setPosition(worldToScreen(position));
    addChild(label, 9997);
}

void Box2dLearningTest::createGroundSegment(b2Vec2 point1, b2Vec2 point2, float friction)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = friction;
    const b2Segment segment = {point1, point2};
    b2CreateSegmentShape(ground, &shapeDef, &segment);
}

void Box2dLearningTest::createContinuousCollision()
{
    addWorldLabel("ordinary dynamic body", {-7.0f, 10.0f});
    addWorldLabel("bullet body", {-7.0f, 4.5f});
    fireProjectiles();
}

void Box2dLearningTest::fireProjectiles(Ref*)
{
    for (b2BodyId& body : _continuousBodies)
    {
        if (b2Body_IsValid(body))
        {
            b2DestroyBody(body);
        }
        body = b2_nullBodyId;
    }

    const b2Polygon targetBox = b2MakeBox(0.06f, 2.0f);
    b2ShapeDef targetShapeDef = b2DefaultShapeDef();
    targetShapeDef.density = 100.0f;
    targetShapeDef.material.customColor = b2_colorWhite;

    for (int lane = 0; lane < 2; ++lane)
    {
        const float y = lane == 0 ? 7.75f : 2.75f;

        b2BodyDef targetDef = b2DefaultBodyDef();
        targetDef.type = b2_dynamicBody;
        targetDef.fixedRotation = true;
        targetDef.enableSleep = false;
        targetDef.position = {6.0f, y};
        _continuousBodies[2 * lane + 1] = b2CreateBody(_world, &targetDef);
        b2CreatePolygonShape(_continuousBodies[2 * lane + 1], &targetShapeDef, &targetBox);

        b2BodyDef projectileDef = b2DefaultBodyDef();
        projectileDef.type = b2_dynamicBody;
        projectileDef.enableSleep = false;
        projectileDef.isBullet = lane == 1;
        projectileDef.position = {-13.0f, y};
        projectileDef.linearVelocity = {110.0f, 0.0f};
        _continuousBodies[2 * lane] = b2CreateBody(_world, &projectileDef);

        b2ShapeDef projectileShapeDef = b2DefaultShapeDef();
        projectileShapeDef.density = 1.0f;
        projectileShapeDef.material.customColor = lane == 0 ? b2_colorYellow : b2_colorCyan;
        const b2Circle projectile = {{0.0f, 0.0f}, 0.22f};
        b2CreateCircleShape(_continuousBodies[2 * lane], &projectileShapeDef, &projectile);
    }

    _elapsed = 0.0f;
}

void Box2dLearningTest::toggleContinuous(Ref*)
{
    _continuousEnabled = !_continuousEnabled;
    b2World_EnableContinuous(_world, _continuousEnabled);
    _continuousLabel->setString(_continuousEnabled ? "CCD: ON" : "CCD: OFF");
    fireProjectiles();
}

void Box2dLearningTest::createCasts()
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.customColor = b2_colorLightSteelBlue;

    b2Polygon box = b2MakeOffsetRoundedBox(2.0f, 0.7f, {-3.0f, 5.0f}, b2MakeRot(0.25f), 0.1f);
    b2CreatePolygonShape(ground, &shapeDef, &box);
    box = b2MakeOffsetBox(1.0f, 2.0f, {5.0f, 9.0f}, b2MakeRot(-0.3f));
    b2CreatePolygonShape(ground, &shapeDef, &box);

    const b2Circle circle = {{0.0f, 12.0f}, 1.2f};
    b2CreateCircleShape(ground, &shapeDef, &circle);
    const b2Capsule capsule = {{7.0f, 3.0f}, {10.0f, 3.0f}, 0.6f};
    b2CreateCapsuleShape(ground, &shapeDef, &capsule);
}

void Box2dLearningTest::installCastTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [this](Touch* touch, Event*) {
        _castTarget = screenToWorld(touch->getLocation());
        return true;
    };
    listener->onTouchMoved = [this](Touch* touch, Event*) {
        _castTarget = screenToWorld(touch->getLocation());
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void Box2dLearningTest::toggleCastMode(Ref*)
{
    _useShapeCast = !_useShapeCast;
    _castModeLabel->setString(_useShapeCast ? "Mode: Circle" : "Mode: Ray");
}

void Box2dLearningTest::createSensorEvents()
{
    b2BodyDef sensorBodyDef = b2DefaultBodyDef();
    b2BodyId sensorBody = b2CreateBody(_world, &sensorBodyDef);

    b2ShapeDef sensorDef = b2DefaultShapeDef();
    sensorDef.isSensor = true;
    sensorDef.enableSensorEvents = true;
    sensorDef.material.customColor = b2_colorMagenta;
    const b2Circle sensor = {{0.0f, 8.0f}, 3.0f};
    b2CreateCircleShape(sensorBody, &sensorDef, &sensor);

    b2ShapeDef visitorDef = b2DefaultShapeDef();
    visitorDef.enableSensorEvents = true;
    visitorDef.material.customColor = b2_colorCyan;
    const b2Circle visitor = {{0.0f, 0.0f}, 0.45f};

    for (int i = 0; i < 3; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_kinematicBody;
        bodyDef.position = {-12.0f, 6.0f + 2.0f * static_cast<float>(i)};
        bodyDef.linearVelocity = {3.0f + static_cast<float>(i), 0.0f};
        _sensorVisitors[i] = b2CreateBody(_world, &bodyDef);
        b2CreateCircleShape(_sensorVisitors[i], &visitorDef, &visitor);
    }

    addWorldLabel("sensor", {0.0f, 10.0f});
}

void Box2dLearningTest::createJointsGallery()
{
    createGroundSegment({-16.0f, 0.0f}, {16.0f, 0.0f});

    auto createAnchor = [this](b2Vec2 position) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.position = position;
        return b2CreateBody(_world, &bodyDef);
    };

    auto createBox = [this](b2Vec2 position, float hx, float hy, b2HexColor color) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = position;
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.customColor = color;
        const b2Polygon box = b2MakeRoundedBox(hx, hy, 0.05f);
        b2CreatePolygonShape(body, &shapeDef, &box);
        return body;
    };

    auto createCircle = [this](b2Vec2 position, float radius, b2HexColor color) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = position;
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.customColor = color;
        const b2Circle circle = {{0.0f, 0.0f}, radius};
        b2CreateCircleShape(body, &shapeDef, &circle);
        return body;
    };

    // Revolute pendulum.
    {
        const b2Vec2 pivot = {-10.0f, 8.8f};
        b2BodyId anchor = createAnchor(pivot);
        b2BodyId bar = createBox({-10.0f, 6.3f}, 0.3f, 2.2f, b2_colorYellow);
        b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
        jointDef.bodyIdA = anchor;
        jointDef.bodyIdB = bar;
        jointDef.localAnchorA = b2Body_GetLocalPoint(anchor, pivot);
        jointDef.localAnchorB = b2Body_GetLocalPoint(bar, pivot);
        b2CreateRevoluteJoint(_world, &jointDef);
        addWorldLabel("Revolute", {-10.0f, 10.0f});
    }

    // Spring distance joint.
    {
        b2BodyId anchor = createAnchor({0.0f, 9.0f});
        b2BodyId ball = createCircle({0.0f, 5.4f}, 0.7f, b2_colorCyan);
        b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
        jointDef.bodyIdA = anchor;
        jointDef.bodyIdB = ball;
        jointDef.length = 3.6f;
        jointDef.enableSpring = true;
        jointDef.hertz = 1.5f;
        jointDef.dampingRatio = 0.2f;
        b2CreateDistanceJoint(_world, &jointDef);
        addWorldLabel("Distance", {0.0f, 10.0f});
    }

    // Motorized prismatic slider.
    {
        b2BodyId anchor = createAnchor({10.0f, 6.5f});
        b2BodyId slider = createBox({10.0f, 6.5f}, 1.2f, 0.5f, b2_colorLimeGreen);
        b2PrismaticJointDef jointDef = b2DefaultPrismaticJointDef();
        jointDef.bodyIdA = anchor;
        jointDef.bodyIdB = slider;
        jointDef.localAxisA = {1.0f, 0.0f};
        jointDef.enableLimit = true;
        jointDef.lowerTranslation = -2.5f;
        jointDef.upperTranslation = 2.5f;
        jointDef.enableMotor = true;
        jointDef.motorSpeed = 1.5f;
        jointDef.maxMotorForce = 100.0f;
        b2CreatePrismaticJoint(_world, &jointDef);
        addWorldLabel("Prismatic", {10.0f, 10.0f});
    }

    // Suspension wheel.
    {
        b2BodyId anchor = createAnchor({-8.0f, 3.5f});
        b2BodyId wheel = createCircle({-8.0f, 2.0f}, 0.9f, b2_colorOrange);
        b2WheelJointDef jointDef = b2DefaultWheelJointDef();
        jointDef.bodyIdA = anchor;
        jointDef.bodyIdB = wheel;
        jointDef.localAnchorA = b2Body_GetLocalPoint(anchor, {-8.0f, 2.0f});
        jointDef.localAnchorB = {0.0f, 0.0f};
        jointDef.localAxisA = {0.0f, 1.0f};
        jointDef.enableSpring = true;
        jointDef.hertz = 2.0f;
        jointDef.dampingRatio = 0.7f;
        jointDef.enableMotor = true;
        jointDef.motorSpeed = 2.0f;
        jointDef.maxMotorTorque = 20.0f;
        b2CreateWheelJoint(_world, &jointDef);
        addWorldLabel("Wheel", {-8.0f, 4.5f});
    }

    // Two bodies constrained as one rigid assembly.
    {
        b2BodyId left = createBox({-1.0f, 1.8f}, 0.9f, 0.6f, b2_colorHotPink);
        b2BodyId right = createBox({1.0f, 1.8f}, 0.9f, 0.6f, b2_colorHotPink);
        const b2Vec2 pivot = {0.0f, 1.8f};
        b2WeldJointDef jointDef = b2DefaultWeldJointDef();
        jointDef.bodyIdA = left;
        jointDef.bodyIdB = right;
        jointDef.localAnchorA = b2Body_GetLocalPoint(left, pivot);
        jointDef.localAnchorB = b2Body_GetLocalPoint(right, pivot);
        b2CreateWeldJoint(_world, &jointDef);
        addWorldLabel("Weld", {0.0f, 4.5f});
    }

    // Motor joint holds a body at a target offset from a static anchor.
    {
        b2BodyId anchor = createAnchor({9.0f, 2.0f});
        b2BodyId follower = createBox({11.0f, 2.0f}, 0.8f, 0.8f, b2_colorAqua);
        b2MotorJointDef jointDef = b2DefaultMotorJointDef();
        jointDef.bodyIdA = anchor;
        jointDef.bodyIdB = follower;
        jointDef.linearOffset = {2.0f, 0.0f};
        jointDef.maxForce = 500.0f;
        jointDef.maxTorque = 500.0f;
        jointDef.correctionFactor = 0.5f;
        b2CreateMotorJoint(_world, &jointDef);
        addWorldLabel("Motor", {10.0f, 4.5f});
    }
}

void Box2dLearningTest::createOneWayPlatform()
{
    createGroundSegment({-15.0f, 0.0f}, {15.0f, 0.0f});
    b2World_SetPreSolveCallback(_world, oneWayPreSolve, this);

    b2BodyDef platformDef = b2DefaultBodyDef();
    platformDef.position = {0.0f, 7.0f};
    b2BodyId platform = b2CreateBody(_world, &platformDef);
    b2ShapeDef platformShapeDef = b2DefaultShapeDef();
    platformShapeDef.enablePreSolveEvents = true;
    platformShapeDef.material.customColor = b2_colorYellow;
    const b2Polygon platformBox = b2MakeBox(4.0f, 0.3f);
    b2CreatePolygonShape(platform, &platformShapeDef, &platformBox);

    b2BodyDef playerDef = b2DefaultBodyDef();
    playerDef.type = b2_dynamicBody;
    playerDef.fixedRotation = true;
    playerDef.position = {0.0f, 1.5f};
    _oneWayPlayer = b2CreateBody(_world, &playerDef);

    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 1.0f;
    playerShapeDef.material.friction = 0.1f;
    playerShapeDef.material.customColor = b2_colorCyan;
    const b2Capsule player = {{0.0f, -0.5f}, {0.0f, 0.5f}, _oneWayRadius};
    _oneWayPlayerShape = b2CreateCapsuleShape(_oneWayPlayer, &playerShapeDef, &player);

    addWorldLabel("one-way contact", {0.0f, 9.0f});
    launchOneWayPlayer();
}

bool Box2dLearningTest::oneWayPreSolve(b2ShapeId shapeIdA, b2ShapeId shapeIdB,
                                       b2Manifold* manifold, void* context)
{
    auto test = static_cast<Box2dLearningTest*>(context);
    return test->shouldEnableOneWayContact(shapeIdA, shapeIdB, *manifold);
}

bool Box2dLearningTest::shouldEnableOneWayContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB,
                                                  const b2Manifold& manifold) const
{
    float sign = 0.0f;
    if (B2_ID_EQUALS(shapeIdA, _oneWayPlayerShape))
    {
        sign = -1.0f;
    }
    else if (B2_ID_EQUALS(shapeIdB, _oneWayPlayerShape))
    {
        sign = 1.0f;
    }
    else
    {
        return true;
    }

    if (sign * manifold.normal.y > 0.95f)
    {
        return true;
    }

    float separation = 0.0f;
    for (int i = 0; i < manifold.pointCount; ++i)
    {
        separation = std::min(separation, manifold.points[i].separation);
    }
    return separation > 0.1f * _oneWayRadius;
}

void Box2dLearningTest::launchOneWayPlayer(Ref*)
{
    if (b2Body_IsValid(_oneWayPlayer))
    {
        b2Body_SetTransform(_oneWayPlayer, {0.0f, 1.5f}, b2Rot_identity);
        b2Body_SetLinearVelocity(_oneWayPlayer, {0.0f, 15.0f});
        _elapsed = 0.0f;
    }
}

void Box2dLearningTest::createCollisionFiltering()
{
    constexpr uint64_t GroundCategory = 0x0008;

    b2BodyDef groundDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &groundDef);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.filter.categoryBits = GroundCategory;
    groundShapeDef.filter.maskBits = UINT64_MAX;
    const b2Segment floor = {{-18.0f, 0.0f}, {18.0f, 0.0f}};
    b2CreateSegmentShape(ground, &groundShapeDef, &floor);

    const uint64_t categories[] = {0x0001, 0x0002, 0x0004};
    const b2HexColor colors[] = {b2_colorLimeGreen, b2_colorOrange, b2_colorTomato};
    const float positions[] = {-10.0f, 0.0f, 10.0f};

    for (int i = 0; i < 3; ++i)
    {
        b2BodyDef platformDef = b2DefaultBodyDef();
        platformDef.position = {positions[i], 6.0f};
        b2BodyId platform = b2CreateBody(_world, &platformDef);

        b2ShapeDef platformShapeDef = b2DefaultShapeDef();
        platformShapeDef.filter.categoryBits = categories[i];
        platformShapeDef.filter.maskBits = i == 2 ? UINT64_MAX : categories[i];
        platformShapeDef.filter.groupIndex = i == 2 ? -1 : 0;
        platformShapeDef.material.customColor = colors[i];
        const b2Polygon platformBox = b2MakeBox(3.0f, 0.25f);
        b2CreatePolygonShape(platform, &platformShapeDef, &platformBox);

        b2BodyDef ballDef = b2DefaultBodyDef();
        ballDef.type = b2_dynamicBody;
        ballDef.position = {positions[i], 15.0f};
        b2BodyId ball = b2CreateBody(_world, &ballDef);

        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.density = 1.0f;
        ballShapeDef.filter.categoryBits = categories[i];
        ballShapeDef.filter.maskBits = i == 0 ? categories[i] | GroundCategory :
                                             (i == 1 ? GroundCategory : UINT64_MAX);
        ballShapeDef.filter.groupIndex = i == 2 ? -1 : 0;
        ballShapeDef.material.customColor = colors[i];
        const b2Circle ballShape = {{0.0f, 0.0f}, 0.75f};
        b2CreateCircleShape(ball, &ballShapeDef, &ballShape);
    }

    addWorldLabel("mask match", {-10.0f, 9.5f});
    addWorldLabel("mask reject", {0.0f, 9.5f});
    addWorldLabel("group -1 ignores", {10.0f, 9.5f});
}

void Box2dLearningTest::createCharacterPlatform()
{
    createGroundSegment({-18.0f, 0.0f}, {18.0f, 0.0f}, 0.9f);

    b2BodyDef platformDef = b2DefaultBodyDef();
    platformDef.type = b2_kinematicBody;
    platformDef.position = {0.0f, 6.0f};
    platformDef.linearVelocity = {2.0f, 0.0f};
    _movingPlatform = b2CreateBody(_world, &platformDef);
    b2ShapeDef platformShapeDef = b2DefaultShapeDef();
    platformShapeDef.material.friction = 0.9f;
    platformShapeDef.material.customColor = b2_colorLimeGreen;
    const b2Polygon platform = b2MakeRoundedBox(3.0f, 0.3f, 0.08f);
    b2CreatePolygonShape(_movingPlatform, &platformShapeDef, &platform);

    b2BodyDef characterDef = b2DefaultBodyDef();
    characterDef.type = b2_dynamicBody;
    characterDef.fixedRotation = true;
    characterDef.linearDamping = 0.5f;
    characterDef.position = {0.0f, 10.0f};
    _character = b2CreateBody(_world, &characterDef);
    b2ShapeDef characterShapeDef = b2DefaultShapeDef();
    characterShapeDef.density = 1.0f;
    characterShapeDef.material.friction = 0.3f;
    characterShapeDef.material.customColor = b2_colorCyan;
    const b2Capsule character = {{0.0f, -0.5f}, {0.0f, 0.5f}, 0.45f};
    b2CreateCapsuleShape(_character, &characterShapeDef, &character);

    b2BodyDef obstacleDef = b2DefaultBodyDef();
    b2BodyId obstacles = b2CreateBody(_world, &obstacleDef);
    b2ShapeDef obstacleShapeDef = b2DefaultShapeDef();
    obstacleShapeDef.material.friction = 0.8f;
    obstacleShapeDef.material.customColor = b2_colorLightSteelBlue;
    b2Polygon step = b2MakeOffsetBox(2.0f, 0.4f, {-10.0f, 2.0f}, b2Rot_identity);
    b2CreatePolygonShape(obstacles, &obstacleShapeDef, &step);
    step = b2MakeOffsetBox(2.5f, 0.3f, {10.0f, 2.5f}, b2MakeRot(0.2f));
    b2CreatePolygonShape(obstacles, &obstacleShapeDef, &step);

    addWorldLabel("kinematic platform", {0.0f, 8.0f});
}

void Box2dLearningTest::moveCharacter(float horizontalImpulse, float verticalImpulse)
{
    if (b2Body_IsValid(_character))
    {
        b2Body_ApplyLinearImpulseToCenter(_character, {horizontalImpulse, verticalImpulse}, true);
    }
}

void Box2dLearningTest::toggleDebug(Ref*)
{
    _debugEnabled = !_debugEnabled;
    _debugDraw.setEnabled(_debugEnabled);
    _debugLabel->setString(_debugEnabled ? "Debug: ON" : "Debug: OFF");
}

void Box2dLearningTest::update(float dt)
{
    b2World_Step(_world, TimeStep, SubStepCount);

    switch (_scenario)
    {
    case Box2dLearningScenario::ContinuousCollision:
        updateContinuous(dt);
        break;
    case Box2dLearningScenario::Casts:
        updateCasts();
        break;
    case Box2dLearningScenario::SensorEvents:
        updateSensors();
        break;
    case Box2dLearningScenario::OneWayPlatform:
        updateOneWayPlatform(dt);
        break;
    case Box2dLearningScenario::CharacterPlatform:
        updateCharacterPlatform();
        break;
    default:
        break;
    }
}

void Box2dLearningTest::updateContinuous(float dt)
{
    _elapsed += dt;
    if (_elapsed > 3.0f)
    {
        fireProjectiles();
    }

    const float ordinaryX = b2Body_IsValid(_continuousBodies[0]) ?
        b2Body_GetPosition(_continuousBodies[0]).x : 0.0f;
    const float bulletX = b2Body_IsValid(_continuousBodies[2]) ?
        b2Body_GetPosition(_continuousBodies[2]).x : 0.0f;
    _infoLabel->setString(StringUtils::format("ordinary x %.1f   bullet x %.1f", ordinaryX, bulletX));
}

void Box2dLearningTest::updateCasts()
{
    _overlay->clear();

    const b2Vec2 translation = b2Sub(_castTarget, _castOrigin);
    b2Vec2 hitPoint = _castTarget;
    b2Vec2 hitNormal = {0.0f, 0.0f};
    float hitFraction = 1.0f;
    bool hit = false;

    if (_useShapeCast)
    {
        struct CastContext
        {
            b2Vec2 point;
            b2Vec2 normal;
            float fraction = 1.0f;
            bool hit = false;
        } context;

        auto callback = [](b2ShapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* data) -> float {
            auto cast = static_cast<CastContext*>(data);
            if (!cast->hit || fraction < cast->fraction)
            {
                cast->point = point;
                cast->normal = normal;
                cast->fraction = fraction;
                cast->hit = true;
            }
            return fraction;
        };

        const b2Circle castCircle = {_castOrigin, 0.55f};
        const b2ShapeProxy proxy = b2MakeProxy(&castCircle.center, 1, castCircle.radius);
        b2World_CastShape(_world, &proxy, translation, b2DefaultQueryFilter(), callback, &context);
        hit = context.hit;
        hitPoint = context.point;
        hitNormal = context.normal;
        hitFraction = context.fraction;

        _overlay->drawCircle(worldToScreen(_castOrigin), castCircle.radius * PixelsPerMeter,
                             0.0f, 24, false, colorForHex(b2_colorCyan));
        const b2Vec2 finalCenter = b2MulAdd(_castOrigin, hitFraction, translation);
        _overlay->drawCircle(worldToScreen(finalCenter), castCircle.radius * PixelsPerMeter,
                             0.0f, 24, false, colorForHex(b2_colorCyan));
    }
    else
    {
        const b2RayResult result =
            b2World_CastRayClosest(_world, _castOrigin, translation, b2DefaultQueryFilter());
        hit = result.hit;
        if (hit)
        {
            hitPoint = result.point;
            hitNormal = result.normal;
            hitFraction = result.fraction;
        }
    }

    const b2Vec2 castEnd = b2MulAdd(_castOrigin, hitFraction, translation);
    _overlay->drawSegment(worldToScreen(_castOrigin), worldToScreen(castEnd), 1.0f,
                          colorForHex(_useShapeCast ? b2_colorCyan : b2_colorYellow));
    _overlay->drawDot(worldToScreen(_castTarget), 4.0f, colorForHex(b2_colorWhite));

    if (hit)
    {
        _overlay->drawDot(worldToScreen(hitPoint), 5.0f, colorForHex(b2_colorLime));
        _overlay->drawSegment(worldToScreen(hitPoint),
                              worldToScreen(b2MulAdd(hitPoint, 1.2f, hitNormal)),
                              1.5f, colorForHex(b2_colorRed));
        _infoLabel->setString(StringUtils::format("hit fraction %.3f", hitFraction));
    }
    else
    {
        _infoLabel->setString("no hit");
    }
}

void Box2dLearningTest::updateSensors()
{
    const b2SensorEvents events = b2World_GetSensorEvents(_world);
    _sensorBeginCount += events.beginCount;
    _sensorEndCount += events.endCount;
    _sensorInsideCount = std::max(0, _sensorInsideCount + events.beginCount - events.endCount);

    for (b2BodyId visitor : _sensorVisitors)
    {
        const b2Vec2 position = b2Body_GetPosition(visitor);
        const b2Vec2 velocity = b2Body_GetLinearVelocity(visitor);
        if (position.x > 12.0f && velocity.x > 0.0f)
        {
            b2Body_SetLinearVelocity(visitor, {-velocity.x, 0.0f});
        }
        else if (position.x < -12.0f && velocity.x < 0.0f)
        {
            b2Body_SetLinearVelocity(visitor, {-velocity.x, 0.0f});
        }
    }

    _infoLabel->setString(StringUtils::format("inside %d   begin %d   end %d",
                                               _sensorInsideCount, _sensorBeginCount, _sensorEndCount));
}

void Box2dLearningTest::updateOneWayPlatform(float dt)
{
    _elapsed += dt;
    const b2Vec2 position = b2Body_GetPosition(_oneWayPlayer);
    const b2Vec2 velocity = b2Body_GetLinearVelocity(_oneWayPlayer);
    if (_elapsed > 5.0f || position.y < -1.0f)
    {
        launchOneWayPlayer();
    }
    _infoLabel->setString(StringUtils::format("player y %.1f   vertical speed %.1f",
                                               position.y, velocity.y));
}

void Box2dLearningTest::updateCharacterPlatform()
{
    const b2Vec2 platformPosition = b2Body_GetPosition(_movingPlatform);
    const b2Vec2 platformVelocity = b2Body_GetLinearVelocity(_movingPlatform);
    if (platformPosition.x < -8.0f && platformVelocity.x < 0.0f)
    {
        b2Body_SetLinearVelocity(_movingPlatform, {2.0f, 0.0f});
    }
    else if (platformPosition.x > 8.0f && platformVelocity.x > 0.0f)
    {
        b2Body_SetLinearVelocity(_movingPlatform, {-2.0f, 0.0f});
    }

    const b2Vec2 characterVelocity = b2Body_GetLinearVelocity(_character);
    _infoLabel->setString(StringUtils::format("character speed %.1f, %.1f",
                                               characterVelocity.x, characterVelocity.y));
}

Vec2 Box2dLearningTest::worldToScreen(b2Vec2 position) const
{
    return {
        VisibleRect::center().x + position.x * PixelsPerMeter,
        VisibleRect::bottom().y + 55.0f + position.y * PixelsPerMeter,
    };
}

b2Vec2 Box2dLearningTest::screenToWorld(const Vec2& position) const
{
    return {
        (position.x - VisibleRect::center().x) / PixelsPerMeter,
        (position.y - VisibleRect::bottom().y - 55.0f) / PixelsPerMeter,
    };
}

void Box2dLearningTest::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
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

void Box2dLearningTest::onDebugDraw(Mat4 transform)
{
    transform.translate(VisibleRect::center().x, VisibleRect::bottom().y + 55.0f, 0.0f);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    b2World_Draw(_world, _debugDraw.getDebugDraw());
    CHECK_GL_ERROR_DEBUG();
    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
