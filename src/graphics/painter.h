#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <list>
#include <functional>
#include <queue>

#include "frametimer.h"
#include "buffermanager.h"

class UIWidget;
class Window;
class RenderBuffer;

enum Graphics {
    FramesInFlight = 2
};

class GPUCommand {
public:
    GPUCommand() : m_commandBuffer(nullptr), m_width(0), m_height(0) { }
    ~GPUCommand();

    bool acquire();
    
    void cancel();

    void submit(bool wait);
    
    SDL_GPUCommandBuffer* getCommand() const { return m_commandBuffer; }
    SDL_GPUTexture* acquireSwapchain();

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

private:
    SDL_GPUCommandBuffer* m_commandBuffer;
    uint32_t m_width, m_height;
};

class Painter {
public:
    Painter() = default;

    std::string getName() { return m_gpuDriver; }

    virtual bool create();
    virtual void destroy();

    void bind() { }
    void unbind() { }
    void refresh();
    void refreshContext() { }
    void reset();
    void resetDevice() { }

    virtual bool beginRender();
    virtual void endRender();
    void flushRender();
    virtual void swapBuffers();

	void pushState(bool doReset = false);
	void popState(bool doReset = false);

    void clear(const Color& color);

    void drawPoint(const PointF& point);
    void drawPoint(const PointI& point) { drawPoint(point.toPointF()); }
    void drawPoints(const std::vector<PointF>& points);
    void drawPoints(const std::vector<PointI>& points);

    void drawLine(const PointF& a, const PointF& b);
    void drawLine(const PointI& a, const PointI& b) { drawLine(a.toPointF(), b.toPointF()); }
    void drawLines(const std::vector<PointF>& lines);
    void drawLines(const std::vector<PointI>& lines);
    void drawLineStrip(const std::vector<PointF>& lines);
    void drawLineStrip(const std::vector<PointI>& lines);

    void drawTriangle(const PointF& a, const PointF& b, const PointF& c);
    void drawTriangle(const PointI& a, const PointI& b, const PointI& c) { drawTriangle(a.toPointF(), b.toPointF(), c.toPointF()); }
    void drawTriangles(const std::vector<PointF>& points, TriangleDrawMode mode);
    void drawTriangles(const std::vector<PointI>& points, TriangleDrawMode mode);
    void drawFilledTriangle(const PointF& a, const PointF& b, const PointF& c);
    void drawFilledTriangle(const PointI& a, const PointI& b, const PointI& c) { drawFilledTriangle(a.toPointF(), b.toPointF(), c.toPointF()); }
    void drawFilledTriangles(const std::vector<PointF>& points, TriangleDrawMode mode);
    void drawFilledTriangles(const std::vector<PointI>& points, TriangleDrawMode mode);

	void drawRect(const RectF& rect);
    void drawRect(const RectI& rect) { drawRect(rect.toRectF()); }
    void drawRects(const std::vector<RectF>& rects);
    void drawRects(const std::vector<RectI>& rects);
    void drawFilledRect(const RectF& rect);
    void drawFilledRect(const RectI& rect) { drawFilledRect(rect.toRectF()); }
    void drawFilledRects(const std::vector<RectF>& rects);
    void drawFilledRects(const std::vector<RectI>& rects);

    void drawTexturedRect(const RectI& destRect, const TexturePtr& texture, const RectI& srcRect) { drawTexturedRect(destRect.toRectF(), texture, srcRect); }
    void drawTexturedRect(const RectF& destRect, const TexturePtr& texture, const RectI& srcRect);
    void drawTexturedRect(const RectI& destRect, const TexturePtr& texture);
    void drawTexturedRect(const RectF& destRect, const TexturePtr& texture);

    void drawTexturedRects(const std::vector<RectI>& destRects, const TexturePtr& texture, const std::vector<RectI>& srcRects);
    void drawTexturedRects(const std::vector<RectF>& destRects, const TexturePtr& texture, const std::vector<RectI>& srcRects);

    virtual void draw();

public:
    void genFrameBuffer(uint32_t* fboId);
    void deleteFrameBuffer(uint32_t* fboId);
    void bindFrameBuffer(uint32_t fboId);
    void setFrameBufferTexture(uint32_t fboId, const TexturePtr& texture);
    void addPendingTexture(const TexturePtr& texture) { m_frameBuffers[m_currentFBO]->addPendingTexture(texture); }
    SDL_GPUDevice* getDevice() const { return m_gpuDevice; }
    GPUCommand& getGPUCommand() { return m_gpuCommand; }

    PainterState* getCurrentState();
	void translate(float x, float y);

    void setColor(const Color& color);
    SizeI getResolution() const;
    void setResolution(const SizeI& resolution);
    void setViewport(const RectI& viewport);
    void setBlendMode(BlendMode blendMode);

protected:
    GPUCommand m_gpuCommand;
    std::unordered_map<uint32_t, BufferManagerPtr> m_frameBuffers;
	std::string m_gpuDriver;
	SDL_GPUDevice* m_gpuDevice = nullptr;
    uint32_t m_currentFBO = 0;
    uint32_t m_fboController = 0;
    std::queue<uint32_t> m_fboIds;
    int m_frames = 0;
    int m_frameIndex = 0;

protected:
    void resetProjectionMatrix();
    void resetTransformMatrix();
    void resetColor() { setColor(Color(255, 255, 255)); }
    void resetBlendMode() { setBlendMode(BlendMode_Blend); }

    void setProjectionMatrix(const Matrix3& projectionMatrix);
    void setTransformMatrix(const Matrix3& transformMatrix);

    PainterState m_state;
    PainterState m_olderStates[10];
    std::vector<PainterState> m_states;
    int m_oldStateIndex = 0;

    int m_drawnPrimitives = 0;
    int m_painterFlags = 0;
    size_t m_stateId = 0;
    uint32_t m_drawCalls = 0;
    uint32_t m_lastDrawnPrimitives = 0;
    uint32_t m_lastDrawCalls = 0;
};

extern Painter* g_painter;

#endif
