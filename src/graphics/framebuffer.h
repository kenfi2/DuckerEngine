#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <utils/include.h>
#include <utils/size.h>
#include <utils/rect.h>

class FrameBuffer : public std::enable_shared_from_this<FrameBuffer> {
public:
    FrameBuffer();
    ~FrameBuffer();
    
    static FrameBufferPtr getTemporaryFrameBuffer();
    static void destroyTemporaryFrameBuffer();

    TexturePtr getTexture() const { return m_texture; }

    void resize(const SizeI& size);
    void bind();
    void release();

    void draw(const RectF& dest, const RectI& src);
    void draw(const RectF& dest);
    void draw(const RectI& dest, const RectI& src) { draw(dest.toRectF(), src); }
    void draw(const RectI& dest) { draw(dest.toRectF()); }
    void draw();

private:
    static FrameBufferPtr m_temporaryFrameBuffer;
    static uint32_t m_boundFbo;
    uint32_t m_fbo;
    uint32_t m_prevBoundFbo;

    TexturePtr m_texture;
    SizeI m_size;
    SizeI m_wantedSize;
    bool m_invalidated = false;
    bool m_smooth = false;
};

#endif
