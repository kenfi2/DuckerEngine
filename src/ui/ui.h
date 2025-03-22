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

    void draw(PointI offset);

    void resize(int width, int height);

    void addChild(const RectI& rect, const Color& color);
    void clearChildren() { m_children.clear(); }

public:
    void setSize(const SizeI& size) { resize(size.w, size.h); }
    void setRect(const RectI& rect) { m_rect = rect; }
    void setColor(const Color& color) { m_color = color; }

private:
    std::vector<UIWidget*> m_children;
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

private:
    UIWidget* m_rootWidget;

};

#endif
