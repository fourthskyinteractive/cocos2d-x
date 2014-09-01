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

#include "platform/rendersystem/gl/CCGLVertexDeclarationImpl.h"

GLVertexDeclarationImpl::GLVertexDeclarationImpl()
	: VertexDeclarationImpl()
	, mStreamIndex(0)
{
	
}

GLVertexDeclarationImpl::~GLVertexDeclarationImpl()
{
	
}

bool GLVertexDeclarationImpl::init()
{
	return recreate();
}

bool GLVertexDeclarationImpl::recreate()
{
	
}

void GLVertexDeclarationImpl::begin() override
{
	mStreamIndex = 0;
	
}

bool GLVertexDeclarationImpl::setStream(BufferImpl* buffer, 
									    int offset, 
									    int semantic, 
									    ElementType type, 
									    int stride, 
									    bool normalize) override
{
	
	return false;
}

void GLVertexDeclarationImpl::build() override
{
	
}

/**************************************************************/

GLVertexDeclarationVAOImpl::GLVertexDeclarationVAOImpl()
	: mVertexArrayObject(0)
	, mStreamIndex(0)
{
	
}

GLVertexDeclarationVAOImpl::~GLVertexDeclarationVAOImpl()
{
	if (mVertexArrayObject != 0)
	{
		glDeleteVertexArrays(1, &mVertexArrayObject);
		mVertexArrayObject = 0;
	}
}

bool GLVertexDeclarationVAOImpl::init()
{
	return recreate();
}

bool GLVertexDeclarationVAOImpl::recreate()
{
	glGenVertexArrays(1, &mVertexArrayObject);
}

void GLVertexDeclarationVAOImpl::begin() override
{
	mStreamIndex = 0;

	// 
	glBindVertexArray(mVertexArrayObject);
}

bool GLVertexDeclarationVAOImpl::setStream(BufferImpl* buffer, 
					   int offset, 
					   int semantic, 
					   ElementType type, 
					   int stride, 
					   bool normalize) override
{
	// Define buffer for this stream
	glBindBuffer(buffer->getTarget(), buffer->getBufferName());
	
	// Define vertex attrib pointer
	GLint glSize = 4;
	GLenum glType = GL_FLOAT;
	switch (type)
	{
	case Byte:
		glSize = 1;
		glType = GL_BYTE;
		break;
	case UnsignedByte:
		glSize = 1;
		glType = GL_UNSIGNED_BYTE;
		break;
	case Short:
		glSize = 2;
		glType = GL_BYTE;
		break;
	case UnsignedShort:
		glSize = 2;
		glType = GL_UNSIGNED_SHORT;
		break;
	case Integer:
		glSize = 4;
		glType = GL_BYTE;
		break;
	case UnsignedInteger:
		glSize = 4;
		glType = GL_UNSIGNED_INT;
		break;
	case Float:
		glSize = 4;
		glType = GL_FLOAT;
		break;
	}
	glVertexAttribPointer(mStreamIndex,
						  glSize,
						  glType,
						  normalize,
						  stride,
						  (GLvoid*) offset);
						  
	// Advance control variables
	mStreamIndex++;
}

void GLVertexDeclarationVAOImpl::build() override
{
	// Just unbind vertex array, configuration is already copyed
	glBindVertexArray(0);
}