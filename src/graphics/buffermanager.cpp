#include "buffermanager.h"
#include "renderbuffer.h"

#include <graphics/texture/texture.h>
#include <graphics/painter.h>

BufferManager::BufferManager(uint32_t id, size_t frames) : m_textures(1)
{
    m_renderBuffer = std::make_shared<RenderBuffer>(frames);
    m_id = id;
}

BufferManager::~BufferManager()
{
}

void BufferManager::clear(const Color& color)
{
    m_renderBuffer->reset();
    m_drawCount = 0;
    m_lastDataCount = 0;
    m_clearColor = color;
    m_clear = true;
}

void BufferManager::reset()
{
    m_renderBuffer->reset();
    m_drawCount = 0;
    m_lastDataCount = 0;
    m_pendingTextures.clear();
    m_textureBufferOffset = 0;
    if(m_textureBuffer && m_textureData) {
        SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), m_textureBuffer.get());
        m_textureData = nullptr;
    }
}

SDL_GPUTexture* BufferManager::createTexture(const TexturePtr& texture)
{
    const SizeI& size = texture->getSize();
    m_width = size.w;
    m_height = size.h;

    SDL_GPUTextureCreateInfo textureInfo;
    SDL_zero(textureInfo);
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.width = m_width;
    textureInfo.height = m_height;
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

    m_textures[texture->getId()].first = GPUTexturePtr(SDL_CreateGPUTexture(g_painter->getDevice(), &textureInfo));
    return m_textures[texture->getId()].first.get();
}

void BufferManager::genTextures(size_t n, uint32_t* textures)
{
    for(size_t i = 0; i < n; ++i) {
        if(!m_freeIds.empty()) {
            textures[i] = m_freeIds.top();
            m_freeIds.pop();
        } else {
            textures[i] = (uint32_t)m_textures.size();
            m_textures.emplace_back(nullptr, nullptr);
        }
    }
}

void BufferManager::updateTextureSampler(size_t n, uint32_t* textures, SDL_GPUSamplerCreateInfo* samplerInfo)
{
    for(size_t i = 0; i < n; ++i)
        m_textures[textures[i]].second = GPUSamplerPtr(SDL_CreateGPUSampler(g_painter->getDevice(), &samplerInfo[i]));
}

void BufferManager::deleteTextures(size_t n, uint32_t* textures)
{
    for(size_t i = 0; i < n; ++i) {
        uint32_t id = textures[i];

        m_textures[id] = std::make_pair(nullptr, nullptr);

        m_freeIds.push(id);
    }
}

void BufferManager::uploadTextureData(const TexturePtr& texture, const unsigned char* data, size_t dataSize)
{
    if(m_textureBuffer == nullptr || m_textureBufferSize < dataSize) {
        size_t bufferSize = dataSize + SmallBufferSize;

        SDL_GPUTransferBufferCreateInfo tbInfo;
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.props = 0;
        tbInfo.size = (uint32_t)bufferSize;

        m_textureBuffer = GPUTransferBufferPtr(SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo));
        m_textureBufferSize = bufferSize;
    }

    if(!m_textureData)
        m_textureData = (unsigned char*)SDL_MapGPUTransferBuffer(g_painter->getDevice(), m_textureBuffer.get(), false);

    if(!m_textures[texture->getId()].first) {
        const SizeI& size = texture->getSize();
    
        SDL_GPUTextureCreateInfo textureInfo;
        SDL_zero(textureInfo);
        textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
        textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        textureInfo.width = size.w;
        textureInfo.height = size.h;
        textureInfo.layer_count_or_depth = 1;
        textureInfo.num_levels = 1;
        textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

        m_textures[texture->getId()].first = GPUTexturePtr(SDL_CreateGPUTexture(g_painter->getDevice(), &textureInfo));
    }

    memcpy(m_textureData + m_textureBufferOffset, data, dataSize);
    m_pendingTextures.emplace_back(texture, m_textureBufferOffset);
    m_textureBufferOffset += dataSize;
}

void BufferManager::uploadPendingTextures(SDL_GPUCommandBuffer *commandBuffer)
{
    SDL_GPUTextureTransferInfo tti;
    tti.pixels_per_row = 0;
    tti.rows_per_layer = 0;
    tti.transfer_buffer = m_textureBuffer.get();

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    for(const auto& it : m_pendingTextures) {
        SDL_GPUTexture* texture = m_textures[it.first->getId()].first.get();

        tti.offset = (uint32_t)it.second;

        const SizeI& size = it.first->getSize();

        SDL_GPUTextureRegion dest;
        SDL_zero(dest);
        dest.texture = texture;
        dest.w = size.w;
        dest.h = size.h;
        dest.d = 1;

        SDL_UploadToGPUTexture(copyPass, &tti, &dest, false);
    }
    SDL_EndGPUCopyPass(copyPass);
}

void BufferManager::render(GPUCommand& gpuCommand)
{
    SDL_GPUCommandBuffer* commandBuffer = gpuCommand.getCommand();

    uploadPendingTextures(commandBuffer);

    SDL_GPUTexture* texture = nullptr;
    uint32_t width = m_width;
    uint32_t height = m_height;
    if(m_texture == nullptr) {
        if(m_id == 0) {
            texture = gpuCommand.acquireSwapchain();
            width = gpuCommand.width();
            height = gpuCommand.height();
        }

        if(!texture)
            return;
    } else
        texture = m_texture;

    if(!texture)
        return;

    static std::vector<SDL_GPUColorTargetInfo> colorTargets(1);

    SDL_zero(colorTargets[0]);
    colorTargets[0].texture = texture;
    if(m_clear) {
        m_clear = false;
        colorTargets[0].load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargets[0].store_op = SDL_GPU_STOREOP_STORE;
        colorTargets[0].clear_color = SDL_FColor{ m_clearColor.rF(), m_clearColor.gF(), m_clearColor.bF(), m_clearColor.aF() };
    } else {
        colorTargets[0].load_op = SDL_GPU_LOADOP_LOAD;
        colorTargets[0].store_op = SDL_GPU_STOREOP_STORE;
    }

    SDL_GPUBuffer* buffer = m_renderBuffer->getBuffer();
    m_renderBuffer->upload();

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), (uint32_t)colorTargets.size(), NULL);

    SDL_GPUBufferBinding binding;
    binding.buffer = buffer;
    binding.offset = 0;

    SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);

    Program* drawProgram = nullptr;
    int updateFlags = 0;
    int32_t lastState = -1;

    RectI frameBufferRect(0, 0, width >> colorTargets[0].mip_level, height >> colorTargets[0].mip_level);
    SDL_GPUViewport viewport;
    SDL_Rect rect;
    for(size_t i = 0; i < m_drawCount; ++i) {
        DrawCommand& drawCommand = m_drawCommands[i];

        const PainterState& drawState = g_painter->getState(drawCommand.state);
        if(lastState != drawState.id) {
            if(lastState == -1) {
                updateFlags = MustUpdateProgramResource;
                drawProgram = drawState.program;
                if(!drawState.clipRect.isEmpty())
                    updateFlags |= MustUpdateClipRect;
                if(!drawState.viewport.isEmpty())
                    updateFlags |= MustUpdateViewport;
            } else
                updateFlags = drawState.flags;
            lastState = (int32_t)drawState.id;
        }

        if(!drawState.program) {
            Program* program = g_programs.get(drawState.blendMode, drawCommand.type, drawState.bindedTextures != 0);
            if(drawProgram != program) {
                drawProgram = program;
                updateFlags = MustUpdateProgramResource;
            }
        }

        if(updateFlags & MustUpdateProgram)
            drawProgram->bind(renderPass);

        if(updateFlags & MustUpdateColor)
            drawProgram->setColor(drawState.color);

        if(updateFlags & MustUpdateLineWidth) {
            // apply geometry shader?
        }

        if(updateFlags & MustUpdatePointSize)
            drawProgram->setSize(drawState.pointSize);

        if(updateFlags & MustUpdateClipRect) {
            if(drawState.viewport.size() == drawState.resolution) {
                rect.x = drawState.clipRect.left();
                rect.y = drawState.resolution.h - drawState.clipRect.bottom() - 1;
                rect.w = drawState.clipRect.width();
                rect.h = drawState.clipRect.height();
            } else {
                rect.x = (int)((drawState.clipRect.left()                              /(float)drawState.resolution.w) * drawState.viewport.width());
                rect.y = (int)(((drawState.resolution.h - drawState.clipRect.bottom() - 1)/(float)drawState.resolution.h) * drawState.viewport.height());
                rect.w = (int)((drawState.clipRect.width()                             /(float)drawState.resolution.w) * drawState.viewport.width());
                rect.h = (int)((drawState.clipRect.height()                            /(float)drawState.resolution.h) * drawState.viewport.height());
            }

            SDL_SetGPUScissor(renderPass, &rect);
        }

        if(updateFlags & MustUpdateResolution)
            drawProgram->setResolution(drawState.resolution);

        if(updateFlags & MustUpdateViewport) {
            viewport.x = (float)drawState.viewport.x();
            viewport.y = (float)drawState.viewport.y();
            viewport.w = (float)drawState.viewport.width();
            viewport.h = (float)drawState.viewport.height();
            viewport.min_depth = 0;
            viewport.max_depth = 1;

            SDL_SetGPUViewport(renderPass, &viewport);
        }

        if(updateFlags & MustUpdateProjectionTransformMatrix) {
            Matrix3 projectionTransformMatrix = drawState.projectionMatrix * drawState.transformMatrix;
            drawProgram->setProjectionTransformMatrix(projectionTransformMatrix);
        }

        drawProgram->pushData(commandBuffer);

        static SDL_GPUTextureSamplerBinding textureBinding[MaxMultiTextures];
        for(size_t textureIndex = 0; textureIndex < drawState.bindedTextures; textureIndex++) {
            const auto& it = m_textures[drawState.textures[textureIndex]];
            textureBinding[textureIndex].texture = it.first.get();
            textureBinding[textureIndex].sampler = it.second.get();
        }
        if(drawState.bindedTextures != 0)
            SDL_BindGPUFragmentSamplers(renderPass, 0, textureBinding, drawState.bindedTextures);

        SDL_DrawGPUPrimitives(renderPass, (uint32_t)drawCommand.vertexCount, 1, (uint32_t)drawCommand.offset, 0);

        if(updateFlags & MustUpdateViewport) {
            viewport.x = 0;
            viewport.y = 0;
            viewport.w = (float)frameBufferRect.width();
            viewport.h = (float)frameBufferRect.height();
            viewport.min_depth = 0;
            viewport.max_depth = 1;

            SDL_SetGPUViewport(renderPass, &viewport);
        }

        if(updateFlags & MustUpdateClipRect) {
            rect.x = frameBufferRect.x();
            rect.y = frameBufferRect.y();
            rect.w = frameBufferRect.width();
            rect.h = frameBufferRect.height();
            SDL_SetGPUScissor(renderPass, &rect);
        }

        updateFlags = 0;
    }
    SDL_EndGPURenderPass(renderPass);
}
