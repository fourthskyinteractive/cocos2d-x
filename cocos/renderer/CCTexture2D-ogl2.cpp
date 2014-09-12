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

#include "renderer/CCTexture2D.h"

#include "CCGL.h"
#include "platform/CCImage.h"
#include "base/ccUtils.h"
#include "base/ccConfig.h"
#include "base/ccMacros.h"
#include "base/CCConfiguration.h"
#include "base/CCPlatformMacros.h"
#include "renderer/ccGLStateCache.h"

#include "deprecated/CCString.h"


#if CC_ENABLE_CACHE_TEXTURE_DATA
#include "renderer/CCTextureCache.h"
#endif

NS_CC_BEGIN

void Texture2D::releaseGLTexture()
{
	if (_name)
	{
		glDeleteTextures(1, &_name);
	}
	_name = 0;
}

bool Texture2D::_initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, Texture2D::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{
	const PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

	if (info.compressed && !Configuration::getInstance()->supportsPVRTC()
		&& !Configuration::getInstance()->supportsETC()
		&& !Configuration::getInstance()->supportsS3TC()
		&& !Configuration::getInstance()->supportsATITC())
	{
		CCLOG("cocos2d: WARNING: PVRTC/ETC images are not supported");
		return false;
	}

	// Create a new GL texture and bind
	glGenTextures(1, &_name);
	bind(0);

	//Set the row align only when mipmapsNum == 1 and the data is uncompressed
	if (mipmapsNum == 1 && !info.compressed)
	{
		unsigned int bytesPerRow = pixelsWide * info.bpp / 8;

		if (bytesPerRow % 8 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		}
		else if (bytesPerRow % 4 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		else if (bytesPerRow % 2 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		}
		else
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	if (mipmapsNum == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if CC_ENABLE_CACHE_TEXTURE_DATA
	if (_antialiasEnabled)
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR), GL_LINEAR, GL_NONE, GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
	else
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST), GL_NEAREST, GL_NONE, GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
#endif

	CHECK_GL_ERROR_DEBUG(); // clean possible GL error

	// Specify OpenGL texture image
	int width = pixelsWide;
	int height = pixelsHigh;

	for (int i = 0; i < mipmapsNum; ++i)
	{
		unsigned char *data = mipmaps[i].address;
		GLsizei datalen = mipmaps[i].len;

		if (info.compressed)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
		}

		if (i > 0 && (width != height || ccNextPOT(width) != width))
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

	return true;
}

bool Texture2D::updateWithData(const void *data, int offsetX, int offsetY, int width, int height)
{
	if (_name)
	{
		bind();
		const PixelFormatInfo& info = _pixelFormatInfoTables.at(_pixelFormat);
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, info.format, info.type, data);

		return true;
	}
	return false;
}

//
// Use to apply MIN/MAG filter
//
// implementation Texture2D (GLFilter)

void Texture2D::generateMipmap()
{
	CCASSERT(_pixelsWide == ccNextPOT(_pixelsWide) && _pixelsHigh == ccNextPOT(_pixelsHigh), "Mipmap texture only works in POT textures");
	bind(0);
	glGenerateMipmap(GL_TEXTURE_2D);
	_hasMipmaps = true;
#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::setHasMipmaps(this, _hasMipmaps);
#endif
}

void Texture2D::bind(GLint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, _name);
}

void Texture2D::unbind(GLint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::setTexParameters(const TexParams &texParams)
{
	CCASSERT((_pixelsWide == ccNextPOT(_pixelsWide) || texParams.wrapS == GL_CLAMP_TO_EDGE) &&
		(_pixelsHigh == ccNextPOT(_pixelsHigh) || texParams.wrapT == GL_CLAMP_TO_EDGE),
		"GL_CLAMP_TO_EDGE should be used in NPOT dimensions");

	bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);

#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

void Texture2D::setAliasTexParameters()
{
	if (!_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = false;

	if (_name == 0)
	{
		return;
	}

	bind(0);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	TexParams texParams = { (GLuint)(_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST), GL_NEAREST, GL_NONE, GL_NONE };
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

void Texture2D::setAntiAliasTexParameters()
{
	if (_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = true;

	if (_name == 0)
	{
		return;
	}

	bind(0);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR), GL_LINEAR, GL_NONE, GL_NONE };
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

NS_CC_END