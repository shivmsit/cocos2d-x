/****************************************************************************
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#ifndef __CCCOLLIDERDETECTOR_H__
#define __CCCOLLIDERDETECTOR_H__

#include "editor-support/cocostudio/CCArmatureDefine.h"
#include "editor-support/cocostudio/CCDatas.h"
#include "editor-support/cocostudio/CocosStudioExport.h"

#ifndef PT_RATIO
#define PT_RATIO 32
#endif


#if ENABLE_PHYSICS_CHIPMUNK_DETECT
#include "chipmunk/chipmunk.h"
#elif ENABLE_PHYSICS_BOX2D_DETECT
#include "box2d/box2d.h"
#endif


namespace cocostudio {

class Bone;

/**
 *  @js NA
 *  @lua NA
 */
class CC_STUDIO_DLL ColliderFilter
{
public:
    virtual ~ColliderFilter() { }
#if ENABLE_PHYSICS_BOX2D_DETECT
public:
    ColliderFilter(uint64_t categoryBits = 0x0001, uint64_t maskBits = 0xFFFF, int groupIndex = 0);
    void updateShape(b2ShapeId shape);

    virtual void setCategoryBits(uint64_t categoryBits) { _categoryBits = categoryBits; }
    virtual uint64_t getCategoryBits() const { return _categoryBits; }

    virtual void setMaskBits(uint64_t maskBits) { _maskBits = maskBits; }
    virtual uint64_t getMaskBits() const { return _maskBits; }

    virtual void setGroupIndex(int groupIndex) { _groupIndex = groupIndex; }
    virtual int getGroupIndex() const { return _groupIndex; }
protected:
    uint64_t _categoryBits;
    uint64_t _maskBits;
    int _groupIndex;
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
public:
    ColliderFilter(cpCollisionType collisionType = 0, cpGroup group = 0);
    void updateShape(cpShape *shape);

    virtual void setCollisionType(cpCollisionType collisionType) { _collisionType = collisionType; }
    virtual cpCollisionType getCollisionType() const { return _collisionType; }

    virtual void setGroup(cpGroup group) { _group = group; }
    virtual cpGroup getGroup() const { return _group; }
protected:
    cpCollisionType _collisionType;
    cpGroup _group;
#endif
};

class CC_STUDIO_DLL ColliderBody : public cocos2d::Ref
{
public:
    ColliderBody(ContourData *contourData);
    ~ColliderBody();

    inline ContourData *getContourData() { return _contourData; }

#if ENABLE_PHYSICS_BOX2D_DETECT || ENABLE_PHYSICS_CHIPMUNK_DETECT 
    void setColliderFilter(ColliderFilter *filter);
    ColliderFilter *getColliderFilter();
#endif

#if ENABLE_PHYSICS_BOX2D_DETECT
    virtual void setB2Fixture(b2ShapeId fixture) { _fixture = fixture; }
    virtual b2ShapeId getB2Fixture() const { return _fixture; }
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
    virtual void setShape(cpShape *shape) { _shape = shape; }
    virtual cpShape *getShape() const { return _shape; }
#elif ENABLE_PHYSICS_SAVE_CALCULATED_VERTEX
    virtual const std::vector<cocos2d::Vec2> &getCalculatedVertexList() const { return _calculatedVertexList; }
#endif

private:

#if ENABLE_PHYSICS_BOX2D_DETECT
    b2ShapeId _fixture;
    ColliderFilter *_filter;
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
    cpShape *_shape;
    ColliderFilter *_filter;
#endif

#if ENABLE_PHYSICS_SAVE_CALCULATED_VERTEX
    std::vector<cocos2d::Vec2> _calculatedVertexList;
#endif

    ContourData *_contourData;

    friend class ColliderDetector;
};

/*
 *  @brief  ContourSprite used to draw the contour of the display
 *  @js NA
 *  @lua NA
 */
class CC_STUDIO_DLL ColliderDetector : public cocos2d::Ref
{
public:
    static ColliderDetector *create();
    static ColliderDetector *create(Bone *bone);
public:
    /**
     * @js ctor
     */
    ColliderDetector();
    /**
     * @js NA
     * @lua NA
     */
    ~ColliderDetector();

    virtual bool init();
    virtual bool init(Bone *bone);

    void addContourData(ContourData *contourData);
    void addContourDataList(cocos2d::Vector<ContourData*> &contourDataList);

    void removeContourData(ContourData *contourData);
    void removeAll();

    void updateTransform(cocos2d::Mat4 &t);

    void setActive(bool active);
    bool getActive();

    const cocos2d::Vector<ColliderBody*>& getColliderBodyList();

#if ENABLE_PHYSICS_BOX2D_DETECT || ENABLE_PHYSICS_CHIPMUNK_DETECT 
    virtual void setColliderFilter(ColliderFilter *filter);
    virtual ColliderFilter *getColliderFilter();
#endif

    virtual void setBone(Bone *bone) { _bone = bone; }
    virtual Bone *getBone() const { return _bone; }

#if ENABLE_PHYSICS_BOX2D_DETECT
    virtual void setBody(b2BodyId body);
    virtual b2BodyId getBody() const;
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
    virtual void setBody(cpBody *body);
    virtual cpBody *getBody() const;
#endif
 protected:
    cocos2d::Vector<ColliderBody*> _colliderBodyList;

    Bone *_bone;

#if ENABLE_PHYSICS_BOX2D_DETECT
    b2BodyId _body;
    ColliderFilter *_filter;
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
    cpBody *_body;
    ColliderFilter *_filter;
#endif

protected:
    bool _active;
};

}

#endif /*__CCCOLLIDERDETECTOR_H__*/
