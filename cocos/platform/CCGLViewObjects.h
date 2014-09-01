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

#ifndef __CCGLVIEWOBJECTS_H__
#define __CCGLVIEWOBJECTS_H__

#include "base/ccTypes.h"
#include <vector>

NS_CC_BEGIN

class ResourceImpl : public Ref
{
public:
	// 
	virtual bool recreate() = 0;
	
};

class BufferImpl : public ResourceImpl
{
public:
	enum class BufferType
	{
		Vertex,
		Index_16bits,
		Index_32bits,
		Constant,
		ShaderResource
	};
	
	// Constructor and destructor
	BufferImpl() : mSizeInBytes(0), _type(Vertex) { }
	virtual ~BufferImpl() { }
	
	// Query methods
	TextureType getType() const { return _type; }
	virtual unsigned int getSizeInBytes() const = 0;
	
	// Init buffer
	virtual bool init(BufferImpl::BufferType type, unsigned int sizeInBytes, bool dynamic = false) = 0;
	
	// Give access of buffer contents to CPU
	virtual void* map() = 0;
	
	// Unlock buffer and return access to GPU
	virtual void unmap() = 0;
	
	// Update buffer data
	virtual void updateData(const void* data) { this->updateData(data, 0, getSizeInBytes()); }
	
	// Update buffer data with specific amount of data
	virtual void updateData(const void* data, int start, int dataSize) = 0;
	
protected:
	static bool _enableShadowCopy;
	
	// Buffer type
	mutable BufferType _type;
	
	// Shadow copy for resource contents
	std::vector<unsigned char> _shadowCopy;
	
public:
	static bool isShadowCopyEnabled() { return _enableShadowCopy; }
    static void enableShadowCopy(bool enabled) { _enableShadowCopy = enabled; }
};

class TextureImpl : public ResourceImpl
{
public:
	enum class TextureType
	{
		ShaderResource,
		Constant,
		RenderTarget,
		DepthTarget,
		StencilTarget
	};
	
	// Constructor and destructor
	TextureImpl() : _type(ShaderResource), _pixelFormat(PixelFormat::AUTO) { }
	virtual ~TextureImpl() { }
	
	// Query methods
	TextureType getType() const { return _type; }
	Texture2D::PixelFormat getPixelFormat() const { return _pixelFormat; }
	virtual unsigned int getWidthInBytes() const = 0;
	virtual unsigned int getHeightInBytes() const = 0;
	virtual unsigned int getDepthInBytes() const = 0;
	virtual unsigned int getNumMipLevels() const = 0;	
	virtual unsigned int getNumSamples() const = 0;	
	virtual unsigned int getNumSlices() const = 0;
	
	// Init texture
	virtual bool init(TextureImpl::TextureType type, 
					   Texture2D::PixelFormat format,
					   int width, 
					   int height, 
					   int depth = 0, 
					   int numMipLevels = 0, 
					   int sampleCount = 1,
					   int slices = 1) = 0;
	
	// 
	virtual void generateMips() = 0;
	
	// 
	virtual void updateData(const void* data) = 0;
		
	// 
	virtual void updateData(const void* data, int start, int dataSize, int mipLevel = 0, int slice = 0) = 0;
	
	// 
	virtual void readData(void* data) = 0;
	
	// 
	virtual void readData(void* data, int start, int dataSize, int mipLevel = 0, int slice = 0) = 0;
	
protected:
	mutable TextureType _type;
	mutable Texture2D::PixelFormat _pixelFormat;
	
};

class VertexDeclarationImpl : public Ref
{
public:
	// 
	virtual bool init() = 0;
	
	//
	virtual void begin() = 0;

	// 
	virtual bool setStream(BufferImpl* buffer, 
						   int offset, 
						   int semantic, 
						   ElementType type, 
						   int stride, 
						   bool normalize) = 0;
	
	// 
	virtual void build() = 0;
};

class ProgramImpl : public Ref
{
public:
	// 
	virtual void build() = 0;
	
	// Get location of the constant. 
	// If OpenGL, is the location within OpenGL program.
	// In other render systems, is the position in buffer pointer
	virtual unsigned int getLocationForName(const char* constantName) const = 0;
	
	// Set uniform using constant name
	virtual unsigned int setUniformData(const char* constantName, const void* pData, unsigned int dataSize) = 0;
	
	// Set uniform using location position
	virtual unsigned int setUniformData(unsigned int location, const void* pData, unsigned int dataSize) = 0;
};

NS_CC_END

#endif /* __CCGLVIEWOBJECTS_H__ */