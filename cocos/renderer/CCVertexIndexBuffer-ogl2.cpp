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

void* VertexBuffer::map()
{
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, getSize(), nullptr, _access);
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void VertexBuffer::unmap()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::releaseGLBuffer()
{
	if (glIsBuffer(_vbo))
	{
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
}

bool VertexBuffer::init(int sizePerVertex, int vertexNumber, bool dynamic)
{
	if (0 == sizePerVertex || 0 == vertexNumber)
		return false;
	_sizePerVertex = sizePerVertex;
	_vertexNumber = vertexNumber;
	_dynamic = dynamic;
	_access = _dynamic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	if (isShadowCopyEnabled())
	{
		_shadowCopy.resize(sizePerVertex * _vertexNumber);
	}

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, getSize(), nullptr, _access);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	return true;
}

bool VertexBuffer::updateVertices(const void* verts, int count, int begin)
{
	if (count <= 0 || nullptr == verts) return false;

	if (begin < 0)
	{
		CCLOGERROR("Update vertices with begin = %d, will set begin to 0", begin);
		begin = 0;
	}

	if (count + begin > _vertexNumber)
	{
		CCLOGERROR("updated vertices exceed the max size of vertex buffer, will set count to _vertexNumber-begin");
		count = _vertexNumber - begin;
	}

	if (isShadowCopyEnabled())
	{
		memcpy(&_shadowCopy[begin * _sizePerVertex], verts, count * _sizePerVertex);
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferSubData(GL_ARRAY_BUFFER, begin * _sizePerVertex, count * _sizePerVertex, verts);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	return true;
}

void VertexBuffer::recreateVBO() const
{
	CCLOG("come to foreground of VertexBuffer");
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	const void* buffer = nullptr;
	if (isShadowCopyEnabled())
	{
		buffer = &_shadowCopy[0];
	}
	CCLOG("recreate IndexBuffer with size %d %d", getSizePerVertex(), _vertexNumber);
	glBufferData(GL_ARRAY_BUFFER, _sizePerVertex * _vertexNumber, buffer, _access);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	if (!glIsBuffer(_vbo))
	{
		CCLOGERROR("recreate VertexBuffer Error");
	}
}

void* IndexBuffer::map()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, getSize(), nullptr, _access);
	return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void IndexBuffer::unmap()
{
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::releaseGLBuffer()
{
	if (glIsBuffer(_vbo))
	{
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
}

bool IndexBuffer::init(IndexBuffer::IndexType type, int number, bool dynamic)
{
	if (number <= 0) return false;

	_type = type;
	_indexNumber = number;
	_dynamic = dynamic;
	_access = _dynamic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, getSize(), nullptr, _access);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	if (isShadowCopyEnabled())
	{
		_shadowCopy.resize(getSize());
	}

	return true;
}

bool IndexBuffer::updateIndices(const void* indices, int count, int begin)
{
	if (count <= 0 || nullptr == indices) return false;

	if (begin < 0)
	{
		CCLOGERROR("Update indices with begin = %d, will set begin to 0", begin);
		begin = 0;
	}

	if (count + begin > _indexNumber)
	{
		CCLOGERROR("updated indices exceed the max size of vertex buffer, will set count to _indexNumber-begin");
		count = _indexNumber - begin;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, begin * getSizePerIndex(), count * getSizePerIndex(), indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	if (isShadowCopyEnabled())
	{
		memcpy(&_shadowCopy[begin * getSizePerIndex()], indices, count * getSizePerIndex());
	}

	return true;
}

void IndexBuffer::recreateVBO() const
{
	CCLOG("come to foreground of IndexBuffer");
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo);
	const void* buffer = nullptr;
	if (isShadowCopyEnabled())
	{
		buffer = &_shadowCopy[0];
	}
	CCLOG("recreate IndexBuffer with size %d %d ", getSizePerIndex(), _indexNumber);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, getSize(), buffer, _access);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();

	if (!glIsBuffer(_vbo))
	{
		CCLOGERROR("recreate IndexBuffer Error");
	}
}


NS_CC_END