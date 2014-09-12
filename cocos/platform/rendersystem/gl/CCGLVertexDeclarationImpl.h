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

#include "platform/CCGLViewObjects.h"
#include "CCGL.h"

NS_CC_BEGIN

// Forward declarations
class GLBufferImpl;

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
						   ElementSemantic semantic,
						   int offset, 
						   int semantic, 
						   ElementDataType type,
						   int stride, 
						   bool normalize = false) override;
	
	virtual void end() override;

	virtual void bind();

	virtual void unbind();
	
protected:
	// Attribute to enable and disable vertex attribs
	uint32_t s_attributeFlags = 0;  // 32 attributes max

	typedef struct StreamElement {
		ElementSemantic semantic;
		GLuint index;
		GLint size;
		GLenum type;
		GLsizei stride;
		GLboolean normalized;
		GLuint offset;
		GLBufferImpl* buffer;
	};
	std::vector<StreamElement> mElementList;
	
private:
	CC_DISALLOW_COPY_AND_ASSIGN(GLVertexDeclarationImpl);
};

class GLVertexDeclarationVAOImpl : public GLVertexDeclarationImpl
{
public:
	GLVertexDeclarationVAOImpl();
	~GLVertexDeclarationVAOImpl();
	
	// Specific query methods
	
	
	virtual bool init() override;
	
	virtual bool recreate() override;

	virtual void begin() override;

	virtual bool setStream(BufferImpl* buffer,
							ElementSemantic	semantic,
							int offset,
							int semantic,
							ElementDataType type,
							int stride,
							bool normalize = false) override;

	virtual void end() override;

	virtual void bind() override;

	virtual void unbind() override;
	
protected:
	GLuint mVertexArrayObject;
	
private:
	CC_DISALLOW_COPY_AND_ASSIGN(GLVertexDeclarationVAOImpl);
};

NS_CC_END

#endif