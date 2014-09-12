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
#include "platform/rendersystem/gl/CCGLBufferImpl.h"
#include "platform/desktop/CCGLViewImpl.h"
#include "platform/rendersystem/gl/CCGLRenderSystem.h"

NS_CC_BEGIN

GLVertexDeclarationImpl::GLVertexDeclarationImpl()
	: VertexDeclarationImpl()
{
	
}

GLVertexDeclarationImpl::~GLVertexDeclarationImpl()
{
	// Release buffers
	for (auto el : mElementList)
	{
		el.buffer->release();
	}

	mElementList.clear();
}

bool GLVertexDeclarationImpl::init()
{
	return recreate();
}

bool GLVertexDeclarationImpl::recreate()
{
	return true;

	return false;
}

void GLVertexDeclarationImpl::begin()
{
	// Restart element list
	mElementList.clear();
}

bool GLVertexDeclarationImpl::setStream(BufferImpl* buffer, 
										ElementSemantic semantic,
									    int offset, 
									    int semantic, 
										ElementDataType type,
									    int stride, 
									    bool normalize)
{
	// Define buffer for this stream
	if (buffer == nullptr)
	{
		return false;
	}

	// Parse type and size
	GLint glSize = 4;
	GLenum glType = GL_FLOAT;
	GLViewImpl::parseDataType(type, glSize, glType);

	// Get semantic index
	const AttributeLocation* loc = std::find_if(attribute_locations,
												attribute_locations + attribute_locations_size,
												[semantic](AttributeLocation al) { return al.semantic == semantic; });

	// Record element
	StreamElement el;
	el.semantic = semantic;
	el.index = loc->location;
	el.size = glSize;
	el.type = glType;
	el.stride = stride;
	el.normalized = normalize;
	el.offset = offset;
	el.buffer = static_cast<GLBufferImpl*>(buffer);
	mElementList.push_back(el);

	// Retain buffer to prevent deletion
	el.buffer->retain();

	return true;
}

void GLVertexDeclarationImpl::end()
{
	// Nothing to do
}

void GLVertexDeclarationImpl::bind()
{
	for (auto el : mElementList)
	{
		glBindBuffer(el.buffer->getBufferTarget(), el.buffer->getBufferName());
		glEnableVertexAttribArray(el.index);
		glVertexAttribPointer(el.index, el.size, el.type, el.normalized, el.stride, (GLvoid*)el.offset);
	}
}

void GLVertexDeclarationImpl::unbind()
{
	// Unbind all buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable vertex attrib
	for (auto el : mElementList)
	{
		glDisableVertexAttribArray(el.index);
	}
}

/**************************************************************/

GLVertexDeclarationVAOImpl::GLVertexDeclarationVAOImpl()
	: GLVertexDeclarationImpl()
	, mVertexArrayObject(0)
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
	if (mVertexArrayObject != 0)
	{
		glDeleteVertexArrays(1, &mVertexArrayObject);
		mVertexArrayObject = 0;
	}

	glGenVertexArrays(1, &mVertexArrayObject);

	return true;
}

void GLVertexDeclarationVAOImpl::begin()
{
	// Bind vertex array
	glBindVertexArray(mVertexArrayObject);
}

bool GLVertexDeclarationVAOImpl::setStream(BufferImpl* buffer, 
										   ElementSemantic semantic,
										   int offset, 
										   int semantic, 
										   ElementDataType type,
										   int stride, 
										   bool normalize)
{
	// Store in the element list
	if (!GLVertexDeclarationImpl::setStream(buffer, semantic, offset, type, stride, normalize))
		return false;
	
	GLBufferImpl* glBuffer = static_cast<GLBufferImpl*>(buffer);
	glBindBuffer(glBuffer->getBufferTarget(), glBuffer->getBufferName());
	
	// Parse type and size
	GLint glSize = 4;
	GLenum glType = GL_FLOAT;
	GLViewImpl::parseDataType(type, glSize, glType);

	// Get semantic index
	const AttributeLocation* loc = std::find_if(attribute_locations,
												attribute_locations + attribute_locations_size,
												[semantic](AttributeLocation al) { return al.semantic == semantic; });

	// Set vertex attribute
	glVertexAttribPointer(loc->location,
							glSize,
							glType,
							normalize,
							stride,
							(GLvoid*) offset);
	
	return true;
}

void GLVertexDeclarationVAOImpl::end()
{
	// Just unbind vertex array, configuration is already copyed
	glBindVertexArray(0);
}

void GLVertexDeclarationVAOImpl::bind()
{
	// Bind vertex array
	glBindVertexArray(mVertexArrayObject);
}

void GLVertexDeclarationVAOImpl::unbind()
{
	glBindVertexArray(0);
}

NS_CC_END