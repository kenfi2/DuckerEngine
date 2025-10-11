#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <utils/include.h>
#include <graphics/shaders/program.h>

struct GPUBufferRelease {
    void operator()(SDL_GPUBuffer* handle) const;
	void operator()(SDL_GPUTransferBuffer* handle) const;
    void operator()(SDL_GPUTexture* texture) const;
    void operator()(SDL_GPUSampler* sampler) const;
};

using GPUBufferPtr = std::unique_ptr<SDL_GPUBuffer, GPUBufferRelease>;
using GPUTransferBufferPtr = std::unique_ptr<SDL_GPUTransferBuffer, GPUBufferRelease>;
using GPUTexturePtr = std::unique_ptr<SDL_GPUTexture, GPUBufferRelease>;
using GPUSamplerPtr = std::unique_ptr<SDL_GPUSampler, GPUBufferRelease>;

class RenderBuffer
{
    struct Data {
        Data() { }

        void* data = nullptr;
        GPUBufferPtr vertexBuffer = nullptr;
        GPUTransferBufferPtr transferBuffer = nullptr;
        size_t size = 0;
    };
public:
    RenderBuffer(size_t frames);

    ~RenderBuffer();

    void setExpandSize(size_t expandSize) { m_expandSize = expandSize; }

    void reset() { m_size = 0; }
    size_t add(size_t count);

    template<typename T>
    const T& at(size_t index);

    SDL_GPUBuffer* getBuffer() const;
    void upload();

private:
    size_t m_expandSize;
    size_t m_size;
    std::vector<Data> m_buffers;
};

template <typename T>
inline const T &RenderBuffer::at(size_t index)
{
    int frameIndex = g_painter->getFrameIndex() % m_buffers.size();

    Data& data = m_buffers[frameIndex];
    return reinterpret_cast<T&>(((unsigned char*)data.data)[index]);
}

#endif
