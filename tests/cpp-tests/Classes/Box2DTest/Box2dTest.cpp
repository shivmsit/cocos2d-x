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

#include "Box2dTest.h"
#include "../testResource.h"
#include "extensions/cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;

namespace
{
constexpr float PTMRatio = 32.0f;
constexpr int ParentNodeTag = 1;
}

Box2DTests::Box2DTests()
{
    ADD_TEST_CASE(Box2DTest);
}

Box2DTest::Box2DTest()
    : _spriteTexture(nullptr)
    , _debugLabel(nullptr)
    , _debugNode(nullptr)
    , _debugDraw(PTMRatio)
    , _debugEnabled(true)
    , _world(b2_nullWorldId)
{
}

Box2DTest::~Box2DTest()
{
    if (B2_IS_NON_NULL(_world))
    {
        b2DestroyWorld(_world);
    }
}

bool Box2DTest::init()
{
    if (!TestCase::init())
    {
        return false;
    }

    auto touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesEnded = CC_CALLBACK_2(Box2DTest::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    initPhysics();
    _debugNode = DrawNode::create();
    _debugDraw.setDrawNode(_debugNode);
    addChild(_debugNode, 5);
    createResetButton();
    createDebugButton();

    auto parent = SpriteBatchNode::create("Images/blocks.png", 100);
    _spriteTexture = parent->getTexture();
    addChild(parent, 0, ParentNodeTag);

    addNewSpriteAtPosition(VisibleRect::center());

    auto label = Label::createWithTTF("Tap screen", "fonts/Marker Felt.ttf", 32.0f);
    label->setColor(Color3B::BLUE);
    label->setPosition(VisibleRect::center().x, VisibleRect::top().y - 50.0f);
    addChild(label);

    scheduleUpdate();
    return true;
}

void Box2DTest::initPhysics()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -10.0f};
    _world = b2CreateWorld(&worldDef);
    b2World_EnableSleeping(_world, true);
    b2World_EnableContinuous(_world, true);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId groundBody = b2CreateBody(_world, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();

    const b2Vec2 leftBottom = {VisibleRect::leftBottom().x / PTMRatio, VisibleRect::leftBottom().y / PTMRatio};
    const b2Vec2 rightBottom = {VisibleRect::rightBottom().x / PTMRatio, VisibleRect::rightBottom().y / PTMRatio};
    const b2Vec2 leftTop = {VisibleRect::leftTop().x / PTMRatio, VisibleRect::leftTop().y / PTMRatio};
    const b2Vec2 rightTop = {VisibleRect::rightTop().x / PTMRatio, VisibleRect::rightTop().y / PTMRatio};

    const b2Segment boundaries[] = {
        {leftBottom, rightBottom},
        {leftTop, rightTop},
        {leftTop, leftBottom},
        {rightBottom, rightTop},
    };
    for (const b2Segment& boundary : boundaries)
    {
        b2CreateSegmentShape(groundBody, &shapeDef, &boundary);
    }
}

void Box2DTest::createResetButton()
{
    auto reset = MenuItemImage::create("Images/r1.png", "Images/r2.png", [this](Ref*) {
        getTestSuite()->restartCurrTest();
    });
    auto menu = Menu::create(reset, nullptr);
    menu->setPosition(VisibleRect::bottom().x, VisibleRect::bottom().y + 30.0f);
    addChild(menu, 10);
}

void Box2DTest::createDebugButton()
{
    _debugLabel = Label::createWithTTF("Debug: ON", "fonts/arial.ttf", 18.0f);
    auto toggle = MenuItemLabel::create(_debugLabel, CC_CALLBACK_1(Box2DTest::toggleDebugCallback, this));
    auto menu = Menu::create(toggle, nullptr);
    menu->setPosition(VisibleRect::rightTop().x - 65.0f, VisibleRect::rightTop().y - 28.0f);
    addChild(menu, 10);
}

void Box2DTest::toggleDebugCallback(Ref*)
{
    _debugEnabled = !_debugEnabled;
    _debugDraw.setEnabled(_debugEnabled);
    _debugLabel->setString(_debugEnabled ? "Debug: ON" : "Debug: OFF");
    refreshDebugDraw();
}

void Box2DTest::addNewSpriteAtPosition(Vec2 position)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {position.x / PTMRatio, position.y / PTMRatio};
    b2BodyId body = b2CreateBody(_world, &bodyDef);

    b2Polygon box = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;
    b2CreatePolygonShape(body, &shapeDef, &box);

    const int x = CCRANDOM_0_1() > 0.5f ? 0 : 1;
    const int y = CCRANDOM_0_1() > 0.5f ? 0 : 1;
    auto sprite = PhysicsSprite::createWithTexture(_spriteTexture, Rect(32 * x, 32 * y, 32, 32));
    sprite->setB2Body(body);
    sprite->setPTMRatio(PTMRatio);
    sprite->setPosition(position);
    getChildByTag(ParentNodeTag)->addChild(sprite);
}

void Box2DTest::update(float dt)
{
    b2World_Step(_world, dt, 4);
    refreshDebugDraw();
}

void Box2DTest::refreshDebugDraw()
{
    _debugDraw.drawWorld(_world);
}

void Box2DTest::onTouchesEnded(const std::vector<Touch*>& touches, Event*)
{
    for (Touch* touch : touches)
    {
        if (touch)
        {
            addNewSpriteAtPosition(touch->getLocation());
        }
    }
}
