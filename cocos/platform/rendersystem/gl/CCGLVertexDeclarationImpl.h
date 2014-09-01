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

#ifndef __CC_EGLVertexDeclarationIMPL_H__
#define __CC_EGLVertexDeclarationIMPL_H__

#include "CCGLViewObjects.h"
#include "platform/rendersystem/gl/CCGLBufferImpl.h"

NS_CC_BEGIN

class GLVertexDeclarationImpl : public VertexDeclarationImpl
{
public:
	GLVertexDeclarationImpl();
	~GLVertexDeclarationImpl();
	
	// Specific query methods
	
	
	virtual bool init() override;
	
	virtual bool recreate() override;

	virtual void begin() override;
 
	virtual bool setStream(BufferImpl* buffer, 
						   int offset, 
						   int semantic, 
						   ElementType type, 
						   int stride, 
						   bool normalize = false) override;
	
	virtual void build() override;
	
protected:
	unsigned int mStreamIndex;
	
private:
	CC_DISALLOW_COPY_AND_ASSIGN(GLVertexDeclarationImpl);
};

class GLVertexDeclarationVAOImpl : public VertexDeclarationImpl
{
public:
	GLVertexDeclarationVAOImpl();
	~GLVertexDeclarationVAOImpl();
	
	// Specific query methods
	GLuint getVertexArray() const { return mVertexArrayObject; }
	
	virtual bool init() override;
	
	virtual bool recreate() override;

	virtual void begin() override;
 
	virtual bool setStream(BufferImpl* buffer, 
						   int offset, 
						   int semantic, 
						   ElementType type, 
						   int stride, 
						   bool normalize = false) override;
	
	virtual void build() override;
	
protected:
	mutable GLuint mVertexArrayObject;
	unsigned int mStreamIndex;
	
private:
	CC_DISALLOW_COPY_AND_ASSIGN(GLVertexDeclarationVAOImpl);
};

NS_CC_END

#endif