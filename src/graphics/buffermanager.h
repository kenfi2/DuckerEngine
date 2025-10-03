#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <graphics/shaders/program.h>

#include <utils/include.h>
#include <utils/color.h>
#include <utils/point.h>
#include <utils/rect.h>
#include <utils/size.h>
#include <utils/matrix.h>

template<typename _T>
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

    void remove(size_t count) {
        m_size -= count;
    }

    _T& emplace_back() {
        size_t index = add(1);
        return m_data[index];
    }

    void reset() { m_size = 0; }
    size_t size() const { return m_size; }
    const _T* data() const { return m_data.data(); }

    template<typename T>
    T& at(size_t index) { return reinterpret_cast<T&>(m_data[index]); }

    _T& operator[](size_t index) { return m_data[index]; }

    auto begin() { return m_data.begin(); }
    auto end() { return m_data.begin() + m_size; }

    auto begin() const { return m_data.begin(); }
    auto end() const { return m_data.begin() + m_size; }

private:
    std::vector<_T> m_data;
    size_t m_size;
};

enum {
    MustUpdateColor = 1 << 0,
    MustUpdateLineWidth = 1 << 1,
    MustUpdatePointSize = 1 << 2,
    MustUpdateBlendMode = 1 << 3,
    MustUpdateClipRect = 1 << 4,
    MustUpdateResolution = 1 << 5,
    MustUpdateViewport = 1 << 6,
    MustUpdateProjectionTransformMatrix = 1 << 7,
    MustUpdateProgram = 1 << 8,
    MustUpdateProgramResource = MustUpdateProgram | MustUpdateColor | MustUpdateLineWidth | MustUpdatePointSize | MustUpdateBlendMode | MustUpdateResolution | MustUpdateProjectionTransformMatrix
};

struct PainterState {
    PainterState() = default;

    void copy(const PainterState& state) { *this = state; }

    Program* program = nullptr;
    SizeI resolution;
    RectI viewport;
    Matrix3 transformMatrix;
    Matrix3 projectionMatrix;
    Color color = 0xffffffff;
    float opacity = 1.0f;
    float lineWidth = 1.0f;
    float pointSize = 1.0f;
    BlendMode blendMode = BlendMode_Blend;
    RectI clipRect;
    size_t id = 0;
    int flags = 0;
};

struct DrawCommand {
    DrawCommand() = default;

    size_t vertexCount = 0;
    size_t offset = 0;
    size_t state = 0;
    PrimitiveType type = LastPrimitiveType;
    TexturePtr texture = nullptr;

    void bindTexture(SDL_GPURenderPass* renderPass);
};

class GPUCommand;
class RenderBuffer;
class BufferManager {
public:
    BufferManager();
    ~BufferManager();

    template<typename T>
    T* add(size_t count, PrimitiveType type, PainterState* state, TexturePtr texture = nullptr);
    void clear(const Color& color);
    void reset();

    SDL_GPUTexture* getTexture() const { return m_texture; }
    void setTexture(const TexturePtr& texture);
    
    void addPendingTexture(const TexturePtr& texture) { m_pendingTextures.push_back(texture); }
    void uploadPendingTextures(SDL_GPUCommandBuffer* commandBuffer);

    SDL_GPUBuffer* getBuffer(uint32_t frameIndex);
    void upload(uint32_t frameIndex);

    auto begin() { return m_drawCommands.begin(); }
    auto end() { return m_drawCommands.end(); }

    auto begin() const { return m_drawCommands.begin(); }
    auto end() const { return m_drawCommands.end(); }

    const auto& getVertexBuffer() const { return m_vertexBuffer; }
    const Color& getClearColor() const { return m_clearColor; }

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }

private:
    DuckerVector<unsigned char> m_vertexBuffer;
    DuckerVector<DrawCommand> m_drawCommands;
    std::vector<TexturePtr> m_pendingTextures;
    RenderBufferPtr m_renderBuffer = nullptr;
    SDL_GPUTexture* m_texture = nullptr;
    Color m_clearColor;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

template<typename T>
inline T* BufferManager::add(size_t count, PrimitiveType type, PainterState* state, TexturePtr texture)
{
    size_t index = m_vertexBuffer.add(count * sizeof(T));

    DrawCommand& drawCommand = m_drawCommands.emplace_back();
    drawCommand.vertexCount = count;
    drawCommand.offset = index / sizeof(T);
    drawCommand.texture = texture;
    drawCommand.state = state->id;
    drawCommand.type = type;
    return &m_vertexBuffer.at<T&>(index);
}

#endif
