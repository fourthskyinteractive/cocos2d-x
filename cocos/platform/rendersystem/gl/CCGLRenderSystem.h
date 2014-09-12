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

#ifndef __CC_EGLRenderSystem_H__
#define __CC_EGLRenderSystem_H__

#include "base/ccTypes.h"

NS_CC_BEGIN

// Vertex attribute names
static const char* ATTRIB_NAME_POSITION;
static const char* ATTRIB_NAME_COLOR;
static const char* ATTRIB_NAME_TEX_COORD;
static const char* ATTRIB_NAME_NORMAL;
static const char* ATTRIB_NAME_BLEND_WEIGHT;
static const char* ATTRIB_NAME_BLEND_INDEX;

// Vertex attribute locations
static const unsigned int ATTRIB_INDEX_POSITION;
static const unsigned int ATTRIB_INDEX_COLOR;
static const unsigned int ATTRIB_INDEX_TEX_COORD;
static const unsigned int ATTRIB_INDEX_NORMAL;
static const unsigned int ATTRIB_INDEX_BLEND_WEIGHT;
static const unsigned int ATTRIB_INDEX_BLEND_INDEX;

// Bind semantic (global to Cocos2d) to internal OpenGL representation
struct AttributeLocation
{
	ElementSemantic semantic;
	const char *attributeName;
	int location;
};

static const AttributeLocation attribute_locations[];
static const int attribute_locations_size;

NS_CC_END

#endif