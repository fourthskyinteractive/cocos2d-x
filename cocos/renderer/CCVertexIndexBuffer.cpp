/****************************************************************************
 Copyright (c) 2013-2014 Chukong Technologies Inc.
 Copyright (c) 2014 Fourth Sky Interactive

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

#include "renderer/CCVertexIndexBuffer.h"
#include "base/CCEventType.h"
#include "base/CCEventListenerCustom.h"

NS_CC_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
bool VertexBuffer::_enableShadowCopy = true;
#else
bool VertexBuffer::_enableShadowCopy = false;
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
bool IndexBuffer::_enableShadowCopy = true;
#else
bool IndexBuffer::_enableShadowCopy = false;
#endif

VertexBuffer* VertexBuffer::create(int sizePerVertex, int vertexNumber, bool dynamic)
{
    auto result = new (std::nothrow) VertexBuffer();
    if(result && result->init(sizePerVertex, vertexNumber, dynamic))
    {
        result->autorelease();
        return result;
    }
    CC_SAFE_DELETE(result);
    return nullptr;
    
}

VertexBuffer::VertexBuffer()
: _vertexNumber(0)
, _sizePerVertex(0)
, _recreateVBOEventListener(nullptr)
, _dynamic(false)
, _vbo(0)
{
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    auto callBack = [this](EventCustom* event)
    {
        this->recreateVBO();
    };

    _recreateVBOEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(EVENT_RENDERER_RECREATED, callBack);

#endif
}

VertexBuffer::~VertexBuffer()
{
	releaseGLBuffer();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    Director::getInstance()->getEventDispatcher()->removeEventListener(_recreateVBOEventListener);
#endif
}

int VertexBuffer::getSizePerVertex() const
{
    return _sizePerVertex;
}

int VertexBuffer::getVertexNumber() const
{
    return _vertexNumber;
}

GLuint VertexBuffer::getVBO() const
{
    return _vbo;
}

int VertexBuffer::getSize() const
{
    return _sizePerVertex * _vertexNumber;
}

IndexBuffer* IndexBuffer::create(IndexType type, int number, bool dynamic)
{
    auto result = new (std::nothrow) IndexBuffer();
    if(result && result->init(type, number, dynamic))
    {
        result->autorelease();
        return result;
    }
    CC_SAFE_DELETE(result);
    return nullptr;
}

IndexBuffer::IndexBuffer()
: _vbo(0)
, _type(IndexType::INDEX_TYPE_SHORT_16)
, _indexNumber(0)
, _recreateVBOEventListener(nullptr)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    auto callBack = [this](EventCustom* event)
    {
        this->recreateVBO();
    };

    _recreateVBOEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(EVENT_RENDERER_RECREATED, callBack);
#endif
}

IndexBuffer::~IndexBuffer()
{
	releaseGLBuffer();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    Director::getInstance()->getEventDispatcher()->removeEventListener(_recreateVBOEventListener);
#endif
}

IndexBuffer::IndexType IndexBuffer::getType() const
{
    return _type;
}

int IndexBuffer::getSizePerIndex() const
{
    return IndexType::INDEX_TYPE_SHORT_16 == _type ? 2 : 4;
}

int IndexBuffer::getIndexNumber() const
{
    return _indexNumber;
}

int IndexBuffer::getSize() const
{
    return getSizePerIndex() * _indexNumber;
}

GLuint IndexBuffer::getVBO() const
{
    return _vbo;
}

NS_CC_END
