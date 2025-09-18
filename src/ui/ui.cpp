#include "ui.h"

#include <engine.h>
#include <graphics/texture/texture.h>
#include <graphics/image.h>
#include <graphics/framebuffer.h>

UIWidget::UIWidget()
{
    m_color = Color(255, 0, 255, 255);
}

UIWidget::~UIWidget()
{
}

void UIWidget::destroy()
{
    for(UIWidget* child : m_children)
        delete child;
    m_children.clear();
}

#define M_PI 3.14159265358979323846

void UIWidget::draw(PointF offset)
{
    RectF drawRect = m_rect.toRectF().translated(offset);
    {
        // FrameBufferPtr frameBuffer = FrameBufferPtr(new FrameBuffer());
        // frameBuffer->resize(drawRect.size().toSize());
        // frameBuffer->bind();
        g_painter->setColor(m_color);
    
        g_painter->drawFilledRect(drawRect);
    
        // frameBuffer->release();
        // frameBuffer->draw(drawRect);
    }

    for(UIWidget* child : m_children) {
        PointF childOffset = offset + drawRect.topLeft().toPointF();
        child->draw(childOffset);
    }
}

void UIWidget::resize(int width, int height)
{
    m_rect = RectI(m_rect.topLeft(), SizeI(width, height));
    m_update = true;
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
    for(int i = 0; i < 10000; ++i) {
        m_rootWidget->addChild(RectI(i % 800, i % 800, i % 100, i % 100), Color(123, i % 127, i % 255));
    }
}

void UIManager::terminate()
{
    m_rootWidget->destroy();
}

void UIManager::render()
{
    m_rootWidget->draw(PointF(5, 5));
}

void UIManager::resize(const SizeI &size)
{
    m_rootWidget->setSize(size);
}
