/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.
Copyright (c) 2014 Evandro Paulino

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

#include "platform/rendersystem/gl/CCGLBufferImpl.h"

NS_CC_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
bool GLBufferImpl::_enableShadowCopy = true;
#else
bool GLBufferImpl::_enableShadowCopy = false;
#endif

GLBufferImpl::GLBufferImpl()
	: BufferImpl()
	, mBufferName(0)
	, mBufferTarget(0)
	, mBufferAccess(0)
{
	
}

GLBufferImpl::~GLBufferImpl()
{
	if (mBufferName != 0)
	{
		glDeleteBuffers(1, &mBufferName);
		mBufferName = 0;
	}
}

bool GLBufferImpl::init(BufferImpl::BufferType type, unsigned int sizeInBytes, bool dynamic)
{
	if(0 == sizeInBytes)
        return false;
	
	// Record size in bytes
	mSizeInBytes = sizeInBytes;
	
	// Set type of buffer...
	mType = type;
	
	// ... and configure target 
	switch(type)
	{
	case BufferType::Vertex:
		mBufferTarget = GL_ARRAY_BUFFER;
		break;
	case BufferType::Index_16bits:
	case BufferType::Index_32bits:
		mBufferTarget = GL_ELEMENT_ARRAY_BUFFER;
		break;
	case BufferType::Constant:
		mBufferTarget = GL_UNIFORM_BUFFER_EXT;
		break;
	/*
	case BufferType::ShaderResource:
		mBufferTarget = GL_SHADER_STORAGE_BUFFER;
		break;
	*/
	default:
		break;
	}
	
	// Configure access
	mDynamic = dynamic;
	mBufferAccess = mDynamic == true ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	
	// Allocate space for shadow copy
	if(isShadowCopyEnabled())
    {
        _shadowCopy.resize(getSizeInBytes());
    }
	
	// Create buffer
	return recreate();
}

//
bool GLBufferImpl::recreate()
{
	glGenBuffers(1, &mBufferName);
	glBindBuffer(mBufferTarget, mBufferName);
	
	if (GLBufferImpl::_enableShadowCopy == true && _shadowCopy.size() > 0)
	{
		glBufferData(mBufferTarget, getSizeInBytes(), &_shadowCopy[0], mBufferAccess);
	}
	else
	{
		glBufferData(mBufferTarget, getSizeInBytes(), nullptr, mBufferAccess);
	}
	glBindBuffer(mBufferTarget, 0);
	
	return true;
}

//
void* GLBufferImpl::map()
{
	if (mDynamic)
	{
		// Orphaning
		glBindBuffer(mBufferTarget, mBufferName);
		glBufferData(mBufferTarget, getSizeInBytes(), nullptr, mBufferAccess);
	}
	
	return glMapBuffer(mBufferTarget, GL_WRITE_ONLY);
}

// 
void GLBufferImpl::unmap()
{
	glUnmapBuffer(mBufferTarget);
	glBindBuffer(mBufferTarget, 0);
}

// 
bool GLBufferImpl::updateData(const void* data, unsigned int start, unsigned int dataSize)
{
	if(dataSize <= 0 || nullptr == data) 
		return false;
    
    if(start < 0)
    {
        CCLOGERROR("Update vertices with begin = %d, will set begin to 0", start);
        start = 0;
    }
    
    if(start + dataSize > getSizeInBytes())
    {
        CCLOGERROR("updated vertices exceed the max size of vertex buffer, will set count to size in bytes - start");
        dataSize = getSizeInBytes() - start;
    }
    
    if(isShadowCopyEnabled())
    {
        memcpy(&_shadowCopy[start], data, dataSize);
    }
	
	glBindBuffer(mBufferTarget, mBufferName);
	glBufferSubData(mBufferTarget, start, dataSize, data);
	glBindBuffer(mBufferTarget, 0);
	
	return true;
}

NS_CC_END