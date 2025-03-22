#include "drawbuffer.h"

#include <utils/include.h>
#include <utils/color.h>
#include <frametimer.h>
#include <graphics/buffer/buffermanager.h>
#include <graphics/painter.h>

int DrawBuffer::drawBufferId = 0;

DrawBuffer::DrawBuffer(uint32_t size)
{
    SDL_GPUBufferCreateInfo bufferInfo;
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bufferInfo.size = size;
    bufferInfo.props = 0;

    m_buffer = SDL_CreateGPUBuffer(g_painter->getDevice(), &bufferInfo);
    if(!m_buffer) {
        SDL_Log("SDL_CreateGPUBuffer: %s", SDL_GetError());
        return;
    }

    std::string name = "GPUBuffer" + std::to_string(++drawBufferId);
    SDL_SetGPUBufferName(g_painter->getDevice(), m_buffer, name.c_str());

    SDL_zero(m_colorInfo);
    m_colorInfo.clear_color.a = 1.0f;
    m_colorInfo.load_op = SDL_GPU_LOADOP_LOAD;
    m_colorInfo.store_op = SDL_GPU_STOREOP_STORE;
}

DrawBuffer::~DrawBuffer()
{
    if(m_buffer)
        SDL_ReleaseGPUBuffer(g_painter->getDevice(), m_buffer);
}

void DrawBuffer::draw(SDL_GPURenderPass* renderPass, const BufferData& buffer) const
{
    static SDL_GPUBufferBinding binding;
    binding.buffer = m_buffer;
    binding.offset = 0;

    SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(renderPass, buffer.getCount(), 1, buffer.getOffset(), 0);
}
