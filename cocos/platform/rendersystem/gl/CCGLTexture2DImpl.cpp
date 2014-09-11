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
#include "base/CCPlatformMacros.h"
#include "base/ccUtils.h"
#include "base/ccMacros.h"
#include "base/CCConfiguration.h"
#include "base/CCDirector.h"
#include "platform/CCImage.h"	// for Image and MipmapInfo

NS_CC_BEGIN

// Copy pixel format info from Texture2D, without guilt.
// Don't blame me, I really believe that this must be close the implementation.
// Anyway, I shall create one list for each implementation
namespace {
	struct PixelFormatInfo {

        PixelFormatInfo(GLenum anInternalFormat, GLenum aFormat, GLenum aType, int aBpp, bool aCompressed, bool anAlpha)
            : internalFormat(anInternalFormat)
            , format(aFormat)
            , type(aType)
            , bpp(aBpp)
            , compressed(aCompressed)
            , alpha(anAlpha)
        {}

        GLenum internalFormat;
        GLenum format;
        GLenum type;
        int bpp;
        bool compressed;
        bool alpha;
    };
    
    typedef std::map<Texture2D::PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;
    typedef PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
    static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
    {
        PixelFormatInfoMapValue(Texture2D::PixelFormat::BGRA8888, PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGBA8888, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGBA4444, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB5A1, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB565, PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB888, PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::A8, PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::I8, PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::AI88, PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
        
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC2, PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC2A, PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC4, PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC4A, PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, true)),
#endif
        
#ifdef GL_ETC1_RGB8_OES
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ETC, PixelFormatInfo(GL_ETC1_RGB8_OES, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT1, PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT3, PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT5, PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_ATC_RGB_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_RGB, PixelFormatInfo(GL_ATC_RGB_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_EXPLICIT_ALPHA, PixelFormatInfo(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_INTERPOLATED_ALPHA, PixelFormatInfo(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
    };

	//CLASS IMPLEMENTATIONS:

	//The PixpelFormat corresponding information
	const PixelFormatInfoMap _pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
		TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));
}

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Default is: RGBA8888 (32-bit textures)
static Texture2D::PixelFormat g_defaultAlphaPixelFormat = Texture2D::PixelFormat::DEFAULT;

GLTexture2DImpl::GLTexture2DImpl()
	: TextureImpl()
	, mTextureName(0)
	, mDepthStencilRenderbufferName(0)
	, mFramebufferName(0)
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

bool GLTexture2DImpl::recreate()
{
	// TODO implement

	return false;
}

bool GLTexture2DImpl::init(TextureImpl::TextureType type,
							Texture2D::PixelFormat format,
							unsigned int width,
							unsigned int height,
							MipmapInfo* mipmaps,
							unsigned int numMipLevels,
							unsigned int sampleCount,
							unsigned int slices)
{
	mType = type;
	mPixelFormat = format;
	mWidth = width;
	mHeight = height;
	mNumMips = numMipLevels;
	mNumSamples = sampleCount;
	mNumSlices = slices;
	
	// Pick pixel format
	if(_pixelFormatInfoTables.find(mPixelFormat) == _pixelFormatInfoTables.end())
    {
        CCLOG("cocos2d: WARNING: unsupported pixelformat: %lx", (unsigned long)mPixelFormat );
        return false;
    }
	const PixelFormatInfo& info = _pixelFormatInfoTables.at(mPixelFormat);
	
	// Check if compressed formats are supported
	if (info.compressed && !Configuration::getInstance()->supportsPVRTC()
                        && !Configuration::getInstance()->supportsETC()
                        && !Configuration::getInstance()->supportsS3TC()
                        && !Configuration::getInstance()->supportsATITC())
    {
        CCLOG("cocos2d: WARNING: PVRTC/ETC images are not supported");
        return false;
    }
	
	//Set the row align only when mipmapsNum == 1 and the data is uncompressed
    if (mNumMips == 1 && !info.compressed)
    {
        unsigned int bytesPerRow = mWidth * info.bpp / 8;

        if(bytesPerRow % 8 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
        }
        else if(bytesPerRow % 4 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        else if(bytesPerRow % 2 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
    }else
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
	
	// Create texture
	if (mTextureName != 0)
	{
		glDeleteTextures(1, &mTextureName);
		mTextureName = 0;
	}
	glGenTextures(1, &mTextureName);
	glBindTexture(GL_TEXTURE_2D, mTextureName);
	
	if (mNumMips == 1)
    {
        glTexParameteri(GL_TEXTURE_2D, 
						GL_TEXTURE_MIN_FILTER, 
						mAntialiasEnabled ? GL_LINEAR : GL_NEAREST);
    }
	else
    {
        glTexParameteri(GL_TEXTURE_2D, 
						GL_TEXTURE_MIN_FILTER, 
						mAntialiasEnabled ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
    }
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mAntialiasEnabled ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	CHECK_GL_ERROR_DEBUG(); // clean possible GL error
	
	// If mipmaps array is null, probably this is a render target texture,
	// so create a zeroed array and configure texture size
	void *data = nullptr;
	if (mipmaps == nullptr /* && mType == TextureType::RenderTarget*/)
	{
		// render target textures must be power-of-two squared
		int powW = (int)(mWidth * CC_CONTENT_SCALE_FACTOR());
		int powH = (int)(mHeight * CC_CONTENT_SCALE_FACTOR());

        if (!Configuration::getInstance()->supportsNPOT())
        {
			powW = ccNextPOT(powW);
			powH = ccNextPOT(powW);
        }

		mWidth = powW;
		mHeight = powH;
		
		// Create a empty mipmap data
		auto dataLen = mWidth * mHeight * 4;
		data = malloc(dataLen);
		if (data == nullptr)
		{
			return false;
		}

		memset(data, 0, dataLen);
		
		MipmapInfo mipmap;
		mipmap.address = static_cast<unsigned char*>(data);
		mipmap.len = dataLen;
		
		mipmaps == &mipmap;
	}
	
	// Load texture data
	width = mWidth;
	height = mHeight;
	for (int i = 0; i < mNumMips; ++i)
	{
		unsigned char *data = mipmaps[i].address;
		GLsizei datalen = mipmaps[i].len;

		if (info.compressed == true)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, 
									i, 
									info.internalFormat, 
									(GLsizei)width, 
									(GLsizei)height, 
									0, 
									datalen, 
									data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 
						 i, 
						 info.internalFormat, 
						 (GLsizei)width, 
						 (GLsizei)height, 
						 0, 
						 info.format, 
						 info.type, 
						 data);
		}

		if (i > 0 && (width != height || ccNextPOT(width) != width ))
		{
			CCLOG("cocos2d: Texture2D. WARNING. Mipmap level %u is not squared. Texture won't render correctly. width=%d != height=%d", i, width, height);
		}

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			CCLOG("cocos2d: Texture2D: Error uploading compressed texture level: %u . glError: 0x%04X", i, err);
			return false;
		}

		width = MAX(width >> 1, 1);
		height = MAX(height >> 1, 1);
	}
	
	CC_SAFE_FREE(data);
	
	// If texture type is render-target, create framebuffer and other objects
	if (mType == TextureType::RenderTarget)
	{
		return createFramebufferObjects(GL_DEPTH24_STENCIL8);
	}
	
	return true;
}


void GLTexture2DImpl::generateMips()
{
	glBindTexture(GL_TEXTURE_2D, mTextureName);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool GLTexture2DImpl::updateData(const void* data, 
								 unsigned int width, 
								 unsigned int height,
								 unsigned int offsetX, 
								 unsigned int offsetY,
								 unsigned int mipLevel, 
								 unsigned int slice)
{
	// Pick pixel format
	const PixelFormatInfo& info = _pixelFormatInfoTables.at(mPixelFormat);
	
	glBindTexture(GL_TEXTURE_2D, mTextureName);
	glTexSubImage2D(GL_TEXTURE_2D,	
					mipLevel, 
					offsetX, 
					offsetY, 
					mWidth, 
					mHeight, 
					info.format, 
					info.type, 
					data);
					
	return true;
}

bool GLTexture2DImpl::readData(void* data, 
								unsigned int width, 
								unsigned int height,
								unsigned int offsetX, 
								unsigned int offsetY,
								unsigned int mipLevel, 
								unsigned int slice)
{
	// Pick pixel format
	const PixelFormatInfo& info = _pixelFormatInfoTables.at(mPixelFormat);
	
	// TODO implement read data
	
	return false;
}

bool GLTexture2DImpl::createFramebufferObjects(GLint depthStencilFormat)
{
	// Store old framebuffer binding
	GLint oldFramebufferName = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebufferName);
	
	// Store old renderbuffer binding
	GLint oldRBO;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRBO);
	
	// generate FBO
	glGenFramebuffers(1, &mFramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferName);

	// associate texture with FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureName, 0);
	
	// Force depth stencil format
	// TODO Check if formats have any issue with any platform
	if (depthStencilFormat != 0)
	{
		//create and attach depth buffer
		glGenRenderbuffers(1, &mDepthStencilRenderbufferName);
		glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbufferName);
		glRenderbufferStorage(GL_RENDERBUFFER, depthStencilFormat, (GLsizei)mWidth, (GLsizei)mHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRenderbufferName);

		// if depth format is the one with stencil part, bind same render buffer as stencil attachment
		if (depthStencilFormat == GL_DEPTH24_STENCIL8)
		{
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRenderbufferName);
		}
	}
	
	// check if it worked (probably worth doing :) )
	CCASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Could not attach texture to framebuffer");
	
	// Restore old framebuffer and renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, oldRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFramebufferName);
	
	return true;
}

NS_CC_END