/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "renderer/CCVertexIndexData.h"
#include "base/CCConfiguration.h"


NS_CC_BEGIN

void VertexData::use()
{
	if (Configuration::getInstance()->supportsShareableVAO())
	{
		if (_VAO == 0)
		{
			glGenVertexArrays(1, &_VAO);
			glBindVertexArray(_VAO);

			for (auto& element : _vertexStreams)
			{
				glEnableVertexAttribArray((GLint)element.second._stream._semantic);
				glBindBuffer(GL_ARRAY_BUFFER, element.second._buffer->getVBO());
				glVertexAttribPointer(GLint(element.second._stream._semantic),
										element.second._stream._size,
										element.second._stream._type,
										element.second._stream._normalize,
										element.second._buffer->getSizePerVertex(),
										(GLvoid*)element.second._stream._offset);
			}

			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		GL::bindVAO(_VAO);

		CHECK_GL_ERROR_DEBUG();
	}
	else
	{ 
		uint32_t flags(0);
		for (auto& element : _vertexStreams)
		{
			flags = flags | (1 << element.second._stream._semantic);
		}

		GL::enableVertexAttribs(flags);

		for (auto& element : _vertexStreams)
		{
			glBindBuffer(GL_ARRAY_BUFFER, element.second._buffer->getVBO());
			glVertexAttribPointer(GLint(element.second._stream._semantic),
									element.second._stream._size,
									element.second._stream._type,
									element.second._stream._normalize,
									element.second._buffer->getSizePerVertex(),
									(GLvoid*)element.second._stream._offset);
		}

		CHECK_GL_ERROR_DEBUG();
	}
}

void VertexData::disable()
{
	// Only for 
	if (Configuration::getInstance()->supportsShareableVAO())
	{
		GL::bindVAO(0);
	}
}

NS_CC_END