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

    DrawCommand& drawCommand = m_drawCommands.emplace_back();
    drawCommand.vertexCount = count;
    drawCommand.offset = index;
    drawCommand.type = type;
    drawCommand.texture = texture;
    return &m_vertexBuffer[index];
}

void BufferManager::reset()
{
    m_vertexBuffer.reset();
    m_drawCommands.reset();
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

void DrawCommand::bindTexture(SDL_GPURenderPass* renderPass)
{
    if(texture)
        texture->bind(renderPass);
    texture = nullptr;
}
