#ifndef BUFFERCHUNK_H
#define BUFFERCHUNK_H

#include <utils/include.h>

constexpr size_t MAX_BUFFER_SIZE = 8 * 1024 * 1024;

class DrawBuffer;

class BufferChunk {
public:
    explicit BufferChunk();
    ~BufferChunk();

    DrawBuffer* getDrawBuffer() const { return m_drawBuffer; }

    uint32_t getOffset() const { return m_offset; }
    uint32_t getCount() const { return m_count; }

    template<typename T>
    T* append(uint32_t count);

    void map();
    void upload(SDL_GPUCommandBuffer* commandBuffer);
    void clear();

private:
    DrawBuffer* m_drawBuffer = nullptr;
    SDL_GPUTransferBuffer* m_transferBuffer = nullptr;
    void* m_data;
    uint32_t m_offset = 0;
    uint32_t m_count = 0;
};

template <typename T>
inline T* BufferChunk::append(uint32_t count)
{
    T* data = reinterpret_cast<T*>(&((char*)m_data)[m_count]);
    m_offset += count;
    m_count += count * sizeof(T);
    return data;
}

#endif
