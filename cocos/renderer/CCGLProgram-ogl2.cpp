/****************************************************************************
Copyright 2011 Jeff Lamarche
Copyright 2012 Goffredo Marocchi
Copyright 2012 Ricardo Quesada
Copyright 2012 cocos2d-x.org
Copyright 2013-2014 Chukong Technologies Inc.
Copyright 2014 Fourth Sky Interactive

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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN false EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "renderer/CCGLProgram.h"

#ifndef WIN32
#include <alloca.h>
#endif

#include "base/CCDirector.h"
#include "base/ccMacros.h"
#include "base/uthash.h"
#include "renderer/ccGLStateCache.h"
#include "CCGL.h"

#include "deprecated/CCString.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#include "CCPrecompiledShaders.h"
#endif

NS_CC_BEGIN

void GLProgram::releaseGLProgram()
{
	if (_vertShader)
	{
		glDeleteShader(_vertShader);
	}

	if (_fragShader)
	{
		glDeleteShader(_fragShader);
	}

	_vertShader = _fragShader = 0;

	if (_program)
	{
		glDeleteProgram(_program);
	}

	_program = 0;
}

bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray)
{

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	GLboolean hasCompiler = false;
	glGetBooleanv(GL_SHADER_COMPILER, &hasCompiler);
	_hasShaderCompiler = (hasCompiler == GL_TRUE);

	if (!_hasShaderCompiler)
	{
		return initWithPrecompiledProgramByteArray(vShaderByteArray, fShaderByteArray);
	}
#endif

	_program = glCreateProgram();
	CHECK_GL_ERROR_DEBUG();

	_vertShader = _fragShader = 0;

	if (vShaderByteArray)
	{
		if (!compileShader(&_vertShader, GL_VERTEX_SHADER, vShaderByteArray))
		{
			CCLOG("cocos2d: ERROR: Failed to compile vertex shader");
			return false;
		}
	}

	// Create and compile fragment shader
	if (fShaderByteArray)
	{
		if (!compileShader(&_fragShader, GL_FRAGMENT_SHADER, fShaderByteArray))
		{
			CCLOG("cocos2d: ERROR: Failed to compile fragment shader");
			return false;
		}
	}

	if (_vertShader)
	{
		glAttachShader(_program, _vertShader);
	}
	CHECK_GL_ERROR_DEBUG();

	if (_fragShader)
	{
		glAttachShader(_program, _fragShader);
	}
	_hashForUniforms = nullptr;

	CHECK_GL_ERROR_DEBUG();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	_shaderId = CCPrecompiledShaders::getInstance()->addShaders(vShaderByteArray, fShaderByteArray);
#endif

	return true;
}

void GLProgram::bindPredefinedVertexAttribs()
{
	static const struct {
		const char *attributeName;
		int location;
	} attribute_locations[] =
	{
		{ GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION },
		{ GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR },
		{ GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORD },
		{ GLProgram::ATTRIBUTE_NAME_NORMAL, GLProgram::VERTEX_ATTRIB_NORMAL },
	};

	const int size = sizeof(attribute_locations) / sizeof(attribute_locations[0]);

	for (int i = 0; i<size; i++) {
		glBindAttribLocation(_program, attribute_locations[i].location, attribute_locations[i].attributeName);
	}
}

void GLProgram::parseVertexAttribs()
{
	_vertexAttribs.clear();

	// Query and store vertex attribute meta-data from the program.
	GLint activeAttributes;
	GLint length;
	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
	if (activeAttributes > 0)
	{
		VertexAttrib attribute;

		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
		if (length > 0)
		{
			GLchar* attribName = (GLchar*)alloca(length + 1);

			for (int i = 0; i < activeAttributes; ++i)
			{
				// Query attribute info.
				glGetActiveAttrib(_program, i, length, nullptr, &attribute.size, &attribute.type, attribName);
				attribName[length] = '\0';
				attribute.name = std::string(attribName);

				// Query the pre-assigned attribute location
				attribute.index = glGetAttribLocation(_program, attribName);
				_vertexAttribs[attribute.name] = attribute;
			}
		}
	}
}

void GLProgram::parseUniforms()
{
	_userUniforms.clear();

	// Query and store uniforms from the program.
	GLint activeUniforms;
	glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &activeUniforms);
	if (activeUniforms > 0)
	{
		GLint length;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
		if (length > 0)
		{
			Uniform uniform;

			GLchar* uniformName = (GLchar*)alloca(length + 1);

			for (int i = 0; i < activeUniforms; ++i)
			{
				// Query uniform info.
				glGetActiveUniform(_program, i, length, nullptr, &uniform.size, &uniform.type, uniformName);
				uniformName[length] = '\0';

				// Only add uniforms that are not built-in.
				// The ones that start with 'CC_' are built-ins
				if (strncmp("CC_", uniformName, 3) != 0) {

					// remove possible array '[]' from uniform name
					if (uniform.size > 1 && length > 3)
					{
						char* c = strrchr(uniformName, '[');
						if (c)
						{
							*c = '\0';
						}
					}
					uniform.name = std::string(uniformName);
					uniform.location = glGetUniformLocation(_program, uniformName);
					GLenum __gl_error_code = glGetError();
					if (__gl_error_code != GL_NO_ERROR)
					{
						CCLOG("error: 0x%x", (int)__gl_error_code);
					}
					assert(__gl_error_code == GL_NO_ERROR);

					_userUniforms[uniform.name] = uniform;
				}
			}
		}
	}
}

bool GLProgram::compileShader(GLuint * shader, GLenum type, const GLchar* source)
{
	GLint status;

	if (!source)
	{
		return false;
	}

	const GLchar *sources[] = {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32 && CC_TARGET_PLATFORM != CC_PLATFORM_LINUX && CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
		(type == GL_VERTEX_SHADER ? "precision highp float;\n" : "precision mediump float;\n"),
#endif
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
	glShaderSource(*shader, sizeof(sources) / sizeof(*sources), sources, nullptr);
	glCompileShader(*shader);

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

	if (!status)
	{
		GLsizei length;
		glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
		GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);

		glGetShaderSource(*shader, length, nullptr, src);
		CCLOG("cocos2d: ERROR: Failed to compile shader:\n%s", src);

		if (type == GL_VERTEX_SHADER)
		{
			CCLOG("cocos2d: %s", getVertexShaderLog().c_str());
		}
		else
		{
			CCLOG("cocos2d: %s", getFragmentShaderLog().c_str());
		}
		free(src);

		return false;;
	}
	return (status == GL_TRUE);
}

GLint GLProgram::getAttribLocation(const std::string &attributeName) const
{
	return glGetAttribLocation(_program, attributeName.c_str());
}

GLint GLProgram::getUniformLocation(const std::string &attributeName) const
{
	return glGetUniformLocation(_program, attributeName.c_str());
}

void GLProgram::bindAttribLocation(const std::string &attributeName, GLuint index) const
{
	glBindAttribLocation(_program, index, attributeName.c_str());
}

void GLProgram::updateUniforms()
{
	_builtInUniforms[UNIFORM_P_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_P_MATRIX);
	_builtInUniforms[UNIFORM_MV_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_MV_MATRIX);
	_builtInUniforms[UNIFORM_MVP_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_MVP_MATRIX);

	_builtInUniforms[UNIFORM_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_TIME);
	_builtInUniforms[UNIFORM_SIN_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_SIN_TIME);
	_builtInUniforms[UNIFORM_COS_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_COS_TIME);

	_builtInUniforms[UNIFORM_RANDOM01] = glGetUniformLocation(_program, UNIFORM_NAME_RANDOM01);

	_builtInUniforms[UNIFORM_SAMPLER0] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER0);
	_builtInUniforms[UNIFORM_SAMPLER1] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER1);
	_builtInUniforms[UNIFORM_SAMPLER2] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER2);
	_builtInUniforms[UNIFORM_SAMPLER3] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER3);

	_flags.usesP = _builtInUniforms[UNIFORM_P_MATRIX] != -1;
	_flags.usesMV = _builtInUniforms[UNIFORM_MV_MATRIX] != -1;
	_flags.usesMVP = _builtInUniforms[UNIFORM_MVP_MATRIX] != -1;
	_flags.usesTime = (
		_builtInUniforms[UNIFORM_TIME] != -1 ||
		_builtInUniforms[UNIFORM_SIN_TIME] != -1 ||
		_builtInUniforms[UNIFORM_COS_TIME] != -1
		);
	_flags.usesRandom = _builtInUniforms[UNIFORM_RANDOM01] != -1;

	this->use();

	// Since sample most probably won't change, set it to 0,1,2,3 now.
	if (_builtInUniforms[UNIFORM_SAMPLER0] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER0], 0);
	if (_builtInUniforms[UNIFORM_SAMPLER1] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER1], 1);
	if (_builtInUniforms[UNIFORM_SAMPLER2] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER2], 2);
	if (_builtInUniforms[UNIFORM_SAMPLER3] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER3], 3);
}

bool GLProgram::link()
{
	CCASSERT(_program != 0, "Cannot link invalid program");

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	if (!_hasShaderCompiler)
	{
		// precompiled shader program is already linked

		//bindPredefinedVertexAttribs();
		parseVertexAttribs();
		parseUniforms();
		return true;
	}
#endif

	GLint status = GL_TRUE;

	bindPredefinedVertexAttribs();

	glLinkProgram(_program);

	parseVertexAttribs();
	parseUniforms();

	if (_vertShader)
	{
		glDeleteShader(_vertShader);
	}

	if (_fragShader)
	{
		glDeleteShader(_fragShader);
	}

	_vertShader = _fragShader = 0;

#if DEBUG || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	glGetProgramiv(_program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		CCLOG("cocos2d: ERROR: Failed to link program: %i", _program);
		GL::deleteProgram(_program);
		_program = 0;
	}
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	if (status == GL_TRUE)
	{
		CCPrecompiledShaders::getInstance()->addProgram(_program, _shaderId);
	}
#endif

	return (status == GL_TRUE);
}

void GLProgram::use()
{
	glUseProgram(_program);
}

GLint GLProgram::getUniformLocationForName(const char* name) const
{
	CCASSERT(name != nullptr, "Invalid uniform name");
	CCASSERT(_program != 0, "Invalid operation. Cannot get uniform location when program is not initialized");

	return glGetUniformLocation(_program, name);
}

void GLProgram::setUniformLocationWith1i(GLint location, GLint i1)
{
	bool updated = updateUniformLocation(location, &i1, sizeof(i1) * 1);

	if (updated)
	{
		glUniform1i((GLint)location, i1);
	}
}

void GLProgram::setUniformLocationWith2i(GLint location, GLint i1, GLint i2)
{
	GLint ints[2] = { i1, i2 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints));

	if (updated)
	{
		glUniform2i((GLint)location, i1, i2);
	}
}

void GLProgram::setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3)
{
	GLint ints[3] = { i1, i2, i3 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints));

	if (updated)
	{
		glUniform3i((GLint)location, i1, i2, i3);
	}
}

void GLProgram::setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4)
{
	GLint ints[4] = { i1, i2, i3, i4 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints));

	if (updated)
	{
		glUniform4i((GLint)location, i1, i2, i3, i4);
	}
}

void GLProgram::setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 2 * numberOfArrays);

	if (updated)
	{
		glUniform2iv((GLint)location, (GLsizei)numberOfArrays, ints);
	}
}

void GLProgram::setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 3 * numberOfArrays);

	if (updated)
	{
		glUniform3iv((GLint)location, (GLsizei)numberOfArrays, ints);
	}
}

void GLProgram::setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 4 * numberOfArrays);

	if (updated)
	{
		glUniform4iv((GLint)location, (GLsizei)numberOfArrays, ints);
	}
}

void GLProgram::setUniformLocationWith1f(GLint location, GLfloat f1)
{
	bool updated = updateUniformLocation(location, &f1, sizeof(f1) * 1);

	if (updated)
	{
		glUniform1f((GLint)location, f1);
	}
}

void GLProgram::setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2)
{
	GLfloat floats[2] = { f1, f2 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats));

	if (updated)
	{
		glUniform2f((GLint)location, f1, f2);
	}
}

void GLProgram::setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3)
{
	GLfloat floats[3] = { f1, f2, f3 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats));

	if (updated)
	{
		glUniform3f((GLint)location, f1, f2, f3);
	}
}

void GLProgram::setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4)
{
	GLfloat floats[4] = { f1, f2, f3, f4 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats));

	if (updated)
	{
		glUniform4f((GLint)location, f1, f2, f3, f4);
	}
}

void GLProgram::setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 2 * numberOfArrays);

	if (updated)
	{
		glUniform2fv((GLint)location, (GLsizei)numberOfArrays, floats);
	}
}

void GLProgram::setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 3 * numberOfArrays);

	if (updated)
	{
		glUniform3fv((GLint)location, (GLsizei)numberOfArrays, floats);
	}
}

void GLProgram::setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 4 * numberOfArrays);

	if (updated)
	{
		glUniform4fv((GLint)location, (GLsizei)numberOfArrays, floats);
	}
}

void GLProgram::setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices) {
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 4 * numberOfMatrices);

	if (updated)
	{
		glUniformMatrix2fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
	}
}

void GLProgram::setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices) {
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 9 * numberOfMatrices);

	if (updated)
	{
		glUniformMatrix3fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
	}
}


void GLProgram::setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices)
{
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 16 * numberOfMatrices);

	if (updated)
	{
		glUniformMatrix4fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
	}
}

NS_CC_END