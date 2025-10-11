#include "painter.h"

void GPUBufferRelease::operator()(SDL_GPUBuffer* buffer) const
{
    SDL_ReleaseGPUBuffer(g_painter->getDevice(), buffer);
}

void GPUBufferRelease::operator()(SDL_GPUTransferBuffer* transferBuffer) const
{
    SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), transferBuffer);
}

void GPUBufferRelease::operator()(SDL_GPUTexture* texture) const
{
    SDL_ReleaseGPUTexture(g_painter->getDevice(), texture);
}

void GPUBufferRelease::operator()(SDL_GPUSampler* sampler) const
{
    SDL_ReleaseGPUSampler(g_painter->getDevice(), sampler);
}

RenderBuffer::RenderBuffer(size_t frames) : m_expandSize(32768), m_size(0)
{
    m_buffers.resize(std::max(frames, 1ULL));
}

RenderBuffer::~RenderBuffer()
{
    for(Data& data : m_buffers) {
        if(data.transferBuffer) {
            if(data.data)
                SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer.get());
        }
    }
}

size_t RenderBuffer::add(size_t count)
{
    int frameIndex = g_painter->getFrameIndex() % m_buffers.size();

    Data& data = m_buffers[frameIndex];
    size_t index = m_size;
    m_size += count;
    if(data.vertexBuffer == nullptr || data.size < m_size) {
        size_t bufferSize = m_size + m_expandSize;

        SDL_GPUBufferCreateInfo bufferInfo;
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bufferInfo.size = (uint32_t)(bufferSize);
        bufferInfo.props = 0;

        data.vertexBuffer = GPUBufferPtr(SDL_CreateGPUBuffer(g_painter->getDevice(), &bufferInfo));
        if(!data.vertexBuffer) {
            SDL_Log("SDL_CreateGPUBuffer: %s", SDL_GetError());
            return (size_t)-1;
        }

        if(data.transferBuffer) {
            if(data.data)
                SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer.get());
        }

        SDL_GPUTransferBufferCreateInfo tbInfo;
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.props = 0;
        tbInfo.size = (uint32_t)(bufferSize);

        data.transferBuffer = GPUTransferBufferPtr(SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo));
        if(!data.transferBuffer) {
            SDL_Log("Error transfering buffer: %s", SDL_GetError());
            return (size_t)-1;
        }

        data.size = bufferSize;
        data.data = SDL_MapGPUTransferBuffer(g_painter->getDevice(), data.transferBuffer.get(), false);
    }
    return index;
}

SDL_GPUBuffer* RenderBuffer::getBuffer() const
{
    int frameIndex = g_painter->getFrameIndex() % m_buffers.size();
    return m_buffers[frameIndex].vertexBuffer.get();
}

void RenderBuffer::upload()
{
    int frameIndex = g_painter->getFrameIndex() % m_buffers.size();

    Data& data = m_buffers[frameIndex];
    if(!data.transferBuffer)
        return;

    SDL_GPUCopyPass *cpass = SDL_BeginGPUCopyPass(g_painter->getGPUCommand().getCommand());

    SDL_GPUTransferBufferLocation loc;
    loc.transfer_buffer = data.transferBuffer.get();
    loc.offset = 0;

    SDL_GPUBufferRegion dest;
    dest.buffer = data.vertexBuffer.get();
    dest.size = (uint32_t)m_size;
    dest.offset = 0;

    SDL_UploadToGPUBuffer(cpass, &loc, &dest, false);
    SDL_EndGPUCopyPass(cpass);
}
