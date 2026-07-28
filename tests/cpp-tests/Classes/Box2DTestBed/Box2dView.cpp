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

#include "Box2dView.h"
#include "Box2dAdvancedTests.h"
#include "Box2dGameplayTests.h"
#include "Box2dLearningTests.h"

#include "renderer/CCRenderer.h"

USING_NS_CC;

namespace
{
constexpr float PixelsPerMeter = 15.0f;
constexpr float TimeStep = 1.0f / 60.0f;
constexpr int SubStepCount = 4;
}

Box2dTestBedSuite::Box2dTestBedSuite()
{
    addTestCase("Pyramid", []() { return Box2dTestBed::create(Box2dTestBedScenario::Pyramid); });
    addTestCase("Friction", []() { return Box2dTestBed::create(Box2dTestBedScenario::Friction); });
    addTestCase("Restitution", []() { return Box2dTestBed::create(Box2dTestBedScenario::Restitution); });
    addTestCase("Bridge", []() { return Box2dTestBed::create(Box2dTestBedScenario::Bridge); });
    addTestCase("Capsule Stack", []() { return Box2dTestBed::create(Box2dTestBedScenario::CapsuleStack); });
    addTestCase("Continuous Collision", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::ContinuousCollision);
    });
    addTestCase("Ray and Shape Cast", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::Casts);
    });
    addTestCase("Sensor Events", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::SensorEvents);
    });
    addTestCase("Joints Gallery", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::JointsGallery);
    });
    addTestCase("One-Way Platform", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::OneWayPlatform);
    });
    addTestCase("Collision Filtering", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::CollisionFiltering);
    });
    addTestCase("Character and Platform", []() {
        return Box2dLearningTest::create(Box2dLearningScenario::CharacterPlatform);
    });
    addTestCase("Contact and Move Events", []() {
        return Box2dGameplayTest::create(Box2dGameplayScenario::ContactAndMoveEvents);
    });
    addTestCase("Chain Terrain", []() {
        return Box2dGameplayTest::create(Box2dGameplayScenario::ChainTerrain);
    });
    addTestCase("Conveyor Belt", []() {
        return Box2dGameplayTest::create(Box2dGameplayScenario::ConveyorBelt);
    });
    addTestCase("Breakable Joint", []() {
        return Box2dGameplayTest::create(Box2dGameplayScenario::BreakableJoint);
    });
    addTestCase("Explosion", []() {
        return Box2dGameplayTest::create(Box2dGameplayScenario::Explosion);
    });
    addTestCase("Car", []() {
        return Box2dAdvancedTest::create(Box2dAdvancedScenario::Car);
    });
    addTestCase("Ragdoll", []() {
        return Box2dAdvancedTest::create(Box2dAdvancedScenario::Ragdoll);
    });
    addTestCase("Soft Body", []() {
        return Box2dAdvancedTest::create(Box2dAdvancedScenario::SoftBody);
    });
}

Box2dTestBed* Box2dTestBed::create(Box2dTestBedScenario scenario)
{
    auto test = new (std::nothrow) Box2dTestBed(scenario);
    if (test && test->init())
    {
        test->autorelease();
        return test;
    }

    CC_SAFE_DELETE(test);
    return nullptr;
}

Box2dTestBed::Box2dTestBed(Box2dTestBedScenario scenario)
    : _scenario(scenario)
    , _world(b2_nullWorldId)
    , _debugDraw(PixelsPerMeter)
    , _debugLabel(nullptr)
    , _debugEnabled(true)
{
}

Box2dTestBed::~Box2dTestBed()
{
    if (B2_IS_NON_NULL(_world))
    {
        b2DestroyWorld(_world);
    }
}

bool Box2dTestBed::init()
{
    if (!TestCase::init())
    {
        return false;
    }

    createWorld();

    // Keep tall testbed geometry from drawing through the CppTests title UI.
    const Rect visibleRect = VisibleRect::getVisibleRect();
    auto headerBackground = LayerColor::create(Color4B::BLACK, visibleRect.size.width, 105.0f);
    headerBackground->setPosition(visibleRect.origin.x, visibleRect.getMaxY() - 105.0f);
    addChild(headerBackground, 9998);

    createDebugButton();
    scheduleUpdate();
    return true;
}

std::string Box2dTestBed::title() const
{
    switch (_scenario)
    {
    case Box2dTestBedScenario::Pyramid:
        return "Box2D 3.1.1 - Pyramid";
    case Box2dTestBedScenario::Friction:
        return "Box2D 3.1.1 - Friction";
    case Box2dTestBedScenario::Restitution:
        return "Box2D 3.1.1 - Restitution";
    case Box2dTestBedScenario::Bridge:
        return "Box2D 3.1.1 - Bridge";
    case Box2dTestBedScenario::CapsuleStack:
        return "Box2D 3.1.1 - Capsule Stack";
    }
    return "Box2D 3.1.1";
}

std::string Box2dTestBed::subtitle() const
{
    switch (_scenario)
    {
    case Box2dTestBedScenario::Pyramid:
        return "A 15-row dynamic box stack";
    case Box2dTestBedScenario::Friction:
        return "Five blocks with friction from 0.75 to 0.0";
    case Box2dTestBedScenario::Restitution:
        return "Seven circles with restitution from 0.0 to 1.0";
    case Box2dTestBedScenario::Bridge:
        return "Thirty planks connected by revolute joints";
    case Box2dTestBedScenario::CapsuleStack:
        return "Twenty dynamic capsules";
    }
    return "";
}

void Box2dTestBed::createWorld()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -10.0f};
    _world = b2CreateWorld(&worldDef);
    b2World_EnableSleeping(_world, true);
    b2World_EnableContinuous(_world, true);

    switch (_scenario)
    {
    case Box2dTestBedScenario::Pyramid:
        createPyramid();
        break;
    case Box2dTestBedScenario::Friction:
        createFriction();
        break;
    case Box2dTestBedScenario::Restitution:
        createRestitution();
        break;
    case Box2dTestBedScenario::Bridge:
        createBridge();
        break;
    case Box2dTestBedScenario::CapsuleStack:
        createCapsuleStack();
        break;
    }
}

void Box2dTestBed::createGroundSegment(b2Vec2 point1, b2Vec2 point2, float friction)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = friction;
    const b2Segment segment = {point1, point2};
    b2CreateSegmentShape(ground, &shapeDef, &segment);
}

void Box2dTestBed::createPyramid()
{
    createGroundSegment({-20.0f, 0.0f}, {20.0f, 0.0f});

    const b2Polygon box = b2MakeRoundedBox(0.45f, 0.45f, 0.05f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;

    constexpr int RowCount = 15;
    for (int row = 0; row < RowCount; ++row)
    {
        for (int column = row; column < RowCount; ++column)
        {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = {
                static_cast<float>(row) * 0.5f + static_cast<float>(column - row) - 7.0f,
                0.5f + static_cast<float>(row),
            };
            b2BodyId body = b2CreateBody(_world, &bodyDef);
            b2CreatePolygonShape(body, &shapeDef, &box);
        }
    }
}

void Box2dTestBed::createFriction()
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId ground = b2CreateBody(_world, &bodyDef);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.material.friction = 0.2f;
    const b2Segment groundSegment = {{-40.0f, 0.0f}, {40.0f, 0.0f}};
    b2CreateSegmentShape(ground, &groundShapeDef, &groundSegment);

    b2Polygon platform = b2MakeOffsetBox(13.0f, 0.25f, {-4.0f, 22.0f}, b2MakeRot(-0.25f));
    b2CreatePolygonShape(ground, &groundShapeDef, &platform);
    platform = b2MakeOffsetBox(0.25f, 1.0f, {10.5f, 19.0f}, b2Rot_identity);
    b2CreatePolygonShape(ground, &groundShapeDef, &platform);
    platform = b2MakeOffsetBox(13.0f, 0.25f, {4.0f, 14.0f}, b2MakeRot(0.25f));
    b2CreatePolygonShape(ground, &groundShapeDef, &platform);
    platform = b2MakeOffsetBox(0.25f, 1.0f, {-10.5f, 11.0f}, b2Rot_identity);
    b2CreatePolygonShape(ground, &groundShapeDef, &platform);
    platform = b2MakeOffsetBox(13.0f, 0.25f, {-4.0f, 6.0f}, b2MakeRot(-0.25f));
    b2CreatePolygonShape(ground, &groundShapeDef, &platform);

    const b2Polygon box = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 25.0f;
    const float friction[] = {0.75f, 0.5f, 0.35f, 0.1f, 0.0f};
    for (int i = 0; i < 5; ++i)
    {
        b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
        dynamicBodyDef.type = b2_dynamicBody;
        dynamicBodyDef.position = {-8.0f + 4.0f * static_cast<float>(i), 28.0f};
        b2BodyId body = b2CreateBody(_world, &dynamicBodyDef);
        shapeDef.material.friction = friction[i];
        b2CreatePolygonShape(body, &shapeDef, &box);
    }
}

void Box2dTestBed::createRestitution()
{
    constexpr int CircleCount = 7;
    createGroundSegment({-10.0f, 0.0f}, {10.0f, 0.0f});

    const b2Circle circle = {{0.0f, 0.0f}, 0.65f};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;

    for (int i = 0; i < CircleCount; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {-6.0f + 2.0f * static_cast<float>(i), 20.0f};
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        shapeDef.material.restitution = static_cast<float>(i) / static_cast<float>(CircleCount - 1);
        b2CreateCircleShape(body, &shapeDef, &circle);
    }
}

void Box2dTestBed::createBridge()
{
    constexpr int PlankCount = 30;
    constexpr float PlankWidth = 0.8f;
    constexpr float StartX = -0.5f * PlankCount * PlankWidth;
    constexpr float BridgeY = 9.0f;

    b2BodyDef anchorDef = b2DefaultBodyDef();
    anchorDef.position = {StartX, BridgeY};
    b2BodyId leftAnchor = b2CreateBody(_world, &anchorDef);
    anchorDef.position = {StartX + PlankCount * PlankWidth, BridgeY};
    b2BodyId rightAnchor = b2CreateBody(_world, &anchorDef);

    const b2Polygon plank = b2MakeRoundedBox(0.4f, 0.12f, 0.03f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 20.0f;
    shapeDef.material.friction = 0.6f;

    b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
    jointDef.enableMotor = true;
    jointDef.maxMotorTorque = 20.0f;
    jointDef.enableSpring = true;
    jointDef.hertz = 2.0f;
    jointDef.dampingRatio = 0.7f;
    jointDef.drawSize = 0.2f;

    b2BodyId previous = leftAnchor;
    for (int i = 0; i < PlankCount; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {StartX + (static_cast<float>(i) + 0.5f) * PlankWidth, BridgeY};
        bodyDef.linearDamping = 0.1f;
        bodyDef.angularDamping = 0.1f;
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2CreatePolygonShape(body, &shapeDef, &plank);

        const b2Vec2 pivot = {StartX + static_cast<float>(i) * PlankWidth, BridgeY};
        jointDef.bodyIdA = previous;
        jointDef.bodyIdB = body;
        jointDef.localAnchorA = b2Body_GetLocalPoint(previous, pivot);
        jointDef.localAnchorB = b2Body_GetLocalPoint(body, pivot);
        b2CreateRevoluteJoint(_world, &jointDef);
        previous = body;
    }

    const b2Vec2 finalPivot = {StartX + PlankCount * PlankWidth, BridgeY};
    jointDef.bodyIdA = previous;
    jointDef.bodyIdB = rightAnchor;
    jointDef.localAnchorA = b2Body_GetLocalPoint(previous, finalPivot);
    jointDef.localAnchorB = b2Body_GetLocalPoint(rightAnchor, finalPivot);
    b2CreateRevoluteJoint(_world, &jointDef);

    const b2Circle circle = {{0.0f, 0.0f}, 0.65f};
    shapeDef.density = 10.0f;
    for (int i = 0; i < 5; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {-8.0f + 4.0f * static_cast<float>(i), 14.0f};
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2CreateCircleShape(body, &shapeDef, &circle);
    }
}

void Box2dTestBed::createCapsuleStack()
{
    createGroundSegment({-10.0f, 0.0f}, {10.0f, 0.0f});

    const b2Capsule capsule = {{-1.0f, 0.0f}, {1.0f, 0.0f}, 0.25f};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.5f;

    for (int i = 0; i < 20; ++i)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {0.15f * static_cast<float>(i % 2), 0.5f + 0.75f * static_cast<float>(i)};
        b2BodyId body = b2CreateBody(_world, &bodyDef);
        b2CreateCapsuleShape(body, &shapeDef, &capsule);
    }
}

void Box2dTestBed::createDebugButton()
{
    _debugLabel = Label::createWithTTF("Debug: ON", "fonts/arial.ttf", 18.0f);
    auto toggle = MenuItemLabel::create(_debugLabel, CC_CALLBACK_1(Box2dTestBed::toggleDebug, this));
    auto menu = Menu::create(toggle, nullptr);
    menu->setPosition(VisibleRect::rightTop().x - 65.0f, VisibleRect::rightTop().y - 85.0f);
    addChild(menu, 10000);
}

void Box2dTestBed::toggleDebug(Ref*)
{
    _debugEnabled = !_debugEnabled;
    _debugDraw.setEnabled(_debugEnabled);
    _debugLabel->setString(_debugEnabled ? "Debug: ON" : "Debug: OFF");
}

void Box2dTestBed::update(float)
{
    b2World_Step(_world, TimeStep, SubStepCount);
}

void Box2dTestBed::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
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

void Box2dTestBed::onDebugDraw(Mat4 transform)
{
    const float originY = VisibleRect::bottom().y + 55.0f;
    transform.translate(VisibleRect::center().x, originY, 0.0f);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    b2World_Draw(_world, _debugDraw.getDebugDraw());
    CHECK_GL_ERROR_DEBUG();
    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
