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

#include "Box2dGameplayTests.h"

#include "renderer/CCRenderer.h"

#include <algorithm>
#include <cmath>

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

Box2dGameplayTest* Box2dGameplayTest::create(Box2dGameplayScenario scenario)
{
    auto test = new (std::nothrow) Box2dGameplayTest(scenario);
    if (test && test->init())
    {
        test->autorelease();
        return test;
    }

    CC_SAFE_DELETE(test);
    return nullptr;
}

Box2dGameplayTest::Box2dGameplayTest(Box2dGameplayScenario scenario)
    : _scenario(scenario)
    , _world(b2_nullWorldId)
    , _debugDraw(PixelsPerMeter)
    , _debugLabel(nullptr)
    , _infoLabel(nullptr)
    , _overlay(nullptr)
    , _debugEnabled(true)
    , _eventBodies{}
    , _contactBeginCount(0)
    , _contactEndCount(0)
    , _contactHitCount(0)
    , _bodyMoveCount(0)
    , _fellAsleepCount(0)
    , _lastHitPoint{0.0f, 0.0f}
    , _lastHitNormal{0.0f, 1.0f}
    , _lastHitSpeed(0.0f)
    , _hitMarkerTime(0.0f)
    , _terrainChain(b2_nullChainId)
    , _chainRunner(b2_nullBodyId)
    , _conveyorShape(b2_nullShapeId)
    , _conveyorBodies{}
    , _conveyorSpeed(3.0f)
    , _breakAnchor(b2_nullBodyId)
    , _breakBodies{}
    , _breakJoints{}
    , _breakForce(150.0f)
    , _breakPending(false)
    , _explosionBodies{}
    , _explosionRadius(2.5f)
    , _explosionFalloff(4.0f)
    , _lastExplosionImpulse(0.0f)
{
    _eventBodies.fill(b2_nullBodyId);
    _conveyorBodies.fill(b2_nullBodyId);
    _breakBodies.fill(b2_nullBodyId);
    _breakJoints.fill(b2_nullJointId);
    _explosionBodies.fill(b2_nullBodyId);
}

Box2dGameplayTest::~Box2dGameplayTest()
{
    if (B2_IS_NON_NULL(_world))
    {
        b2DestroyWorld(_world);
    }
}

bool Box2dGameplayTest::init()
{
    if (!TestCase::init())
    {
        return false;
    }

    createWorld();
    createHeader();

    _overlay = DrawNode::create();
    addChild(_overlay, 9997);

    _infoLabel = Label::createWithTTF("", "fonts/arial.ttf", 12.0f);
    _infoLabel->setPosition(VisibleRect::center().x + 25.0f, VisibleRect::top().y - 88.0f);
    addChild(_infoLabel, 10000);

    createDebugButton();
    createScenarioControls();
    scheduleUpdate();
    return true;
}

std::string Box2dGameplayTest::title() const
{
    switch (_scenario)
    {
    case Box2dGameplayScenario::ContactAndMoveEvents:
        return "Box2D 3.1.1 - Contact and Move Events";
    case Box2dGameplayScenario::ChainTerrain:
        return "Box2D 3.1.1 - Chain Terrain";
    case Box2dGameplayScenario::ConveyorBelt:
        return "Box2D 3.1.1 - Conveyor Belt";
    case Box2dGameplayScenario::BreakableJoint:
        return "Box2D 3.1.1 - Breakable Joint";
    case Box2dGameplayScenario::Explosion:
        return "Box2D 3.1.1 - Explosion";
    }
    return "Box2D 3.1.1";
}

std::string Box2dGameplayTest::subtitle() const
{
    switch (_scenario)
    {
    case Box2dGameplayScenario::ContactAndMoveEvents:
        return "Buffered begin/end/hit contacts and contiguous body-move events";
    case Box2dGameplayScenario::ChainTerrain:
        return "A one-sided chain removes internal-edge ghost bumps";
    case Box2dGameplayScenario::ConveyorBelt:
        return "Surface tangent speed moves bodies without moving the platform";
    case Box2dGameplayScenario::BreakableJoint:
        return "Joints break when measured constraint force exceeds the limit";
    case Box2dGameplayScenario::Explosion:
        return "Geometry-aware radial impulse with radius and falloff";
    }
    return "";
}

void Box2dGameplayTest::createWorld()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.hitEventThreshold = 1.0f;
    if (_scenario == Box2dGameplayScenario::Explosion)
    {
        worldDef.gravity = {0.0f, 0.0f};
    }
    else
    {
        worldDef.gravity = {0.0f, -10.0f};
    }
    _world = b2CreateWorld(&worldDef);

    switch (_scenario)
    {
    case Box2dGameplayScenario::ContactAndMoveEvents:
        createContactAndMoveEvents();
        break;
    case Box2dGameplayScenario::ChainTerrain:
        createChainTerrain();
        break;
    case Box2dGameplayScenario::ConveyorBelt:
        createConveyorBelt();
        break;
    case Box2dGameplayScenario::BreakableJoint:
        createBreakableJoint();
        break;
    case Box2dGameplayScenario::Explosion:
        createExplosion();
        break;
    }
}

void Box2dGameplayTest::createHeader()
{
    const Rect visibleRect = VisibleRect::getVisibleRect();
    auto header = LayerColor::create(Color4B::BLACK, visibleRect.size.width, 105.0f);
    header->setPosition(visibleRect.origin.x, visibleRect.getMaxY() - 105.0f);
    addChild(header, 9998);
}

void Box2dGameplayTest::createDebugButton()
{
    _debugLabel = Label::createWithTTF("Debug: ON", "fonts/arial.ttf", 16.0f);
    auto toggle = MenuItemLabel::create(_debugLabel, CC_CALLBACK_1(Box2dGameplayTest::toggleDebug, this));
    auto menu = Menu::create(toggle, nullptr);
    menu->setPosition(VisibleRect::rightTop().x - 60.0f, VisibleRect::rightTop().y - 87.0f);
    addChild(menu, 10000);
}

void Box2dGameplayTest::createScenarioControls()
{
    const float y = VisibleRect::top().y - 87.0f;
    const float left = VisibleRect::left().x;

    switch (_scenario)
    {
    case Box2dGameplayScenario::ContactAndMoveEvents:
        addControl("Reset drop", {left + 55.0f, y},
                   [this](Ref* sender) { resetEventBodies(sender); });
        break;
    case Box2dGameplayScenario::ChainTerrain:
        addControl("Relaunch", {left + 45.0f, y},
                   [this](Ref* sender) { launchChainRunner(sender); });
        break;
    case Box2dGameplayScenario::ConveyorBelt:
        addControl("Reverse", {left + 42.0f, y},
                   [this](Ref* sender) { reverseConveyor(sender); });
        addControl("Reset", {left + 102.0f, y},
                   [this](Ref* sender) { resetConveyorBodies(sender); });
        break;
    case Box2dGameplayScenario::BreakableJoint:
        addControl("Stress", {left + 38.0f, y},
                   [this](Ref* sender) { stressBreakableChain(sender); });
        addControl("Reset", {left + 92.0f, y},
                   [this](Ref* sender) { resetBreakableChain(sender); });
        break;
    case Box2dGameplayScenario::Explosion:
        addControl("Explode", {left + 38.0f, y},
                   [this](Ref*) { explode(3.0f); });
        addControl("Implode", {left + 98.0f, y},
                   [this](Ref*) { explode(-3.0f); });
        addControl("Reset", {left + 153.0f, y},
                   [this](Ref* sender) { resetExplosionBodies(sender); });
        break;
    }
}

void Box2dGameplayTest::addControl(const std::string& text, const Vec2& position,
                                   const std::function<void(Ref*)>& callback)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 13.0f);
    auto item = MenuItemLabel::create(label, callback);
    auto menu = Menu::create(item, nullptr);
    menu->setPosition(position);
    addChild(menu, 10000);
}

void Box2dGameplayTest::addWorldLabel(const std::string& text, b2Vec2 position)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 12.0f);
    label->setPosition(worldToScreen(position));
    addChild(label, 9997);
}

b2BodyId Box2dGameplayTest::createGroundSegment(b2Vec2 point1, b2Vec2 point2)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = 0.7f;
    shapeDef.material.customColor = b2_colorLightSteelBlue;
    const b2Segment segment = {point1, point2};
    b2CreateSegmentShape(ground, &shapeDef, &segment);
    return ground;
}

void Box2dGameplayTest::createContactAndMoveEvents()
{
    createGroundSegment({-16.0f, 0.0f}, {16.0f, 0.0f});
    createGroundSegment({-16.0f, 0.0f}, {-16.0f, 12.0f});
    createGroundSegment({16.0f, 0.0f}, {16.0f, 12.0f});
    createGroundSegment({-12.0f, 5.0f}, {-3.0f, 3.0f});
    createGroundSegment({4.0f, 4.0f}, {12.0f, 6.0f});
    addWorldLabel("white marks = move-event transforms", {0.0f, 9.5f});
    resetEventBodies();
}

void Box2dGameplayTest::resetEventBodies(Ref*)
{
    for (b2BodyId& body : _eventBodies)
    {
        if (b2Body_IsValid(body))
        {
            b2DestroyBody(body);
        }
        body = b2_nullBodyId;
    }

    _contactBeginCount = 0;
    _contactEndCount = 0;
    _contactHitCount = 0;
    _bodyMoveCount = 0;
    _fellAsleepCount = 0;
    _hitMarkerTime = 0.0f;

    for (int i = 0; i < static_cast<int>(_eventBodies.size()); ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {-10.5f + 3.0f * static_cast<float>(i % 4),
                            12.0f + 2.2f * static_cast<float>(i / 4)};
        bodyDef.angularVelocity = i % 2 == 0 ? 1.2f : -0.8f;
        _eventBodies[i] = b2CreateBody(_world, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.5f;
        shapeDef.material.restitution = 0.25f;
        shapeDef.material.customColor = i % 2 == 0 ? b2_colorCyan : b2_colorOrange;
        shapeDef.enableContactEvents = true;
        shapeDef.enableHitEvents = true;

        if (i % 3 == 0)
        {
            const b2Circle circle = {{0.0f, 0.0f}, 0.65f};
            b2CreateCircleShape(_eventBodies[i], &shapeDef, &circle);
        }
        else if (i % 3 == 1)
        {
            const b2Capsule capsule = {{0.0f, -0.45f}, {0.0f, 0.45f}, 0.35f};
            b2CreateCapsuleShape(_eventBodies[i], &shapeDef, &capsule);
        }
        else
        {
            const b2Polygon box = b2MakeRoundedBox(0.6f, 0.6f, 0.08f);
            b2CreatePolygonShape(_eventBodies[i], &shapeDef, &box);
        }
    }
}

void Box2dGameplayTest::createChainTerrain()
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);

    // Counter-clockwise loop: the right-side normals of the top edges point up.
    const b2Vec2 points[] = {
        {18.0f, 2.0f}, {15.0f, 2.0f}, {12.0f, 3.2f}, {9.0f, 2.0f},
        {6.0f, 4.2f}, {3.0f, 2.0f}, {0.0f, 3.4f}, {-3.0f, 2.0f},
        {-6.0f, 4.0f}, {-9.0f, 2.0f}, {-12.0f, 3.0f}, {-15.0f, 2.0f},
        {-18.0f, 2.0f}, {-18.0f, -3.0f}, {18.0f, -3.0f},
    };
    b2SurfaceMaterial material = b2DefaultSurfaceMaterial();
    material.friction = 0.25f;
    material.customColor = b2_colorLightSteelBlue;
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.points = points;
    chainDef.count = static_cast<int>(sizeof(points) / sizeof(points[0]));
    chainDef.materials = &material;
    chainDef.materialCount = 1;
    chainDef.isLoop = true;
    _terrainChain = b2CreateChain(ground, &chainDef);

    addWorldLabel("one chain - smooth shared vertices", {0.0f, 9.5f});
    launchChainRunner();
}

void Box2dGameplayTest::launchChainRunner(Ref*)
{
    if (b2Body_IsValid(_chainRunner))
    {
        b2DestroyBody(_chainRunner);
    }

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {-14.0f, 7.0f};
    bodyDef.linearVelocity = {6.0f, 0.0f};
    _chainRunner = b2CreateBody(_world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.1f;
    shapeDef.material.customColor = b2_colorYellow;
    const b2Circle circle = {{0.0f, 0.0f}, 0.75f};
    b2CreateCircleShape(_chainRunner, &shapeDef, &circle);
}

void Box2dGameplayTest::createConveyorBelt()
{
    createGroundSegment({-16.0f, 0.0f}, {16.0f, 0.0f});

    b2BodyDef platformDef = b2DefaultBodyDef();
    platformDef.position = {0.0f, 5.0f};
    b2BodyId platform = b2CreateBody(_world, &platformDef);
    b2ShapeDef platformShapeDef = b2DefaultShapeDef();
    platformShapeDef.material.friction = 0.9f;
    platformShapeDef.material.tangentSpeed = _conveyorSpeed;
    platformShapeDef.material.customColor = b2_colorLimeGreen;
    const b2Polygon belt = b2MakeRoundedBox(12.0f, 0.3f, 0.2f);
    _conveyorShape = b2CreatePolygonShape(platform, &platformShapeDef, &belt);

    addWorldLabel("static body; contact tangent speed does the moving", {0.0f, 9.5f});
    resetConveyorBodies();
}

void Box2dGameplayTest::resetConveyorBodies(Ref*)
{
    for (b2BodyId& body : _conveyorBodies)
    {
        if (b2Body_IsValid(body))
        {
            b2DestroyBody(body);
        }
        body = b2_nullBodyId;
    }

    for (int i = 0; i < static_cast<int>(_conveyorBodies.size()); ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {-8.0f + 3.0f * static_cast<float>(i), 7.0f};
        _conveyorBodies[i] = b2CreateBody(_world, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.8f;
        shapeDef.material.customColor = i % 2 == 0 ? b2_colorCyan : b2_colorOrange;
        const b2Polygon box = b2MakeRoundedBox(0.65f, 0.65f, 0.08f);
        b2CreatePolygonShape(_conveyorBodies[i], &shapeDef, &box);
    }
}

void Box2dGameplayTest::reverseConveyor(Ref*)
{
    _conveyorSpeed = -_conveyorSpeed;
    if (b2Shape_IsValid(_conveyorShape))
    {
        b2SurfaceMaterial material = b2Shape_GetSurfaceMaterial(_conveyorShape);
        material.tangentSpeed = _conveyorSpeed;
        b2Shape_SetSurfaceMaterial(_conveyorShape, material);
    }
}

void Box2dGameplayTest::createBreakableJoint()
{
    createGroundSegment({-16.0f, 0.0f}, {16.0f, 0.0f});
    b2BodyDef anchorDef = b2DefaultBodyDef();
    anchorDef.position = {0.0f, 10.0f};
    _breakAnchor = b2CreateBody(_world, &anchorDef);
    addWorldLabel("Stress adds a downward impulse", {-8.0f, 8.5f});
    resetBreakableChain();
}

void Box2dGameplayTest::resetBreakableChain(Ref*)
{
    for (b2BodyId& body : _breakBodies)
    {
        if (b2Body_IsValid(body))
        {
            b2DestroyBody(body);
        }
        body = b2_nullBodyId;
    }
    _breakJoints.fill(b2_nullJointId);
    _breakPending = false;

    b2BodyId previous = _breakAnchor;
    for (int i = 0; i < static_cast<int>(_breakBodies.size()); ++i)
    {
        const float y = 8.7f - 1.3f * static_cast<float>(i);
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {0.0f, y};
        bodyDef.enableSleep = false;
        _breakBodies[i] = b2CreateBody(_world, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = i == static_cast<int>(_breakBodies.size()) - 1 ? 4.0f : 1.0f;
        shapeDef.material.customColor =
            i == static_cast<int>(_breakBodies.size()) - 1 ? b2_colorOrange : b2_colorCyan;
        const b2Polygon link = b2MakeRoundedBox(0.45f, 0.52f, 0.08f);
        b2CreatePolygonShape(_breakBodies[i], &shapeDef, &link);

        const b2Vec2 pivot = {0.0f, y + 0.65f};
        b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
        jointDef.bodyIdA = previous;
        jointDef.bodyIdB = _breakBodies[i];
        jointDef.localAnchorA = b2Body_GetLocalPoint(previous, pivot);
        jointDef.localAnchorB = b2Body_GetLocalPoint(_breakBodies[i], pivot);
        _breakJoints[i] = b2CreateRevoluteJoint(_world, &jointDef);
        previous = _breakBodies[i];
    }
}

void Box2dGameplayTest::stressBreakableChain(Ref*)
{
    const b2BodyId weight = _breakBodies.back();
    if (b2Body_IsValid(weight))
    {
        b2Body_ApplyLinearImpulseToCenter(weight, {18.0f, -35.0f}, true);
        _breakPending = true;
    }
}

void Box2dGameplayTest::createExplosion()
{
    addWorldLabel("inner circle = full impulse; outer circle = falloff", {0.0f, 9.5f});
    resetExplosionBodies();
}

void Box2dGameplayTest::resetExplosionBodies(Ref*)
{
    for (b2BodyId& body : _explosionBodies)
    {
        if (b2Body_IsValid(body))
        {
            b2DestroyBody(body);
        }
        body = b2_nullBodyId;
    }

    _lastExplosionImpulse = 0.0f;
    const int count = static_cast<int>(_explosionBodies.size());
    for (int i = 0; i < count; ++i)
    {
        const float angle = 2.0f * B2_PI * static_cast<float>(i) / static_cast<float>(count);
        const float radius = i % 2 == 0 ? 4.0f : 5.4f;
        const b2CosSin cs = b2ComputeCosSin(angle);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {radius * cs.cosine, 6.0f + radius * cs.sine};
        bodyDef.rotation = b2MakeRot(angle);
        bodyDef.linearDamping = 0.15f;
        bodyDef.angularDamping = 0.15f;
        _explosionBodies[i] = b2CreateBody(_world, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.customColor = i % 2 == 0 ? b2_colorYellow : b2_colorHotPink;
        if (i % 3 == 0)
        {
            const b2Circle circle = {{0.0f, 0.0f}, 0.45f};
            b2CreateCircleShape(_explosionBodies[i], &shapeDef, &circle);
        }
        else
        {
            const b2Polygon box = b2MakeRoundedBox(0.55f, 0.25f, 0.06f);
            b2CreatePolygonShape(_explosionBodies[i], &shapeDef, &box);
        }
    }
}

void Box2dGameplayTest::explode(float impulse)
{
    b2ExplosionDef explosionDef = b2DefaultExplosionDef();
    explosionDef.position = {0.0f, 6.0f};
    explosionDef.radius = _explosionRadius;
    explosionDef.falloff = _explosionFalloff;
    explosionDef.impulsePerLength = impulse;
    b2World_Explode(_world, &explosionDef);
    _lastExplosionImpulse = impulse;
}

void Box2dGameplayTest::toggleDebug(Ref*)
{
    _debugEnabled = !_debugEnabled;
    _debugDraw.setEnabled(_debugEnabled);
    _debugLabel->setString(_debugEnabled ? "Debug: ON" : "Debug: OFF");
}

void Box2dGameplayTest::update(float dt)
{
    b2World_Step(_world, TimeStep, SubStepCount);

    switch (_scenario)
    {
    case Box2dGameplayScenario::ContactAndMoveEvents:
        updateContactAndMoveEvents(dt);
        break;
    case Box2dGameplayScenario::ChainTerrain:
        updateChainTerrain();
        break;
    case Box2dGameplayScenario::ConveyorBelt:
        updateConveyorBelt();
        break;
    case Box2dGameplayScenario::BreakableJoint:
        updateBreakableJoint();
        break;
    case Box2dGameplayScenario::Explosion:
        updateExplosion();
        break;
    }
}

void Box2dGameplayTest::updateContactAndMoveEvents(float dt)
{
    _overlay->clear();

    const b2ContactEvents contacts = b2World_GetContactEvents(_world);
    _contactBeginCount += contacts.beginCount;
    _contactEndCount += contacts.endCount;
    _contactHitCount += contacts.hitCount;
    for (int i = 0; i < contacts.hitCount; ++i)
    {
        const b2ContactHitEvent& hit = contacts.hitEvents[i];
        _lastHitPoint = hit.point;
        _lastHitNormal = hit.normal;
        _lastHitSpeed = hit.approachSpeed;
        _hitMarkerTime = 0.4f;
    }

    const b2BodyEvents bodyEvents = b2World_GetBodyEvents(_world);
    _bodyMoveCount = bodyEvents.moveCount;
    for (int i = 0; i < bodyEvents.moveCount; ++i)
    {
        const b2BodyMoveEvent& event = bodyEvents.moveEvents[i];
        const Vec2 point = worldToScreen(event.transform.p);
        _overlay->drawDot(point, 2.0f, colorForHex(b2_colorWhite));
        const b2Vec2 axisEnd = b2Add(event.transform.p,
                                     {0.6f * event.transform.q.c, 0.6f * event.transform.q.s});
        _overlay->drawSegment(point, worldToScreen(axisEnd), 1.0f, colorForHex(b2_colorWhite));
        if (event.fellAsleep)
        {
            ++_fellAsleepCount;
        }
    }

    _hitMarkerTime = std::max(0.0f, _hitMarkerTime - dt);
    if (_hitMarkerTime > 0.0f)
    {
        _overlay->drawDot(worldToScreen(_lastHitPoint), 5.0f, colorForHex(b2_colorRed));
        _overlay->drawSegment(worldToScreen(_lastHitPoint),
                              worldToScreen(b2MulAdd(_lastHitPoint, 1.3f, _lastHitNormal)),
                              1.5f, colorForHex(b2_colorYellow));
    }

    _infoLabel->setString(StringUtils::format("B/E/H %d/%d/%d | move %d sleep %d | hit %.1f",
                                               _contactBeginCount, _contactEndCount, _contactHitCount,
                                               _bodyMoveCount, _fellAsleepCount, _lastHitSpeed));
}

void Box2dGameplayTest::updateChainTerrain()
{
    if (!b2Body_IsValid(_chainRunner))
    {
        return;
    }

    b2Body_ApplyForceToCenter(_chainRunner, {15.0f, 0.0f}, true);
    const b2Vec2 position = b2Body_GetPosition(_chainRunner);
    if (position.x > 16.0f || position.y < -2.0f)
    {
        launchChainRunner();
        return;
    }

    const b2Vec2 velocity = b2Body_GetLinearVelocity(_chainRunner);
    _infoLabel->setString(StringUtils::format("runner x %.1f y %.1f | speed %.1f",
                                               position.x, position.y, b2Length(velocity)));
}

void Box2dGameplayTest::updateConveyorBelt()
{
    float sumX = 0.0f;
    int count = 0;
    for (b2BodyId body : _conveyorBodies)
    {
        if (b2Body_IsValid(body))
        {
            sumX += b2Body_GetPosition(body).x;
            ++count;
        }
    }

    float averageX = count > 0 ? sumX / count : 0.0f;
    if (count > 0 && std::fabs(averageX) > 14.0f)
    {
        resetConveyorBodies();
        averageX = 0.0f;
    }

    _infoLabel->setString(StringUtils::format("speed %+.1f | avg x %.1f",
                                               _conveyorSpeed, averageX));
}

void Box2dGameplayTest::updateBreakableJoint()
{
    int intactCount = 0;
    float maximumForce = 0.0f;
    int breakIndex = -1;
    for (int i = 0; i < static_cast<int>(_breakJoints.size()); ++i)
    {
        b2JointId& joint = _breakJoints[i];
        if (!b2Joint_IsValid(joint))
        {
            joint = b2_nullJointId;
            continue;
        }

        ++intactCount;
        const float force = b2Length(b2Joint_GetConstraintForce(joint));
        if (force > maximumForce)
        {
            maximumForce = force;
            breakIndex = i;
        }
    }

    if (_breakPending && breakIndex >= 0 && maximumForce > _breakForce)
    {
        b2DestroyJoint(_breakJoints[breakIndex]);
        _breakJoints[breakIndex] = b2_nullJointId;
        _breakPending = false;
        --intactCount;
    }

    _infoLabel->setString(StringUtils::format("intact %d / %d | max %.0f N | break %.0f N",
                                               intactCount, static_cast<int>(_breakJoints.size()),
                                               maximumForce, _breakForce));
}

void Box2dGameplayTest::updateExplosion()
{
    _overlay->clear();
    const Vec2 center = worldToScreen({0.0f, 6.0f});
    _overlay->drawCircle(center, _explosionRadius * PixelsPerMeter, 0.0f, 40, false,
                         colorForHex(b2_colorYellow));
    _overlay->drawCircle(center, (_explosionRadius + _explosionFalloff) * PixelsPerMeter,
                         0.0f, 48, false, colorForHex(b2_colorAzure, 0.7f));

    float speedSum = 0.0f;
    int count = 0;
    for (b2BodyId body : _explosionBodies)
    {
        if (b2Body_IsValid(body))
        {
            speedSum += b2Length(b2Body_GetLinearVelocity(body));
            ++count;
        }
    }
    _infoLabel->setString(StringUtils::format("impulse %+.0f | avg speed %.1f",
                                               _lastExplosionImpulse,
                                               count > 0 ? speedSum / count : 0.0f));
}

Vec2 Box2dGameplayTest::worldToScreen(b2Vec2 position) const
{
    return {
        VisibleRect::center().x + position.x * PixelsPerMeter,
        VisibleRect::bottom().y + 55.0f + position.y * PixelsPerMeter,
    };
}

void Box2dGameplayTest::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
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

void Box2dGameplayTest::onDebugDraw(Mat4 transform)
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
