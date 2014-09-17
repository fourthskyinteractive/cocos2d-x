/****************************************************************************
Copyright (c) 2008      Apple Inc. All Rights Reserved.
Copyright (c) 2010-2012 cocos2d-x.org
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

#include "renderer/CCFrameBuffer.h"

#include "base/ccUtils.h"
#include "base/CCConfiguration.h"
#include "base/CCDirector.h"

#include "CCGL.h"

NS_CC_BEGIN

FrameBuffer::FrameBuffer()
: _colorTexture(nullptr)
, _depthStencilTexture(nullptr)
, _resolveTexture(nullptr)
, _FBO(0)
, _oldFBO(0)
, _textureCopy(nullptr)
, _isQCOM(false)
{

}

FrameBuffer::~FrameBuffer()
{
	CC_SAFE_RELEASE(_colorTexture);
	CC_SAFE_RELEASE(_depthStencilTexture);
	CC_SAFE_RELEASE(_resolveTexture);
}

void FrameBuffer::releaseGLObjects()
{

}

FrameBuffer* FrameBuffer::create(int width, int height, Texture2D::PixelFormat format, GLenum depthStencilFormat, int samples)
{
	FrameBuffer *ret = new FrameBuffer();
	if (ret && ret->init(width, height, format, depthStencilFormat, samples))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool FrameBuffer::init(int width, int height, Texture2D::PixelFormat format, GLenum depthStencilFormat, int samples)
{
	CCASSERT(format != Texture2D::PixelFormat::A8, "only RGB and RGBA formats are valid for a render texture");

	/*  Certain Qualcomm Andreno gpu's will retain data in memory after a 
		frame buffer switch which corrupts the render to the texture. 
		The solution is to clear the frame buffer before rendering to the texture. 
		However, calling glClear has the unintended result of clearing the current texture. 
		Create a temporary texture to overcome this. At the end of RenderTexture::begin(), 
		switch the attached texture to the second one, call glClear, and then switch back 
		to the original texture. This solution is unnecessary for other devices as they don't 
		have the same issue with switching frame buffers.
	*/
	_isQCOM = Configuration::getInstance()->checkForGLExtension("GL_QCOM");

	bool ret = false;
	void *data = nullptr;
	do {
		width = (int)(width * CC_CONTENT_SCALE_FACTOR());
		height = (int)(height * CC_CONTENT_SCALE_FACTOR());

		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
		GLint oldRBO;
		glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRBO);

		// textures must be power of two squared
		int powW = 0;
		int powH = 0;

		if (Configuration::getInstance()->supportsNPOT())
		{
			powW = width;
			powH = height;
		}
		else
		{
			powW = ccNextPOT(width);
			powH = ccNextPOT(height);
		}

		auto dataLen = powW * powH * 4;
		data = malloc(dataLen);
		CC_BREAK_IF(!data);

		memset(data, 0, dataLen);
		_pixelFormat = format;

		_colorTexture = new (std::nothrow) Texture2D();
		if (_colorTexture)
		{
			_colorTexture->initWithData(data, dataLen, (Texture2D::PixelFormat)_pixelFormat, powW, powH, Size((float)width, (float)height));
		}
		else
		{
			break;
		}

		if (_isQCOM)
		{
			_textureCopy = new Texture2D();
			if (_textureCopy)
			{
				_textureCopy->initWithData(data, dataLen, (Texture2D::PixelFormat)_pixelFormat, powW, powH, Size((float)width, (float)height));
			}
			else
			{
				break;
			}
		}

		// generate FBO
		glGenFramebuffers(1, &_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

		// associate texture with FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture->getName(), 0);

		if (depthStencilFormat != 0)
		{
			//create and attach depth buffer
			glGenRenderbuffers(1, &_depthRenderBufffer);
			glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBufffer);
			glRenderbufferStorage(GL_RENDERBUFFER, depthStencilFormat, (GLsizei)powW, (GLsizei)powH);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBufffer);

			// if depth format is the one with stencil part, bind same render buffer as stencil attachment
			if (depthStencilFormat == GL_DEPTH24_STENCIL8)
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBufffer);
			}
		}

		// check if it worked (probably worth doing :) )
		CCASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Could not attach texture to framebuffer");

		glBindRenderbuffer(GL_RENDERBUFFER, oldRBO);
		glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);

	} while (0);

	CC_SAFE_FREE(data);

	return true;
}

GLint FrameBuffer::getFBO() const
{
	return _FBO;
}

void FrameBuffer::reset()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);

	glGenFramebuffers(1, &_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	_colorTexture->setAliasTexParameters();

	if (_textureCopy)
	{
		_textureCopy->setAliasTexParameters();
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture->getName(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
}

void FrameBuffer::bind()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	if (_isQCOM)
	{
		// -- bind a temporary texture so we can clear the render buffer without losing our texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureCopy->getName(), 0);
		CHECK_GL_ERROR_DEBUG();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture->getName(), 0);
		CHECK_GL_ERROR_DEBUG();
	}
}

void FrameBuffer::restore()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
}

bool FrameBuffer::download(void* buffer)
{
	if (buffer == nullptr)
		return false;

	const Size& s = _colorTexture->getContentSizeInPixels();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	if (_isQCOM)
	{
		// -- bind a temporary texture so we can clear the render buffer without losing our texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureCopy->getName(), 0);
		CHECK_GL_ERROR_DEBUG();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture->getName(), 0);
	}
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, s.width, s.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);

	CHECK_GL_ERROR_DEBUG();

	return true;
}

void FrameBuffer::discard()
{

}

void FrameBuffer::resolve()
{

}

NS_CC_END