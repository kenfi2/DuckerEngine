#ifndef UI_H
#define UI_H

#include <utils/include.h>
#include <utils/color.h>
#include <utils/rect.h>
#include <utils/point.h>
#include <utils/size.h>

#include <vector>

class DrawBuffer;

class UIWidget {
public:
    UIWidget();
    ~UIWidget();

    void destroy();

    void draw(PointF offset = PointF(0, 0));

    void resize(int width, int height);

    void addChild(const RectI& rect, const Color& color);

    TexturePtr getTexture() const { return m_texture; }
    const std::vector<UIWidget*>& getChildren() const { return m_children; }
    uint32_t getChildCount() const { return (uint32_t)m_children.size(); }
    void clearChildren() { m_children.clear(); }

public:
    void setSize(const SizeI& size) { resize(size.w, size.h); }

    RectI getRect() const { return m_rect; }
    void setRect(const RectI& rect) { m_rect = rect; }

    Color getColor() const { return m_color; }
    void setColor(const Color& color) { m_color = color; }

private:
    std::vector<UIWidget*> m_children;
    FrameBufferPtr m_frameBuffer = nullptr;
    TexturePtr m_texture;
    RectI m_rect;
    Color m_color;
    bool m_update = false;
};

class UIManager {
public:
    void init();
    void terminate();

    void render();
    void resize(const SizeI& size);

    UIWidget* getRootWidget() const { return m_rootWidget; }

private:
    UIWidget* m_rootWidget;

};

#endif
