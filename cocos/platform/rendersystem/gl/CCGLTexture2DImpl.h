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

#ifndef __CC_EGLTexture2DIMPL_H__
#define __CC_EGLTexture2DIMPL_H__

#include "CCGLViewObjects.h"

NS_CC_BEGIN

class GLTexture2DImpl : public Texture2DImpl
{
public:
	GLVertexDeclarationImpl();
	~GLVertexDeclarationImpl();
	
	// Specific query methods
	virtual unsigned int getWidthInBytes() const override { return mWidth; }
	virtual unsigned int getHeightInBytes() const override { return mHeight; }
	virtual unsigned int getDepthInBytes() const override { return mDepth; }
	virtual unsigned int getNumMipLevels() const override { return mNumMips; }
	virtual unsigned int getNumSamples() const override { return mNumSamples; }
	virtual unsigned int getNumSlices() const override { return mNumSlices; }
	
	GLuint getTextureName() const { return mTextureName; }
	GLuint getDepthStencilRenderbufferName const { return mDepthStencilRenderbufferName; }
	GLuint getFramebufferName() const { return mFramebufferName; }
	
	// Init texture
	virtual bool init(TextureImpl::TextureType type, 
					   Texture2D::PixelFormat format,
					   int width, 
					   int height, 
					   int depth = 0, 
					   int numMipLevels = 0, 
					   int sampleCount = 1,
					   int slices = 1) override;
	
	// 
	virtual void generateMips() override;
	
	// 
	virtual void updateData(const void* data) override;
		
	// 
	virtual void updateData(const void* data, int start, int dataSize, int mipLevel = 0, int slice = 0) override;
	
	// 
	virtual void readData(void* data) override;
	
	// 
	virtual void readData(void* data, int start, int dataSize, int mipLevel = 0, int slice = 0) override;
	
protected:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDepth;
	unsigned int mNumMips;
	unsigned int mNumSamples;
	unsigned int mNumSlices;
	
	GLuint mTextureName;
	GLuint mDepthStencilRenderbufferName;
	GLuint mFramebufferName;
	GLuint mOldFramebufferName;
	
};

NS_CC_END

#endif