#include "buffermanager.h"
#include "renderbuffer.h"

#include <graphics/texture/texture.h>

BufferManager::BufferManager()
{
    m_renderBuffer = std::make_shared<RenderBuffer>();
}

BufferManager::~BufferManager()
{
}

VertexBuffer *BufferManager::add(size_t count, PrimitiveType type, TexturePtr texture)
{
    size_t index = m_vertexBuffer.add(count);
    m_drawCommands.emplace_back(count, index, type, texture);
    return &m_vertexBuffer[index];
}

void BufferManager::reset()
{
    m_vertexBuffer.reset();

    size_t lastSize = m_drawCommands.size();
    m_drawCommands.clear();

    if(m_drawCommands.size() < lastSize + 64)
        m_drawCommands.reserve(lastSize + 64);

    m_pendingTextures.clear();
}

void BufferManager::uploadPendingTextures(SDL_GPUCommandBuffer* commandBuffer)
{
    for(const TexturePtr& texture : m_pendingTextures)
        texture->upload(commandBuffer);
}

SDL_GPUBuffer *BufferManager::getBuffer(uint32_t frameIndex)
{
    return m_renderBuffer->acquireVertexBuffer(m_vertexBuffer.size(), frameIndex);
}

void BufferManager::upload(uint32_t frameIndex)
{
    m_renderBuffer->upload(m_vertexBuffer.data(), m_vertexBuffer.size(), frameIndex);
}
