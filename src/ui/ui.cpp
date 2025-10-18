#include "ui.h"

#include <engine.h>
#include <graphics/texture/texture.h>
#include <graphics/image.h>
#include <graphics/framebuffer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

UIWidget::UIWidget()
{
    m_color = Color(255, 0, 255, 255);
}

TexturePtr texture2;

UIWidget::~UIWidget()
{
    texture2 = nullptr;
}

void UIWidget::destroy()
{
    for(UIWidget* child : m_children)
        delete child;
    m_children.clear();
}

#define M_PI 3.14159265358979323846

PointF animatedOffset(float amplitude = 10.f, float speed = 1.f) {
    using clock = std::chrono::steady_clock;
    static auto start = clock::now();

    auto now = clock::now();
    float t = std::chrono::duration<float>(now - start).count();

    float dx = std::sin(t * speed) * amplitude;
    float dy = std::cos(t * speed) * amplitude;

    return PointF(dx, dy);
}

static UIWidget* g_rootWidget = nullptr;

void UIWidget::draw(PointF offset)
{
    RectF drawRect = m_rect.toRectF().translated(offset);
/* 
    if(this == g_rootWidget) {
        m_frameBuffer->bind();
        g_painter->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
    }
 */
    g_painter->setColor(m_color);
    g_painter->drawFilledRect(drawRect);

    for(UIWidget* child : m_children) {
        PointF childOffset = offset + drawRect.topLeft().toPointF();
        childOffset += animatedOffset(10, 4);
        child->draw(childOffset);
    }
   /*  if(this == g_rootWidget) {
        m_frameBuffer->release();
        m_frameBuffer->draw(drawRect);
    } */

    if(this == g_rootWidget)
        g_painter->drawTexturedRect(RectI(50, 50, m_texture->getSize()), m_texture);
}

void UIWidget::resize(int width, int height)
{
    m_rect = RectI(m_rect.topLeft(), SizeI(width, height));
    m_update = true;
    m_frameBuffer = FrameBufferPtr(new FrameBuffer());
    m_frameBuffer->resize(m_rect.size());
    
}

void UIWidget::addChild(const RectI& rect, const Color& color)
{
    UIWidget *widget = new UIWidget;
    widget->setRect(rect);
    widget->setColor(color);

    m_children.push_back(widget);
}

void UIManager::init()
{
    m_rootWidget = new UIWidget;
    g_rootWidget = m_rootWidget;
    for(int i = 0; i < 10000; ++i) {
        m_rootWidget->addChild(RectI(i % 800, i % 800, i % 100, i % 100), Color(123, i % 127, i % 255));
    }
    if(!m_rootWidget->m_texture) {
        int w, h, channels;
        unsigned char* data = stbi_load("tts.png", &w, &h, &channels, 4);
        if(data) {
            m_rootWidget->m_texture = TexturePtr(new Texture());

            std::vector<uint32_t> dd(w * h);
            memcpy(dd.data(), data, w*h*4);

            ImagePtr image = ImagePtr(new Image(dd, SizeI(w, h)));
            m_rootWidget->m_texture->uploadPixels(image);

            stbi_image_free(data);
        }
        else
            std::cout << "Falha ao carregar imagem:\n";
    }
}

void UIManager::terminate()
{
    m_rootWidget->destroy();
}

void UIManager::render()
{
    m_rootWidget->draw(PointF(0, 0));
}

void UIManager::resize(const SizeI &size)
{
    m_rootWidget->setSize(size + SizeI(1, 1));
}
