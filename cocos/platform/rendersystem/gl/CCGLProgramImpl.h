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

#ifndef __CC_EGLProgramIMPL_H__
#define __CC_EGLProgramIMPL_H__

#include "platform/CCGLViewObjects.h"
#include "CCGL.h"

NS_CC_BEGIN

class GLProgramImpl : public ProgramImpl
{
public:
	GLProgramImpl();
	~GLProgramImpl();
	
	// Specific query methods
	GLuint getProgramName() const { return mProgramName; }

	// 
	virtual bool recreate() override;
	
	// 
	virtual bool init(const std::string& vertexShaderName, const std::string& fragmentShaderName) override;
	
	// Methods to set uniform values
	virtual void setFloatUniform(unsigned int location, float f1) override;
	virtual void setFloatUniform(unsigned int location, float f1, float f2) override;
	virtual void setFloatUniform(unsigned int location, float f1, float f2, float f3) override;
	virtual void setFloatUniform(unsigned int location, float f1, float f2, float f3, float f4) override;
	virtual void setIntegerUniform(unsigned int location, int i1) override;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2) override;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2, int i3) override;
	virtual void setIntegerUniform(unsigned int location, int i1, int i2, int i3, int i4) override;
	virtual void setIntegerUniformArray(unsigned int location, const int* ints, int numElements, int numArrays = 1) override;
	virtual void setFloatUniformArray(unsigned int location, const float* floats, int numElements, int numArrays = 1) override;
	
protected:
	GLuint            mProgramName;
    GLuint            mVertShader;
    GLuint            mFragShader;
	
	virtual void parseVertexStructure() override;
    
	virtual void parseUniforms() override;
	
	// Specific methods for this implementation
	bool compileShader(GLuint* shader, GLenum type, const GLchar* source);
	bool link();
};

NS_CC_END

#endif