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
#include "renderer/CCTexture2D.h"

NS_CC_BEGIN

// Forward declarations
class Image;
typedef struct _MipmapInfo MipmapInfo;



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
	BufferImpl() : mSizeInBytes(0), mType(BufferType::Vertex), mDynamic(false) { }
	virtual ~BufferImpl() { }
	
	// Query methods
	BufferType getType() const { return mType; }
	unsigned int getSizeInBytes() const { return mSizeInBytes; }
	
	// Init buffer
	virtual bool init(BufferImpl::BufferType type, unsigned int sizeInBytes, bool dynamic = false) = 0;
	
	// Give access of buffer contents to CPU
	virtual void* map() = 0;
	
	// Unlock buffer and return access to GPU
	virtual void unmap() = 0;
	
	// Update buffer data
	virtual bool updateData(const void* data) { return this->updateData(data, 0, getSizeInBytes()); }
	
	// Update buffer data with specific amount of data
	virtual bool updateData(const void* data, unsigned int start, unsigned int dataSize) = 0;
	
protected:
	static bool _enableShadowCopy;
	
	// Buffer type
	mutable BufferType mType;
	
	// Size of the buffer
	mutable unsigned int mSizeInBytes;
	
	// Is this buffer dynamic modified?
	bool mDynamic;
	
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
	TextureImpl() : mType(TextureType::ShaderResource), mPixelFormat(Texture2D::PixelFormat::AUTO),
					mWidth(0), mHeight(0), mNumMips(0), mNumSamples(1),
					mNumSlices(1) { }
	virtual ~TextureImpl() { }
	
	// Query methods
	TextureType getType() const { return mType; }
	Texture2D::PixelFormat getPixelFormat() const { return mPixelFormat; }
	unsigned int getWidth() const { return mWidth; }
	unsigned int getHeight() const { return mHeight; }
	unsigned int getNumMipLevels() const { return mNumMips; }
	unsigned int getNumSamples() const  { return mNumSamples; }
	unsigned int getNumSlices() const { return mNumSlices; }
	
	// Init texture
	virtual bool init(TextureImpl::TextureType type, 
					   Texture2D::PixelFormat format,
					   unsigned int width, 
					   unsigned int height, 
					   MipmapInfo* mipmaps = nullptr,
					   unsigned int numMipLevels = 1, 
					   unsigned int sampleCount = 1,
					   unsigned int slices = 1) = 0;
	
	// 
	virtual void generateMips() = 0;
	
	// 
	virtual bool updateData(const void* data)
	{
		return this->updateData(data, getWidth(), getHeight());
	}
		
	// 
	virtual bool updateData(const void* data, 
							unsigned int width, 
							unsigned int height,
							unsigned int offsetX = 0, 
							unsigned int offsetY = 0,
							unsigned int mipLevel = 0, 
							unsigned int slice = 0) = 0;
	
	// 
	virtual bool readData(void* data)
	{
		return this->readData(data, getWidth(), getHeight());
	}
	
	// 
	virtual bool readData(void* data, 
						  unsigned int width, 
						  unsigned int height,
						  unsigned int offsetX = 0, 
						  unsigned int offsetY = 0,
						  unsigned int mipLevel = 0, 
						  unsigned int slice = 0) = 0;
	
protected:
	mutable TextureType mType;
	mutable Texture2D::PixelFormat mPixelFormat;
	
	mutable unsigned int mWidth;
	mutable unsigned int mHeight;
	mutable unsigned int mNumMips;
	mutable unsigned int mNumSamples;
	mutable unsigned int mNumSlices;
	bool mAntialiasEnabled;
	
};

class VertexDeclarationImpl : public ResourceImpl
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
	virtual void end() = 0;
};

class ProgramImpl : public Ref
{
public:
	// Info about uniforms will be available with tuples
	typedef std::tuple</*location*/unsigned int, 
					   /*name*/std::string, 
					   /*element type*/ElementType, 
					   /*num elements*/unsigned int, 
					   /*array size*/unsigned int> UniformInfo;
	typedef std::vector<UniformInfo> UniformInfoList;

	// Query methods
	const UniformInfoList& getLocationInfo() const { return mLocationInfo; }
	unsigned int getNumLocations() const { return mLocationInfo.size(); }
	
	// Initialize program with shader code
	virtual bool init(const std::string& vertexShaderName, const std::string& fragmentShaderName) = 0;
	
	// Methods to set uniform values
	virtual void setFloatUniform(unsigned int location, float f1) = 0;
	virtual void setFloatUniform(unsigned int location, float f1, float f2) = 0;
	virtual void setFloatUniform(unsigned int location, float f1, float f2, float f3) = 0;
	virtual void setFloatUniform(unsigned int location, float f1, float f2, float f3, float f4) = 0;
	virtual void setIntegerUniform(unsigned int location, int i1) = 0;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2) = 0;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2, int i3) = 0;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2, int i3, int i4) = 0;
	virtual void setIntegerUniformArray(unsigned int location, const int* ints, int numElements, int numArrays = 1) = 0;
	virtual void setFloatUniformArray(unsigned int location, const float* floats, int numElements, int numArrays = 1) = 0;
								 
protected:
	mutable UniformInfoList mLocationInfo;
	
	// Parse the input vertex structure
	virtual void parseVertexStructure() = 0;
    
	// Parse the uniforms for shaders
	virtual void parseUniforms() = 0;
};

NS_CC_END

#endif /* __CCGLVIEWOBJECTS_H__ */