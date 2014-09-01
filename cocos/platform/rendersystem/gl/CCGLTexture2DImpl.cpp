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

#include "platform/rendersystem/gl/CCGLTexture2DImpl.h"

NS_CC_BEGIN

GLTexture2DImpl::GLTexture2DImpl()
	: Texture2DImpl()
	, mWidth(0)
	, mHeight(0)
	, mDepth(0)
	, mNumMips(0)
	, mNumSamples(0)
	, mNumSlices(0)
	, mTextureName(0)
	, mDepthStencilRenderbufferName(0)
	, mFramebufferName(0)
	, mOldFramebufferName(0)
{
	
}

GLTexture2DImpl::~GLTexture2DImpl()
{
	if (mFramebufferName != 0)
	{
		glDeleteFramebuffers(1, &mFramebufferName);
		mFramebufferName = 0;
	}
	
	if (mTextureName != 0)
	{
		glDeleteTextures(1, &mTextureName);
		mTextureName = 0;
	}
}

bool GLTexture2DImpl::init(TextureImpl::TextureType type, 
						   Texture2D::PixelFormat format,
						   int width, int height, int depth, 
						   int numMipLevels, int sampleCount, 
						   int slices)
{
	_type = type;
	_pixelFormat = format;
	mWidth = width;
	mHeight = height;
	mDepth = depth;
	mNumMips = numMipLevels;
	mNumSamples = sampleCount;
	mNumSlices = slices;
	
	// Create texture
	glGenTextures(1, &mTextureName);
	
	
}


void GLTexture2DImpl::generateMips()
{
	glBindTexture2D(mTextureName);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture2D(0);
}

void GLTexture2DImpl::updateData(const void* data)
{
	
}

void GLTexture2DImpl::updateData(const void* data, int start, int dataSize, int mipLevel, int slice)
{
	
}

void GLTexture2DImpl::readData(void* data)
{
	
}

void GLTexture2DImpl::readData(void* data, int start, int dataSize, int mipLevel, int slice)
{
	
}

NS_CC_END