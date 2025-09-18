#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <utils/include.h>
#include <utils/color.h>
#include <utils/point.h>
#include <utils/rect.h>
#include <utils/size.h>
#include <utils/matrix.h>

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

private:
    SDL_GPUCommandBuffer* m_commandBuffer;
    uint32_t m_width, m_height;
};

class Painter {
    struct PainterState {
        Program* program;
        SizeI resolution;
        RectI viewport;
        Matrix3 transformMatrix;
        Matrix3 projectionMatrix;
        Color color;
        float opacity;
        float lineWidth;
        float pointSize;
        BlendMode blendMode;
        RectI clipRect;
    };

public:
    Painter() = default;

    std::string getName() { return m_gpuDriver; }

    bool create();
    void destroy();

    void bind() { }
    void unbind() { }
    void refresh();
    void refreshContext() { }
    void reset();
    void resetDevice() { }

    bool beginRender();
    void endRender();
    void flushRender();
    void swapBuffers();

	void pushState(bool doReset = false);
	void popState(bool doReset = false);

    void clear(const Color&) { }

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

    void draw();

public:
    void genFrameBuffer(uint32_t* fboId);
    void deleteFrameBuffer(uint32_t* fboId);
    void bindFrameBuffer(uint32_t fboId);
    void setFrameBufferTexture(uint32_t fboId, const TexturePtr& texture);
    void addPendingTexture(const TexturePtr& texture) { m_frameBuffers[m_currentFBO]->addPendingTexture(texture); }
    SDL_GPUDevice* getDevice() const { return m_gpuDevice; }
    GPUCommand& getGPUCommand() { return m_gpuCommand; }

    void preDraw(SDL_GPURenderPass* renderPass);
	void translate(float x, float y);

    void setColor(const Color& color) { m_color = color; }
    SizeI getResolution() const { return m_resolution; }
    void setResolution(const SizeI& resolution);
    void setViewport(const RectI& viewport);

private:
    GPUCommand m_gpuCommand;
    std::unordered_map<uint32_t, BufferManagerPtr> m_frameBuffers;
	std::string m_gpuDriver;
	SDL_GPUDevice* m_gpuDevice = nullptr;
    uint32_t m_currentFBO = 0;
    uint32_t m_fboController = 0;
    std::queue<uint32_t> m_fboIds;
    int m_frames = 0;
    int m_frameIndex = 0;

private:
    void resetProjectionMatrix();
    void resetTransformMatrix();
    void resetColor() { setColor(Color(255, 255, 255)); }
    void updateProgram();
    void updateProjectionTransformMatrix();
    void updateResolution(SDL_GPURenderPass* renderPass);
    void updateViewport(SDL_GPURenderPass* renderPass);

    void setProjectionMatrix(const Matrix3& projectionMatrix);
    void setTransformMatrix(const Matrix3& transformMatrix);

	float m_projectionTransformMatrix[16];

	int m_painterStateIndex = 0;
    bool m_mustUpdateResolution = true;
    bool m_mustUpdateViewport = false;

    PainterState m_olderStates[10];
    int m_oldStateIndex = 0;

    SizeI m_resolution;
    RectI m_viewport;
    Color m_color = 0xffffffff;
    float m_opacity = 1.0f;
    float m_lineWidth = 1.0f;
    float m_pointSize = 1.0f;
    RectI m_clipRect;
    BlendMode m_blendMode = BlendMode_Blend;
    Matrix3 m_projectionMatrix;
    Matrix3 m_transformMatrix;

    int m_drawnPrimitives = 0;
    uint32_t m_drawCalls = 0;
    uint32_t m_lastDrawnPrimitives = 0;
    uint32_t m_lastDrawCalls = 0;
};

extern Painter* g_painter;

#endif
