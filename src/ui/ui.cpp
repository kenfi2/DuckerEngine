#include "ui.h"

#include <engine.h>
#include <graphics/texture/texture.h>
#include <graphics/image.h>

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

void UIWidget::draw(PointI offset)
{
    RectI drawRect = m_rect.translated(offset);
    g_painter->setColor(m_color);
    g_painter->drawFilledRect(drawRect);
    for(UIWidget* child : m_children) {
        PointI childOffset = offset + drawRect.topLeft();
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

static int CHILD = 0;

void UIManager::init()
{
    m_rootWidget = new UIWidget;
}

void UIManager::terminate()
{
    m_rootWidget->destroy();
}

void UIManager::render()
{
    m_rootWidget->draw(PointI(5, 5));
}

void UIManager::resize(const SizeI &size)
{
    m_rootWidget->setSize(size);
}
