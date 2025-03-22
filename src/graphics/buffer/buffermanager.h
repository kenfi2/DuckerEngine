#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "bufferchunk.h"

#include <graphics/drawbuffer.h>
#include <graphics/program.h>

struct SolidVertex { float x, y, z, r, g, b, a; };
struct TextureVertex { float x, y, z, u, v; };

struct BaseShaderHash {
    virtual size_t getId() const = 0;
    virtual size_t getSize() const = 0;
};

template<typename T>
struct ShaderHash : public BaseShaderHash {
    size_t getId() const override { return typeid(T).hash_code(); }
    size_t getSize() const override { return sizeof(T); }
};

class BufferData {
public:
    BufferData(const TexturePtr& texture, Program* program, BufferChunk* chunk, uint32_t count) :
        m_texture(texture), m_program(program), m_chunk(chunk), m_offset(chunk->getOffset()), m_count(count) { }

    TexturePtr getTexture() const { return m_texture; }
    DrawBuffer* getDrawBuffer() const { return m_chunk->getDrawBuffer(); }
    Program* getProgram() const { return m_program; }

    uint32_t getOffset() const { return m_offset; }
    uint32_t getCount() const { return m_count; }

private:
    TexturePtr m_texture;
    Program* m_program;
    BufferChunk* m_chunk;
    uint32_t m_offset;
    uint32_t m_count;
};

class BufferManager {
public:
    bool init(const std::string& gpuDriver);
    void clear();

    template<typename T>
    T* acquireBuffer(uint32_t count, PrimitiveType primitiveType, TexturePtr texture = nullptr);

    void map();

    void upload(SDL_GPUCommandBuffer* commandBuffer);

    void reset();

    const std::vector<BufferData>& getBuffers() const { return m_buffers; }

private:
    std::vector<BufferData> m_buffers;
    std::vector<BufferChunk> m_chunks;
};

template<typename T>
T* BufferManager::acquireBuffer(uint32_t count, PrimitiveType primitiveType, TexturePtr texture)
{
    size_t requiredSize = count * sizeof(T);
    BufferChunk* chunk = nullptr;
    for(BufferChunk& ch : m_chunks) {
        if(ch.getCount() + requiredSize < MAX_BUFFER_SIZE) {
            chunk = &ch;
            break;
        }
    }

    if(!chunk) {
        m_chunks.emplace_back();
        chunk = &m_chunks.back();
    }

    Program* program = g_programs.get(primitiveType, ShaderHash<T>().getId());
    m_buffers.emplace_back(texture, program, chunk, count);
    return chunk->append<T>(count);
}

#endif
