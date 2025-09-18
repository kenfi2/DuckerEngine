#include "framebuffer.h"
#include "painter.h"

#include <graphics/texture/texture.h>

FrameBufferPtr FrameBuffer::m_temporaryFrameBuffer;
uint32_t FrameBuffer::m_boundFbo = 0;

FrameBuffer::FrameBuffer()
{
    m_prevBoundFbo = 0;
    m_fbo = 0;
}

FrameBuffer::~FrameBuffer()
{
    if(m_fbo != 0)
        g_painter->deleteFrameBuffer(&m_fbo);
}

FrameBufferPtr FrameBuffer::getTemporaryFrameBuffer()
{
    if(!m_temporaryFrameBuffer)
        m_temporaryFrameBuffer = FrameBufferPtr(new FrameBuffer);
    return m_temporaryFrameBuffer;
}

void FrameBuffer::destroyTemporaryFrameBuffer()
{
    m_temporaryFrameBuffer = nullptr;
}

void FrameBuffer::resize(const SizeI &size)
{
    if(!size.isValid())
        return;

    if(m_texture && size == m_wantedSize)
        return;
    
    g_painter->genFrameBuffer(&m_fbo);

    m_texture = TexturePtr(new Texture);
    m_texture->setSize(size);
    m_texture->setSmooth(m_smooth);
    m_size = m_texture->getSize();
    m_invalidated = true;
    m_wantedSize = size;
    m_texture->generate();
    g_painter->setFrameBufferTexture(m_fbo, m_texture);

}

void FrameBuffer::bind()
{
    g_painter->pushState(true);
    g_painter->setResolution(m_size);
    g_painter->setViewport(RectI(0, 0, m_size));
    // g_painter->resetClipRect();
    g_painter->bindFrameBuffer(m_fbo);
    m_prevBoundFbo = m_boundFbo;
    m_boundFbo = m_fbo;
}

void FrameBuffer::release()
{
    g_painter->draw();
    g_painter->bindFrameBuffer(m_prevBoundFbo);
    m_boundFbo = m_prevBoundFbo;
    g_painter->popState();
}

void FrameBuffer::draw(const RectF &dest, const RectI &src)
{
    g_painter->drawTexturedRect(dest, m_texture, src);
}

void FrameBuffer::draw(const RectF &dest)
{
    draw(dest, RectI(0, 0, m_size));
}

void FrameBuffer::draw()
{
    draw(RectI(0, 0, g_painter->getResolution()), RectI(0, 0, m_size));
}
