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

#ifndef __CC_EGLBufferIMPL_H__
#define __CC_EGLBufferIMPL_H__

#include "CCGLViewObjects.h"

NS_CC_BEGIN

class GLBufferImpl : public BufferImpl
{
public:
	
	GLBufferImpl();
	~GLBufferImpl();
	
	// Specific query methods
	GLuint getBufferName() const { return mBufferName; }
	GLenum getBufferTarget() const { return mBufferTarget; }
	
	// 
	virtual bool init(BufferImpl::BufferType type, unsigned int sizeInBytes, bool dynamic = false) override;
	
	//
	virtual bool recreate() override;

	//
	virtual void* map() override;
	
	// 
	virtual void unmap() override;
	
	// 
	virtual bool updateData(const void* data, unsigned int start, unsigned int dataSize) override;
	
protected;
	mutable GLuint mBufferName;
	mutable GLenum mBufferTarget
	GLenum mBufferAccess;
	
private:
	CC_DISALLOW_COPY_AND_ASSIGN(GLBufferImpl);
};

NS_CC_END

#endif
