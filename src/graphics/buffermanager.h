#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <graphics/shaders/program.h>

template<typename T>
class DuckerVector {
public:
    DuckerVector() {
        m_size = 0;
        m_data.resize(2048);
    }

    size_t add(size_t count) {
        size_t index = m_size;
        m_size += count;
        if(m_size > m_data.size()) {
            size_t newCapacity = std::max(m_size * 3 / 2, m_data.size() + 2048);
            m_data.resize(newCapacity);
        }
        return index;
    }

    T& emplace_back() {
        size_t index = add(1);
        return m_data[index];
    }

    void reset() { m_size = 0; }
    size_t size() const { return m_size; }
    const T* data() const { return m_data.data(); }

    T& operator[](size_t index) { return m_data[index]; }

    auto begin() { return m_data.begin(); }
    auto end() { return m_data.end() - m_size; }

    auto begin() const { return m_data.begin(); }
    auto end() const { return m_data.end() - m_size; }

private:
    std::vector<T> m_data;
    size_t m_size;
};

struct DrawCommand {
    DrawCommand() = default;
    DrawCommand(size_t vertexCount, size_t offset, PrimitiveType type, TexturePtr texture = nullptr) : vertexCount(vertexCount), offset(offset), type(type), texture(texture) { }

    size_t vertexCount = 0;
    size_t offset = 0;
    PrimitiveType type = PrimitiveTypeTriangleList;
    TexturePtr texture = nullptr;

    void bindTexture(SDL_GPURenderPass* renderPass);
};

class GPUCommand;
class RenderBuffer;
class BufferManager {
public:
    BufferManager();
    ~BufferManager();

    VertexBuffer* add(size_t count, PrimitiveType type, TexturePtr texture = nullptr);
    void reset();

    SDL_GPUTexture* getTexture() const { return m_texture; }
    void setTexture(SDL_GPUTexture* texture) { m_texture = texture; }
    
    void addPendingTexture(const TexturePtr& texture) { m_pendingTextures.push_back(texture); }
    void uploadPendingTextures(SDL_GPUCommandBuffer* commandBuffer);

    SDL_GPUBuffer* getBuffer(uint32_t frameIndex);
    void upload(uint32_t frameIndex);

    auto begin() { return m_drawCommands.begin(); }
    auto end() { return m_drawCommands.end(); }

    auto begin() const { return m_drawCommands.begin(); }
    auto end() const { return m_drawCommands.end(); }

private:
    DuckerVector<VertexBuffer> m_vertexBuffer;
    DuckerVector<DrawCommand> m_drawCommands;
    std::vector<TexturePtr> m_pendingTextures;
    RenderBufferPtr m_renderBuffer = nullptr;
    SDL_GPUTexture* m_texture = nullptr;
};

#endif
