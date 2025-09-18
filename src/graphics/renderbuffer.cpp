#include "renderbuffer.h"
#include "painter.h"

RenderBuffer::RenderBuffer()
{
    m_buffers.resize(FramesInFlight);
}

RenderBuffer::~RenderBuffer()
{
    for(Data& data : m_buffers) {
        if(data.vertexBuffer)
            SDL_ReleaseGPUBuffer(g_painter->getDevice(), data.vertexBuffer);
        if(data.transferBuffer)
            SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer);
    }
}

SDL_GPUBuffer* RenderBuffer::acquireVertexBuffer(size_t size, int frameIndex)
{
    Data& data = m_buffers[frameIndex];
    if(data.vertexBuffer == nullptr || data.vertexSize < size) {
        size_t bufferSize = size + 5000;
        if(data.vertexBuffer)
            SDL_ReleaseGPUBuffer(g_painter->getDevice(), data.vertexBuffer);

        SDL_GPUBufferCreateInfo bufferInfo;
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bufferInfo.size = (uint32_t)(bufferSize * sizeof(VertexBuffer));
        bufferInfo.props = 0;

        data.vertexBuffer = SDL_CreateGPUBuffer(g_painter->getDevice(), &bufferInfo);
        if(data.vertexBuffer)
            data.vertexSize = bufferSize;
        else
            SDL_Log("SDL_CreateGPUBuffer: %s", SDL_GetError());
    }
    return data.vertexBuffer;
}

void RenderBuffer::upload(const VertexBuffer* vertexData, size_t size, int frameIndex)
{
    // Currently based on what we need, this code contains overhead in DX12
    // making it completely unnecessary to use CopyBufferRegion to transfer data from the CPU to the GPU
    // only Map and UnMap working directly in the buffer created initially would be necessary, but what can we do?
    Data& data = m_buffers[frameIndex];

    size_t neededSize = size * sizeof(VertexBuffer);

    if(!data.transferBuffer || data.transferSize < neededSize) {
        if(data.transferBuffer)
            SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer);

        SDL_GPUTransferBufferCreateInfo tbInfo;
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.props = 0;
        tbInfo.size = (uint32_t)(neededSize + 5000);

        data.transferBuffer = SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo);
        if(!data.transferBuffer) {
            SDL_Log("Error transfering buffer: %s", SDL_GetError());
            return;
        }
        data.transferSize = neededSize + 5000;
    }

    void* map = SDL_MapGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer, false);
    memcpy(map, vertexData, neededSize);
    SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer);

    SDL_GPUCopyPass *cpass = SDL_BeginGPUCopyPass(g_painter->getGPUCommand().getCommand());

    SDL_GPUTransferBufferLocation loc;
    loc.transfer_buffer = data.transferBuffer;
    loc.offset = 0;

    SDL_GPUBufferRegion dest;
    dest.buffer = data.vertexBuffer;
    dest.size = (uint32_t)neededSize;
    dest.offset = 0;

    SDL_UploadToGPUBuffer(cpass, &loc, &dest, false);
    SDL_EndGPUCopyPass(cpass);
}
