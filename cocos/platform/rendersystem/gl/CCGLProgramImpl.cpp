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

#include "platform/rendersystem/gl/CCGLProgramImpl.h"
#include "platform/rendersystem/gl/CCGLRenderSystem.h"
#include "platform/CCFileUtils.h"

NS_CC_BEGIN

GLProgramImpl::GLProgramImpl()
: mProgramName(0)
	, mVertShader(0)
	, mFragShader(0)
{
	
}

GLProgramImpl::~GLProgramImpl()
{
	if (mVertShader)
    {
        glDeleteShader(mVertShader);
		mVertShader = 0;
    }
    
    if (mFragShader)
    {
        glDeleteShader(mFragShader);
		mFragShader = 0;
    }
    
	if (mProgramName)
    {
		glDeleteProgram(mProgramName);
		mProgramName = 0;
    }
}

bool GLProgramImpl::recreate()
{
	// TODO implement

	return false;
}

bool GLProgramImpl::init(const std::string& vertexShaderName, const std::string& fragmentShaderName)
{
	auto fileUtils = FileUtils::getInstance();
	std::string vertexSource = fileUtils->getStringFromFile(FileUtils::getInstance()->fullPathForFilename(vertexShaderName));
	std::string fragmentSource = fileUtils->getStringFromFile(FileUtils::getInstance()->fullPathForFilename(fragmentShaderName));
	
	// Create program
	mProgramName = glCreateProgram();
    CHECK_GL_ERROR_DEBUG();
	
	if (vertexSource.length() > 0)
    {
        if (!compileShader(&mVertShader, GL_VERTEX_SHADER, vertexSource.c_str()))
        {
            CCLOG("cocos2d: ERROR: Failed to compile vertex shader");
            return false;
       }
    }

    // Create and compile fragment shader
    if (fragmentSource.length() > 0)
    {
        if (!compileShader(&mFragShader, GL_FRAGMENT_SHADER, fragmentSource.c_str()))
        {
            CCLOG("cocos2d: ERROR: Failed to compile fragment shader");
            return false;
        }
    }
	
	// Attach shaders to program
	if (mVertShader)
    {
		glAttachShader(mProgramName, mVertShader);
    }
    CHECK_GL_ERROR_DEBUG();

    if (mFragShader)
    {
		glAttachShader(mProgramName, mFragShader);
    }
    
    CHECK_GL_ERROR_DEBUG();

	CCASSERT(mProgramName != 0, "Cannot link invalid program");

	// Parse vertex structure
	parseVertexStructure();

	// Parse shader constants
	parseUniforms();

	return link();
}

void GLProgramImpl::setFloatUniform(unsigned int location, float f1)
{
	
}

void GLProgramImpl::setFloatUniform(unsigned int location, float f1, float f2)
{
	
}

void GLProgramImpl::setFloatUniform(unsigned int location, float f1, float f2, float f3)
{
	
}

void GLProgramImpl::setFloatUniform(unsigned int location, float f1, float f2, float f3, float f4)
{
	
}

void GLProgramImpl::setIntegerUniform(unsigned int location, int i1)
{
	
}

void GLProgramImpl::setIntegerUniform(unsigned int location, int i1, int i2)
{
	
}

void GLProgramImpl::setIntegerUniform(unsigned int location, int i1, int i2, int i3)
{
	
}

void GLProgramImpl::setIntegerUniform(unsigned int location, int i1, int i2, int i3, int i4)
{
	
}

void GLProgramImpl::setIntegerUniformArray(unsigned int location, const int* ints, int numElements, int numArrays)
{
	
}

void GLProgramImpl::setFloatUniformArray(unsigned int location, const float* floats, int numElements, int numArrays)
{
	
}

void GLProgramImpl::parseVertexStructure()
{

}

void GLProgramImpl::parseUniforms()
{
	mLocationInfo.clear();
	
	// Query and store uniforms from the program.
	GLint activeUniforms;
	glGetProgramiv(mProgramName, GL_ACTIVE_UNIFORMS, &activeUniforms);
	if(activeUniforms > 0)
	{
        GLint length;
		glGetProgramiv(mProgramName, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
		if(length > 0)
		{
			GLchar* uniformName = (GLchar*)alloca(length + 1);

			for(int i = 0; i < activeUniforms; ++i)
			{
				
			}
		}
	}
}

bool GLProgramImpl::compileShader(GLuint* shader, GLenum type, const GLchar* source)
{
    if (source == nullptr)
    {
        return false;
    }
	
	const GLchar *sources[] = {
        (type == GL_VERTEX_SHADER ? "precision highp float;\n" : "precision mediump float;\n"),
        "uniform mat4 CC_PMatrix;\n"
        "uniform mat4 CC_MVMatrix;\n"
        "uniform mat4 CC_MVPMatrix;\n"
        "uniform vec4 CC_Time;\n"
        "uniform vec4 CC_SinTime;\n"
        "uniform vec4 CC_CosTime;\n"
        "uniform vec4 CC_Random01;\n"
        "uniform sampler2D CC_Texture0;\n"
        "uniform sampler2D CC_Texture1;\n"
        "uniform sampler2D CC_Texture2;\n"
        "uniform sampler2D CC_Texture3;\n"
        "//CC INCLUDES END\n\n",
		source,
    };
	
	*shader = glCreateShader(type);
	glShaderSource(*shader, sizeof(sources)/sizeof(*sources), sources, nullptr);
    glCompileShader(*shader);
	
	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	
	if (! status)
    {
		GLsizei length;
		glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
		//GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);
		std::unique_ptr<GLchar[]> src((GLchar*)malloc(sizeof(GLchar) * length));

		glGetShaderSource(*shader, length, nullptr, src.get());
		CCLOG("cocos2d: ERROR: Failed to compile shader:\n%s", src.get());

		/*
		if (type == GL_VERTEX_SHADER)
		{
			CCLOG("cocos2d: %s", getVertexShaderLog().c_str());
		}
		else
		{
			CCLOG("cocos2d: %s", getFragmentShaderLog().c_str());
		}
		*/

		return false;;
    }
	
	return (status == GL_TRUE);
}

void GLProgramImpl::bindPredefinedVertexAttribs()
{
	for (int i = 0; i<attribute_locations_size; i++) {
		glBindAttribLocation(mProgramName, attribute_locations[i].location, attribute_locations[i].attributeName);
	}
}

bool GLProgramImpl::link()
{	
	// Link program
	glLinkProgram(mProgramName);

	// Delete shaders, don't have use anymore
	if (mVertShader)
	{
		glDeleteShader(mVertShader);
		mVertShader = 0;
	}

	if (mFragShader)
	{
		glDeleteShader(mFragShader);
		mFragShader = 0;
	}

	// Check for errors
	GLint status = GL_TRUE;
	glGetProgramiv(mProgramName, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		CCLOG("cocos2d: ERROR: Failed to link program: %i", mProgramName);
		glDeleteProgram(mProgramName);
		mProgramName = 0;
	}

	return (status == GL_TRUE);
}

NS_CC_END