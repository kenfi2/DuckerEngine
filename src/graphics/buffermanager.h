#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <graphics/shaders/program.h>

#include <utils/include.h>
#include <stack>
#include "renderbuffer.h"

#include "painterstate.h"

struct DrawCommand {
    DrawCommand() = default;

    size_t vertexCount = 0;
    size_t offset = 0;
    size_t state = 0;
    PrimitiveType type = LastPrimitiveType;
};

class GPUCommand;
class RenderBuffer;
class BufferManager {
    enum {
        SmallBufferSize = 2 * 1024 * 1024,
        MediumBufferSize = 8 * 1024 * 1024,
        LargeBufferSize = 32 * 1024 * 1024 
    };
    
public:
    BufferManager(uint32_t id, size_t frames = 1);
    ~BufferManager();

    template<typename T>
    T* add(size_t count, PrimitiveType type);
    void clear(const Color& color);
    void reset();

    SDL_GPUTexture* createTexture(const TexturePtr& texture);
    void setTexture(SDL_GPUTexture* texture) { m_texture = texture; }

    void genTextures(size_t n, uint32_t* textures);
    void updateTextureSampler(size_t n, uint32_t* textures, SDL_GPUSamplerCreateInfo* samplerInfo);
    void deleteTextures(size_t n, uint32_t* textures);

    void uploadTextureData(const TexturePtr& texture, const unsigned char* data, size_t dataSize);
    void uploadPendingTextures(SDL_GPUCommandBuffer* commandBuffer);

    void render(GPUCommand& gpuCommand);

private:
    GPUTransferBufferPtr m_textureBuffer = nullptr;
    unsigned char* m_textureData = nullptr;
    size_t m_textureBufferSize = 0;
    size_t m_textureBufferOffset = 0;

private:
    std::vector<DrawCommand> m_drawCommands;
    std::vector<std::pair<TexturePtr, size_t>> m_pendingTextures;
    Color m_clearColor;
    RenderBufferPtr m_renderBuffer = nullptr;
    SDL_GPUTexture* m_texture = nullptr;
    std::vector<std::pair<GPUTexturePtr, GPUSamplerPtr>> m_textures;
    std::stack<uint32_t> m_freeIds;
    size_t m_drawCount = 0;
    size_t m_lastDataCount = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    int32_t m_id = -1;
    bool m_clear = false;
};

template<typename T>
inline T* BufferManager::add(size_t count, PrimitiveType type)
{
    size_t index = m_renderBuffer->add(count * sizeof(T));

    if(++m_drawCount > m_drawCommands.size())
        m_drawCommands.emplace_back();

    DrawCommand& drawCommand = m_drawCommands[m_drawCount - 1];
    drawCommand.vertexCount = count;
    drawCommand.offset = index / sizeof(T);
    drawCommand.state = g_painter->getCurrentState();
    drawCommand.type = type;
    return &m_renderBuffer->at<T&>(index);
}

#endif
