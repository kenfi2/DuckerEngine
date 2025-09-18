#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <utils/include.h>
#include <graphics/shaders/program.h>

class RenderBuffer
{
    struct Data {
        Data() { }

        SDL_GPUBuffer* vertexBuffer = nullptr;
        SDL_GPUTransferBuffer* transferBuffer = nullptr;
        size_t vertexSize = 0;
        size_t transferSize = 0;
    };
public:
    RenderBuffer();

    ~RenderBuffer();

    SDL_GPUBuffer* acquireVertexBuffer(size_t size, int frameIndex);
    void upload(const VertexBuffer* vertexData, size_t size, int frameIndex);

private:
    std::vector<Data> m_buffers;
};

#endif
