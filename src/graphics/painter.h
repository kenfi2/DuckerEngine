#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <utils/include.h>
#include <utils/color.h>
#include <utils/point.h>
#include <utils/rect.h>
#include <utils/matrix.h>

#include <list>
#include <functional>

#include <graphics/buffer/buffermanager.h>
#include "frametimer.h"

class Window;

struct PainterState {
	Matrix3 m_projectionMatrix;
	Matrix3 m_transformMatrix;
};

enum TriangleDrawMode {
    DrawTriangles = 1000,
    DrawTriangleFan,
    DrawTriangleStrip
};

enum {
    UploadFrame = 0,
    DrawFrame = 1,
    LastFrame
};

class GPUCommand {
public:
    GPUCommand() : m_commandBuffer(nullptr), m_swapchainTexture(nullptr), m_width(0), m_height(0) { }
    ~GPUCommand();

    bool acquire(bool swapchain = false);
    
    void cancel();

    void submit(bool waitFences = true);
    
    SDL_GPUCommandBuffer* command() const { return m_commandBuffer; }

    SDL_GPUTexture* swapchain() const { return m_swapchainTexture; }

private:
    SDL_GPUCommandBuffer* m_commandBuffer;
    SDL_GPUTexture* m_swapchainTexture;
    uint32_t m_width, m_height;
};

class Painter : public PainterState {
public:
	bool init();
	void destroy();

    SDL_GPUDevice* getDevice() const { return m_gpuDevice; }

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

	void beginFrame();
    void endFrame();

	void clear();
	void draw();
public:
	void translate(float x, float y);

	void pushState(bool doReset = false);
	void popState(bool doReset = false);

    void setColor(const Color& color) { m_color = color; }

    int getLostFrames() const { return m_lostFrames; }

private:
	void reset();
	void resetProjectionMatrix();
	void resetTransformMatrix();
    void resetColor() { setColor(Color(255, 255, 255)); }
	void updateProjectionTransformMatrix();

	void setProjectionMatrix(const Matrix3& projectionMatrix);
	void setTransformMatrix(const Matrix3& transformMatrix);

    BufferManager m_bufferManager;
	GPUCommand m_gpuCommand;
	float m_projectionTransformMatrix[16];
	PainterState m_painterStates[10];
	std::string m_gpuDriver;
	SDL_GPUDevice* m_gpuDevice = nullptr;
    Color m_color;
	int m_painterStateIndex = 0;
    int m_lostFrames = 0;
    int m_currentFrame = 0;
};

extern Painter* g_painter;

#endif
