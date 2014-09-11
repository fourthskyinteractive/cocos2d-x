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

#include "platform/rendersystem/gl/CCGLRenderSystem.h"

// Vertex attribute names
const char* cocos2d::ATTRIB_NAME_POSITION = "a_position";
const char* cocos2d::ATTRIB_NAME_COLOR = "a_color";
const char* cocos2d::ATTRIB_NAME_TEX_COORD = "a_texCoord";
const char* cocos2d::ATTRIB_NAME_NORMAL = "a_normal";
const char* cocos2d::ATTRIB_NAME_BLEND_WEIGHT = "a_blendWeight";
const char* cocos2d::ATTRIB_NAME_BLEND_INDEX = "a_blendIndex";

// Vertex attribute locations
const unsigned int cocos2d::ATTRIB_INDEX_POSITION = 0;
const unsigned int cocos2d::ATTRIB_INDEX_COLOR = 1;
const unsigned int cocos2d::ATTRIB_INDEX_TEX_COORD = 2;
const unsigned int cocos2d::ATTRIB_INDEX_NORMAL = 3;
const unsigned int cocos2d::ATTRIB_INDEX_BLEND_WEIGHT = 4;
const unsigned int cocos2d::ATTRIB_INDEX_BLEND_INDEX = 5;

const cocos2d::AttributeLocation cocos2d::attribute_locations[] =
{
	{ cocos2d::ElementSemantic::POSITION, cocos2d::ATTRIB_NAME_POSITION, cocos2d::ATTRIB_INDEX_POSITION },
	{ cocos2d::ElementSemantic::COLOR, cocos2d::ATTRIB_NAME_COLOR, cocos2d::ATTRIB_INDEX_COLOR },
	{ cocos2d::ElementSemantic::TEX_COORD, cocos2d::ATTRIB_NAME_TEX_COORD, cocos2d::ATTRIB_INDEX_TEX_COORD },
	{ cocos2d::ElementSemantic::NORMAL, cocos2d::ATTRIB_NAME_NORMAL, cocos2d::ATTRIB_INDEX_NORMAL },
	{ cocos2d::ElementSemantic::BLEND_WEIGHT, cocos2d::ATTRIB_NAME_BLEND_WEIGHT, cocos2d::ATTRIB_INDEX_BLEND_WEIGHT },
	{ cocos2d::ElementSemantic::BLEND_INDEX, cocos2d::ATTRIB_NAME_BLEND_INDEX, cocos2d::ATTRIB_INDEX_BLEND_INDEX }
};
const int cocos2d::attribute_locations_size = sizeof(cocos2d::attribute_locations) / sizeof(cocos2d::attribute_locations[0]);