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

#include "platform/CCGLViewObjects.h"
#include "CCGL.h"

NS_CC_BEGIN

class GLTexture2DImpl : public TextureImpl
{
public:
	GLTexture2DImpl();
	~GLTexture2DImpl();
	
	// Specific query methods
	GLuint getTextureName() const { return mTextureName; }
	GLuint getDepthStencilRenderbufferName() const { return mDepthStencilRenderbufferName; }
	GLuint getFramebufferName() const { return mFramebufferName; }

	// 
	virtual bool recreate() override;
	
	// Init texture
	virtual bool init(TextureImpl::TextureType type, 
					   Texture2D::PixelFormat format,
					   unsigned int width, 
					   unsigned int height, 
					   MipmapInfo* mipmaps = nullptr,
					   unsigned int numMipLevels = 1, 
					   unsigned int sampleCount = 1,
					   unsigned int slices = 1) override;
	
	// 
	virtual void generateMips() override;
		
	// 
	virtual bool updateData(const void* data, 
							unsigned int width, 
							unsigned int height,
							unsigned int offsetX = 0, 
							unsigned int offsetY = 0,
							unsigned int mipLevel = 0, 
							unsigned int slice = 0) override;
	
	// 
	virtual bool readData(void* data, 
						  unsigned int width, 
						  unsigned int height,
						  unsigned int offsetX = 0, 
						  unsigned int offsetY = 0,
						  unsigned int mipLevel = 0, 
						  unsigned int slice = 0) override;
	
protected:
	GLuint mTextureName;
	GLuint mDepthStencilRenderbufferName;
	GLuint mFramebufferName;
	
	bool createFramebufferObjects(GLint depthStencilFormat);
};

NS_CC_END

#endif