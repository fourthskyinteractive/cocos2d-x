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

#ifndef __CC_EGLBufferIMPL_DESKTOP_H__
#define __CC_EGLBufferIMPL_DESKTOP_H__

#include "CCGLViewObjects.h"

NS_CC_BEGIN

class GLBufferImpl : public BufferImpl
{
public:
	
	GLBufferImpl();
	~GLBufferImpl();
	
	// Specific query methods
	GLuint getBufferName() const { return mBuffer; }	
	
	// 
	virtual bool init(BufferImpl::BufferType type, unsigned int sizeInBytes, bool dynamic = false);
	
	//
	virtual bool recreate() const
	
	//
	virtual unsigned int getSizeInBytes() const;

	//
	virtual void* map();
	
	// 
	virtual void unmap();
	
	// 
	virtual void updateData(const void* data, int start, int dataSize);
	
protected;
	mutable GLuint mBuffer;
	GLenum mBufferTarget
	GLenum mBufferAccess;
	
	mutable unsigned int mSizeInBytes;
	bool mDynamic;
};

NS_CC_END

#endif
