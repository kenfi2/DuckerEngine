#include "buffermanager.h"
#include "renderbuffer.h"

#include <graphics/texture/texture.h>
#include <graphics/painter.h>

BufferManager::BufferManager()
{
    m_renderBuffer = std::make_shared<RenderBuffer>();
}

BufferManager::~BufferManager()
{
}

void BufferManager::clear(const Color& color)
{
    m_vertexBuffer.reset();
    m_drawCommands.reset();
    m_clearColor = color;
}

void BufferManager::reset()
{
    m_vertexBuffer.reset();
    m_drawCommands.reset();
    m_pendingTextures.clear();
}

void BufferManager::setTexture(const TexturePtr& texture)
{
    const SizeI& size = texture->getSize();
    m_width = size.w;
    m_height = size.h;
    m_texture = texture->get();
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
    m_renderBuffer->upload((void*)m_vertexBuffer.data(), m_vertexBuffer.size(), frameIndex);
}

void DrawCommand::bindTexture(SDL_GPURenderPass* renderPass)
{
    if(texture)
        texture->bind(renderPass);
    texture = nullptr;
}
