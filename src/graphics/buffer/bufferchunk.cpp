#include "bufferchunk.h"

#include <graphics/drawbuffer.h>
#include <graphics/painter.h>

BufferChunk::BufferChunk()
{
    SDL_GPUTransferBufferCreateInfo tbInfo;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.props = 0;
    tbInfo.size = MAX_BUFFER_SIZE;

    m_transferBuffer = SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo);
    if(!m_transferBuffer) {
        SDL_Log("Error transfering buffer: %s", SDL_GetError());
        return;
    }
    
    m_drawBuffer = new DrawBuffer(MAX_BUFFER_SIZE);
}

BufferChunk::~BufferChunk()
{
    if(m_transferBuffer)
        SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), m_transferBuffer);
    if(m_drawBuffer)
        delete m_drawBuffer;
}

void BufferChunk::map()
{
    m_data = SDL_MapGPUTransferBuffer(g_painter->getDevice(), m_transferBuffer, false);
}

void BufferChunk::upload(SDL_GPUCommandBuffer *commandBuffer)
{
    SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), m_transferBuffer);

    SDL_GPUCopyPass *cpass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation loc;
    loc.transfer_buffer = m_transferBuffer;
    loc.offset = 0;

    SDL_GPUBufferRegion dest;
    dest.buffer = m_drawBuffer->getBuffer();
    dest.size = m_count;
    dest.offset = 0;

    SDL_UploadToGPUBuffer(cpass, &loc, &dest, false);
    SDL_EndGPUCopyPass(cpass);
}

void BufferChunk::clear()
{
    m_count = 0;
    m_offset = 0;
    memset(m_data, 0, MAX_BUFFER_SIZE);
}

