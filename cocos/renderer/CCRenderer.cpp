/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "renderer/CCRenderer.h"

#include <algorithm>

#include "renderer/CCQuadCommand.h"
#include "renderer/CCBatchCommand.h"
#include "renderer/CCCustomCommand.h"
#include "renderer/CCGroupCommand.h"
#include "renderer/CCPrimitiveCommand.h"
#include "renderer/CCGLProgramCache.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCMeshCommand.h"
#include "base/CCConfiguration.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventType.h"
#include "renderer/CCVertexIndexBuffer.h"
#include "renderer/CCVertexIndexData.h"

NS_CC_BEGIN

// helper
static bool compareRenderCommand(RenderCommand* a, RenderCommand* b)
{
	return a->getGlobalOrder() < b->getGlobalOrder();
}

// queue

void RenderQueue::push_back(RenderCommand* command)
{
	float z = command->getGlobalOrder();
	if (z < 0)
		_queueNegZ.push_back(command);
	else if (z > 0)
		_queuePosZ.push_back(command);
	else
		_queue0.push_back(command);
}

ssize_t RenderQueue::size() const
{
	return _queueNegZ.size() + _queue0.size() + _queuePosZ.size();
}

void RenderQueue::sort()
{
	// Don't sort _queue0, it already comes sorted
	std::sort(std::begin(_queueNegZ), std::end(_queueNegZ), compareRenderCommand);
	std::sort(std::begin(_queuePosZ), std::end(_queuePosZ), compareRenderCommand);
}

RenderCommand* RenderQueue::operator[](ssize_t index) const
{
	if (index < static_cast<ssize_t>(_queueNegZ.size()))
		return _queueNegZ[index];

	index -= _queueNegZ.size();

	if (index < static_cast<ssize_t>(_queue0.size()))
		return _queue0[index];

	index -= _queue0.size();

	if (index < static_cast<ssize_t>(_queuePosZ.size()))
		return _queuePosZ[index];

	CCASSERT(false, "invalid index");
	return nullptr;
}

void RenderQueue::clear()
{
	_queueNegZ.clear();
	_queue0.clear();
	_queuePosZ.clear();
}

//
//
//
static const int DEFAULT_RENDER_QUEUE = 0;

//
// constructors, destructors, init
//
Renderer::Renderer()
	: _lastMaterialID(0)
	, _lastBatchedMeshCommand(nullptr)
	, _numQuads(0)
	, _glViewAssigned(false)
	, _isRendering(false)
	, _vertexBuffer(nullptr)
	, _indexBuffer(nullptr)
	, _vertexData(nullptr)
#if CC_ENABLE_CACHE_TEXTURE_DATA
	, _cacheTextureListener(nullptr)
#endif
{
	_groupCommandManager = new GroupCommandManager();

	_commandGroupStack.push(DEFAULT_RENDER_QUEUE);

	RenderQueue defaultRenderQueue;
	_renderGroups.push_back(defaultRenderQueue);
	_batchedQuadCommands.reserve(BATCH_QUADCOMMAND_RESEVER_SIZE);
}

Renderer::~Renderer()
{
	_renderGroups.clear();
	_groupCommandManager->release();

	CC_SAFE_RELEASE(_vertexBuffer);
	CC_SAFE_RELEASE(_indexBuffer);
	CC_SAFE_RELEASE(_vertexData);

#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(_cacheTextureListener);
#endif
}

void Renderer::initGLView()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
	_cacheTextureListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
		/** listen the event that renderer was recreated on Android/WP8 */
		this->setupVBOAndVAO();
	});

	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_cacheTextureListener, -1);
#endif

	setupIndices();

	setupVBOAndVAO();

	_glViewAssigned = true;
}

void Renderer::setupIndices()
{
	for (int i = 0; i < VBO_SIZE; i++)
	{
		_indices[i * 6 + 0] = (GLushort)(i * 4 + 0);
		_indices[i * 6 + 1] = (GLushort)(i * 4 + 1);
		_indices[i * 6 + 2] = (GLushort)(i * 4 + 2);
		_indices[i * 6 + 3] = (GLushort)(i * 4 + 3);
		_indices[i * 6 + 4] = (GLushort)(i * 4 + 2);
		_indices[i * 6 + 5] = (GLushort)(i * 4 + 1);
	}
}

void Renderer::setupVBOAndVAO()
{
	// Create buffers
	_vertexBuffer = VertexBuffer::create(sizeof(V3F_C4B_T2F), 4 * VBO_SIZE, true);
	_indexBuffer = IndexBuffer::create(IndexBuffer::IndexType::INDEX_TYPE_SHORT_16, 6 * VBO_SIZE, false);
	_vertexBuffer->retain();
	_indexBuffer->retain();

	// Update buffers' data
	mapBuffers();

	_vertexData = VertexData::create();
	_vertexData->retain();

	// vertices
	_vertexData->setStream(_vertexBuffer, VertexStreamAttribute(0, GLProgram::VERTEX_ATTRIB_POSITION, GL_FLOAT, 3));

	// colors
	_vertexData->setStream(_vertexBuffer, VertexStreamAttribute(offsetof(V3F_C4B_T2F, colors), GLProgram::VERTEX_ATTRIB_COLOR, GL_UNSIGNED_BYTE, 4, true));

	// tex coords
	_vertexData->setStream(_vertexBuffer, VertexStreamAttribute(offsetof(V3F_C4B_T2F, texCoords), GLProgram::VERTEX_ATTRIB_TEX_COORD, GL_FLOAT, 2));
}

void Renderer::mapBuffers()
{
	_vertexBuffer->updateVertices(_quads, 4 * VBO_SIZE, 0);
	_indexBuffer->updateIndices(_indices, 6 * VBO_SIZE, 0);
}

void Renderer::addCommand(RenderCommand* command)
{
	int renderQueue = _commandGroupStack.top();
	addCommand(command, renderQueue);
}

void Renderer::addCommand(RenderCommand* command, int renderQueue)
{
	CCASSERT(!_isRendering, "Cannot add command while rendering");
	CCASSERT(renderQueue >= 0, "Invalid render queue");
	CCASSERT(command->getType() != RenderCommand::Type::UNKNOWN_COMMAND, "Invalid Command Type");
	_renderGroups[renderQueue].push_back(command);
}

void Renderer::pushGroup(int renderQueueID)
{
	CCASSERT(!_isRendering, "Cannot change render queue while rendering");
	_commandGroupStack.push(renderQueueID);
}

void Renderer::popGroup()
{
	CCASSERT(!_isRendering, "Cannot change render queue while rendering");
	_commandGroupStack.pop();
}

int Renderer::createRenderQueue()
{
	RenderQueue newRenderQueue;
	_renderGroups.push_back(newRenderQueue);
	return (int)_renderGroups.size() - 1;
}

void Renderer::visitRenderQueue(const RenderQueue& queue)
{
	ssize_t size = queue.size();

	for (ssize_t index = 0; index < size; ++index)
	{
		auto command = queue[index];
		auto commandType = command->getType();
		if (RenderCommand::Type::QUAD_COMMAND == commandType)
		{
			flush3D();
			auto cmd = static_cast<QuadCommand*>(command);
			//Batch quads
			if (_numQuads + cmd->getQuadCount() > VBO_SIZE)
			{
				CCASSERT(cmd->getQuadCount() >= 0 && cmd->getQuadCount() < VBO_SIZE, "VBO is not big enough for quad data, please break the quad data down or use customized render command");

				//Draw batched quads if VBO is full
				drawBatchedQuads();
			}

			_batchedQuadCommands.push_back(cmd);

			memcpy(_quads + _numQuads, cmd->getQuads(), sizeof(V3F_C4B_T2F_Quad) * cmd->getQuadCount());
			convertToWorldCoordinates(_quads + _numQuads, cmd->getQuadCount(), cmd->getModelView());

			_numQuads += cmd->getQuadCount();

		}
		else if (RenderCommand::Type::GROUP_COMMAND == commandType)
		{
			flush();
			int renderQueueID = ((GroupCommand*)command)->getRenderQueueID();
			visitRenderQueue(_renderGroups[renderQueueID]);
		}
		else if (RenderCommand::Type::CUSTOM_COMMAND == commandType)
		{
			flush();
			auto cmd = static_cast<CustomCommand*>(command);
			cmd->execute();
		}
		else if (RenderCommand::Type::BATCH_COMMAND == commandType)
		{
			flush();
			auto cmd = static_cast<BatchCommand*>(command);
			cmd->execute();
		}
		else if (RenderCommand::Type::PRIMITIVE_COMMAND == commandType)
		{
			flush();
			auto cmd = static_cast<PrimitiveCommand*>(command);
			cmd->execute();
		}
		else if (RenderCommand::Type::MESH_COMMAND == commandType)
		{
			flush2D();
			auto cmd = static_cast<MeshCommand*>(command);
			if (_lastBatchedMeshCommand == nullptr || _lastBatchedMeshCommand->getMaterialID() != cmd->getMaterialID())
			{
				flush3D();
				cmd->preBatchDraw();
				cmd->batchDraw();
				_lastBatchedMeshCommand = cmd;
			}
			else
			{
				cmd->batchDraw();
			}
		}
		else
		{
			CCLOGERROR("Unknown commands in renderQueue");
		}
	}
}

void Renderer::render()
{
	//Uncomment this once everything is rendered by new renderer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//TODO setup camera or MVP
	_isRendering = true;

	if (_glViewAssigned)
	{
		// cleanup
		_drawnBatches = _drawnVertices = 0;

		//Process render commands
		//1. Sort render commands based on ID
		for (auto &renderqueue : _renderGroups)
		{
			renderqueue.sort();
		}
		visitRenderQueue(_renderGroups[0]);
		flush();
	}
	clean();
	_isRendering = false;
}

void Renderer::clean()
{
	// Clear render group
	for (size_t j = 0; j < _renderGroups.size(); j++)
	{
		//commands are owned by nodes
		// for (const auto &cmd : _renderGroups[j])
		// {
		//     cmd->releaseToCommandPool();
		// }
		_renderGroups[j].clear();
	}

	// Clear batch quad commands
	_batchedQuadCommands.clear();
	_numQuads = 0;

	_lastMaterialID = 0;
	_lastBatchedMeshCommand = nullptr;
}

void Renderer::convertToWorldCoordinates(V3F_C4B_T2F_Quad* quads, ssize_t quantity, const Mat4& modelView)
{
	//    kmMat4 matrixP, mvp;
	//    kmGLGetMatrix(KM_GL_PROJECTION, &matrixP);
	//    kmMat4Multiply(&mvp, &matrixP, &modelView);
	for (ssize_t i = 0; i<quantity; ++i)
	{
		V3F_C4B_T2F_Quad *q = &quads[i];
		Vec3 *vec1 = (Vec3*)&q->bl.vertices;
		modelView.transformPoint(vec1);

		Vec3 *vec2 = (Vec3*)&q->br.vertices;
		modelView.transformPoint(vec2);

		Vec3 *vec3 = (Vec3*)&q->tr.vertices;
		modelView.transformPoint(vec3);

		Vec3 *vec4 = (Vec3*)&q->tl.vertices;
		modelView.transformPoint(vec4);
	}
}

void Renderer::drawBatchedQuads()
{
	//TODO we can improve the draw performance by insert material switching command before hand.

	int quadsToDraw = 0;
	int startQuad = 0;

	//Upload buffer to VBO
	if (_numQuads <= 0 || _batchedQuadCommands.empty())
	{
		return;
	}

	// Update vertices
	void* buf = _vertexBuffer->map();
	memcpy(buf, _quads, sizeof(_quads[0]) * _numQuads);
	_vertexBuffer->unmap();

	_vertexData->use();

	GLView* glView = Director::getInstance()->getOpenGLView();

	//Start drawing verties in batch
	for (const auto& cmd : _batchedQuadCommands)
	{
		auto newMaterialID = cmd->getMaterialID();
		if (_lastMaterialID != newMaterialID || newMaterialID == QuadCommand::MATERIAL_ID_DO_NOT_BATCH)
		{
			//Draw quads
			if (quadsToDraw > 0)
			{
				glView->drawElements(GL_TRIANGLES, (GLsizei)quadsToDraw * 6, _indexBuffer, startQuad * 6);
				_drawnBatches++;
				_drawnVertices += quadsToDraw * 6;

				startQuad += quadsToDraw;
				quadsToDraw = 0;
			}

			//Use new material
			cmd->useMaterial();
			_lastMaterialID = newMaterialID;
		}

		quadsToDraw += cmd->getQuadCount();
	}

	//Draw any remaining quad
	if (quadsToDraw > 0)
	{
		glView->drawElements(GL_TRIANGLES, (GLsizei)quadsToDraw * 6, _indexBuffer, startQuad * 6);
		_drawnBatches++;
		_drawnVertices += quadsToDraw * 6;
	}

	_vertexData->disable();

	_batchedQuadCommands.clear();
	_numQuads = 0;
}

void Renderer::flush()
{
	flush2D();
	flush3D();
}

void Renderer::flush2D()
{
	drawBatchedQuads();
	_lastMaterialID = 0;
}

void Renderer::flush3D()
{
	if (_lastBatchedMeshCommand)
	{
		_lastBatchedMeshCommand->postBatchDraw();
		_lastBatchedMeshCommand = nullptr;
	}
}

// helpers

bool Renderer::checkVisibility(const Mat4 &transform, const Size &size)
{
	// half size of the screen
	Size screen_half = Director::getInstance()->getWinSize();
	screen_half.width /= 2;
	screen_half.height /= 2;

	float hSizeX = size.width / 2;
	float hSizeY = size.height / 2;

	Vec4 v4world, v4local;
	v4local.set(hSizeX, hSizeY, 0, 1);
	transform.transformVector(v4local, &v4world);

	// center of screen is (0,0)
	v4world.x -= screen_half.width;
	v4world.y -= screen_half.height;

	// convert content size to world coordinates
	float wshw = std::max(fabsf(hSizeX * transform.m[0] + hSizeY * transform.m[4]), fabsf(hSizeX * transform.m[0] - hSizeY * transform.m[4]));
	float wshh = std::max(fabsf(hSizeX * transform.m[1] + hSizeY * transform.m[5]), fabsf(hSizeX * transform.m[1] - hSizeY * transform.m[5]));

	// compare if it in the positive quadrant of the screen
	float tmpx = (fabsf(v4world.x) - wshw);
	float tmpy = (fabsf(v4world.y) - wshh);
	bool ret = (tmpx < screen_half.width && tmpy < screen_half.height);

	return ret;
}

NS_CC_END
