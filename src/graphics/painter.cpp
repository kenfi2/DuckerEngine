#include <window.h>

#include <graphics/shaders/shaders.h>

#include "engine.h"
#include "painter.h"
#include "framebuffer.h"
#include "renderbuffer.h"

#include <graphics/texture/texture.h>
#include <ui/ui.h>

static SDL_GPUShaderFormat g_shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB;

bool Painter::create()
{
    if(m_gpuDriver.empty()) {
#ifdef WIN32
        m_gpuDriver = "direct3d12";
        // m_gpuDriver = "vulkan";
#else
        m_gpuDriver = "vulkan";
#endif
    }

    bool debugMode;
#ifdef _DEBUG
    debugMode = true;
#else
    debugMode = false;
#endif

    m_gpuDevice = SDL_CreateGPUDevice(g_shaderFormats, false, m_gpuDriver.c_str());
    if(!m_gpuDevice) {
        std::cout << "Failed to create " << m_gpuDriver << " device. Trying with another." << std::endl;
        for(int i = 0; i < SDL_GetNumGPUDrivers(); ++i) {
            std::string gpuDriver(SDL_GetGPUDriver(i));
            if(gpuDriver == m_gpuDriver)
                continue;
            m_gpuDriver = gpuDriver;
            m_gpuDevice = SDL_CreateGPUDevice(g_shaderFormats, debugMode, gpuDriver.c_str());
            if(m_gpuDevice)
                break;
        }

        if(!m_gpuDevice) {
            SDL_Log("SDL_CreateGPUDevice: %s", SDL_GetError());
            return false;
        }
    }

    std::cout << "Selected driver: " << m_gpuDriver << std::endl;

    SDL_ClaimWindowForGPUDevice(m_gpuDevice, g_window->getSDLWindow());
    SDL_SetGPUSwapchainParameters(m_gpuDevice, g_window->getSDLWindow(), SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
    SDL_SetGPUAllowedFramesInFlight(m_gpuDevice, FramesInFlight);

    m_frameBuffers.reserve(32);
    m_frameBuffers[0] = std::make_shared<BufferManager>();
    m_states.resize(1);

    reset();

    return g_programs.init(m_gpuDriver);
}

void Painter::destroy()
{
    m_frameBuffers.clear();
    g_programs.clear();
    FrameBuffer::destroyTemporaryFrameBuffer();
    SDL_ReleaseWindowFromGPUDevice(m_gpuDevice, g_window->getSDLWindow());
    SDL_DestroyGPUDevice(m_gpuDevice);
    m_gpuDevice = nullptr;
}

void Painter::genFrameBuffer(uint32_t* fboId)
{
    if(!fboId)
        return;
    uint32_t newId;
    if(!m_fboIds.empty()) {
        newId = m_fboIds.front();
        m_fboIds.pop();
    } else
        newId = ++m_fboController;

    *fboId = newId;
    m_frameBuffers[*fboId] = std::make_shared<BufferManager>();
}

void Painter::deleteFrameBuffer(uint32_t* fboId)
{
    if(!fboId || *fboId == 0)
        return;
    m_fboIds.push(*fboId);
    m_frameBuffers[*fboId] = nullptr;
}

void Painter::bindFrameBuffer(uint32_t fboId)
{
    m_currentFBO = fboId;
    if(fboId != 0)
        m_frameBuffers[fboId]->reset();
}

void Painter::setFrameBufferTexture(uint32_t fboId, const TexturePtr &texture)
{
    if(fboId != 0)
        m_frameBuffers[fboId]->setTexture(texture);
}

PainterState* Painter::getCurrentState()
{
    bool newState = false;
    if(m_painterFlags != 0) {
        ++m_stateId;
        newState = true;
    }
    
    if(m_stateId >= m_states.size())
        m_states.resize(m_stateId * 2);

    PainterState& state = m_states[m_stateId];
    if(newState) {
        state.copy(m_state);
        state.id = m_stateId;
        state.flags = m_painterFlags;
        m_painterFlags = 0;
    }
    return &state;
}

void Painter::translate(float x, float y)
{
    Matrix3 translateMatrix = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
           x,    y, 1.0f
    };

    setTransformMatrix(m_state.transformMatrix * translateMatrix);
}

void Painter::setColor(const Color &color)
{
    if(m_state.color == color)
        return;
    m_state.color = color;
    m_painterFlags |= MustUpdateColor;
}

SizeI Painter::getResolution() const
{
    return m_state.resolution;
}

void Painter::setResolution(const SizeI &resolution)
{
    if(m_state.resolution == resolution)
        return;
    m_state.resolution = resolution;
    resetProjectionMatrix();
}

void Painter::setViewport(const RectI&)
{
    /* if(m_state.viewport == viewport)
        return;
    m_state.viewport = viewport;
    m_painterFlags |= MustUpdateViewport; */
}

void Painter::setBlendMode(BlendMode blendMode)
{
    if(m_state.blendMode == blendMode)
        return;
    m_state.blendMode = blendMode;
    m_painterFlags |= MustUpdateBlendMode;
}

void Painter::reset()
{
    resetColor();
    resetProjectionMatrix();
    resetTransformMatrix();
    resetBlendMode();
}

void Painter::refresh()
{
}

void Painter::resetProjectionMatrix()
{
    float dx = 2.0f / m_state.resolution.w;
    float dy = 2.0f / m_state.resolution.h;

    setProjectionMatrix({
        dx,                       0.0f,                      0.0f,
        0.0f,                      -dy,                      0.0f,
        -1.0f,                     1.0f,                      1.0f
    });
}

void Painter::resetTransformMatrix()
{
    setTransformMatrix(Matrix3());
}

void Painter::setProjectionMatrix(const Matrix3 &projectionMatrix)
{
    if(m_state.projectionMatrix == projectionMatrix)
        return;
    m_state.projectionMatrix = projectionMatrix;
    m_painterFlags |= MustUpdateProjectionTransformMatrix;
}

void Painter::setTransformMatrix(const Matrix3& transformMatrix)
{
    if(m_state.transformMatrix == transformMatrix)
        return;
    m_state.transformMatrix = transformMatrix;
    m_painterFlags |= MustUpdateProjectionTransformMatrix;
}

void Painter::drawPoint(const PointF& point)
{
    static std::vector<PointF> points(1);
    points[0].x = point.x;
    points[0].y = point.y;
    drawPoints(points);
}

void Painter::drawPoints(const std::vector<PointF>& points)
{
    auto* vertexData = m_frameBuffers[m_currentFBO]->add<SolidVertexBuffer>(points.size(), PrimitiveTypePointList, getCurrentState());

    for(uint32_t i = 0; i < points.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = points[i];
        d.x = p.x;
        d.y = p.y;
    }
}

void Painter::drawPoints(const std::vector<PointI>& points)
{
    std::vector<PointF> pointsF(points.size());
    for(uint32_t i = 0; i < pointsF.size(); ++i)
        pointsF[i] = points[i].toPointF();
    drawPoints(pointsF);
}

void Painter::drawLine(const PointF &a, const PointF &b)
{
    static std::vector<PointF> lines(2);
    lines[0] = a;
    lines[1] = b;
    drawLines(lines);
}

void Painter::drawLines(const std::vector<PointF> &lines)
{
    auto* vertexData = m_frameBuffers[m_currentFBO]->add<SolidVertexBuffer>(lines.size(), PrimitiveTypeLineList, getCurrentState());
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
    }
}

void Painter::drawLines(const std::vector<PointI>& lines)
{
    std::vector<PointF> linesF(lines.size());
    for(uint32_t i = 0; i < linesF.size(); ++i)
        linesF[i] = lines[i].toPointF();
    drawLines(linesF);
}

void Painter::drawLineStrip(const std::vector<PointF> &lines)
{
    auto* vertexData = m_frameBuffers[m_currentFBO]->add<SolidVertexBuffer>(lines.size(), PrimitiveTypeLineStrip, getCurrentState());
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
    }
}

void Painter::drawLineStrip(const std::vector<PointI> &lines)
{
    std::vector<PointF> linesF(lines.size());
    for(uint32_t i = 0; i < linesF.size(); ++i)
        linesF[i] = lines[i].toPointF();
    drawLineStrip(linesF);
}

void Painter::drawTriangle(const PointF& a, const PointF& b, const PointF& c)
{
    static std::vector<PointF> points(4);
    points[0] = a;
    points[1] = b;
    points[2] = c;
    points[3] = a;
    drawLineStrip(points);
}

void Painter::drawTriangles(const std::vector<PointF>& points, TriangleDrawMode mode)
{
    if(mode == DrawTriangleStrip) {
        for(uint32_t i = 2; i < points.size(); ++i)
            drawTriangle(points[i-2], points[i-1], points[i]);
    } else if(mode == DrawTriangles) {
        for(uint32_t i = 0; i < points.size(); i += 3)
            drawTriangle(points[i], points[i+1], points[i+2]);
    } else { // triangle fan
        for(uint32_t i = 2; i < points.size(); i++)
            drawTriangle(points[0], points[i-1], points[i]);
    }
}

void Painter::drawTriangles(const std::vector<PointI>& points, TriangleDrawMode mode)
{
    std::vector<PointF> pointsF(points.size());
    for(uint32_t i = 0; i < points.size(); ++i)
        pointsF[i] = points[i].toPointF();
    drawTriangles(pointsF, mode);
}

void Painter::drawFilledTriangle(const PointF &a, const PointF &b, const PointF &c)
{
    static std::vector<PointF> points(3);
    points[0] = a;
    points[1] = b;
    points[2] = c;
    drawFilledTriangles(points, DrawTriangles);
}

void Painter::drawFilledTriangles(const std::vector<PointF> &points, TriangleDrawMode mode)
{
    if(mode == DrawTriangleFan) {
        if(points.size() < 3)
            return;

        uint32_t count = (uint32_t)points.size() - 2;
        std::vector<PointF> triangles(count * 3);
        for(uint32_t i = 0; i < count; ++i) {
            triangles[i * 3 + 0] = points[0];
            triangles[i * 3 + 1] = points[i + 1];
            triangles[i * 3 + 2] = points[i + 2];
        }

        drawFilledTriangles(triangles, DrawTriangles);
        return;
    }

    PrimitiveType primitiveType = PrimitiveTypeTriangleList;
    if(mode == DrawTriangleStrip)
        primitiveType = PrimitiveTypeTriangleStrip;
    else if(mode == DrawTriangles)
        primitiveType = PrimitiveTypeTriangleList;

    auto* vertexData = m_frameBuffers[m_currentFBO]->add<SolidVertexBuffer>(points.size(), primitiveType, getCurrentState());
    for(uint32_t i = 0; i < points.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& point = points[i];
        d.x = point.x;
        d.y = point.y;
    }
}

void Painter::drawFilledTriangles(const std::vector<PointI> &points, TriangleDrawMode mode)
{
    std::vector<PointF> pointsF(points.size());
    for(uint32_t i = 0; i < points.size(); ++i)
        pointsF[i] = points[i].toPointF();
    drawFilledTriangles(pointsF, mode);
}

void Painter::drawRect(const RectF& rect)
{
    static std::vector<PointF> points(5);
    points[0] = rect.topLeft();
    points[1] = rect.topRight();
    points[2] = rect.bottomRight();
    points[3] = rect.bottomLeft();
    points[4] = points[0];
    drawLineStrip(points);
}

void Painter::drawRects(const std::vector<RectF>& rects)
{
    for(uint32_t i = 0; i < rects.size(); ++i)
        drawRect(rects[i]);
}

void Painter::drawRects(const std::vector<RectI>& rects)
{
    for(uint32_t i = 0; i < rects.size(); ++i)
        drawRect(rects[i]);
}

void Painter::drawFilledRect(const RectF &rect)
{
    auto* vertexData = m_frameBuffers[m_currentFBO]->add<SolidVertexBuffer>(4, PrimitiveTypeTriangleStrip, getCurrentState());

    vertexData[0].x = rect.left();
    vertexData[0].y = rect.top();

    vertexData[1].x = rect.right();
    vertexData[1].y = rect.top();

    vertexData[2].x = rect.left();
    vertexData[2].y = rect.bottom();

    vertexData[3].x = rect.right();
    vertexData[3].y = rect.bottom();
}

void Painter::drawFilledRects(const std::vector<RectF> &rects)
{
    for(uint32_t i = 0; i < rects.size(); ++i)
        drawFilledRect(rects[i]);
}

void Painter::drawFilledRects(const std::vector<RectI> &rects)
{
    static std::vector<RectF> rectsF(rects.size());
    for(uint32_t i = 0; i < rects.size(); ++i)
        rectsF[i] = rects[i].toRectF();
    drawFilledRects(rectsF);
}

void Painter::drawTexturedRect(const RectF &destRect, const TexturePtr &texture, const RectI &srcRect)
{
    if(!texture)
        return;

    static std::vector<RectF> destRectsF(1);
    static std::vector<RectI> srcRects(1);
    destRectsF[0] = destRect;
    srcRects[0] = srcRect;
    drawTexturedRects(destRectsF, texture, srcRects);
}

void Painter::drawTexturedRect(const RectI &destRect, const TexturePtr &texture)
{
    drawTexturedRect(destRect, texture, RectI(0, 0, texture->getSize()));
}

void Painter::drawTexturedRect(const RectF &destRect, const TexturePtr &texture)
{
    drawTexturedRect(destRect, texture, RectI(0, 0, texture->getSize()));
}

void Painter::drawTexturedRects(const std::vector<RectI> &destRects, const TexturePtr &texture, const std::vector<RectI> &srcRects)
{
    static std::vector<RectF> destRectsF;
    destRectsF.resize(destRects.size());
    for(int i = 0; i < destRectsF.size(); ++i)
        destRectsF[i] = destRects[i].toRectF();
    drawTexturedRects(destRectsF, texture, srcRects);
}

void Painter::drawTexturedRects(const std::vector<RectF> &destRects, const TexturePtr &texture, const std::vector<RectI>& srcRects)
{
    if(!texture)
        return;

    size_t size = destRects.size();
    auto* vertexData = m_frameBuffers[m_currentFBO]->add<TexelVertexBuffer>(size * 6, PrimitiveTypeTriangleStrip, getCurrentState(), texture);

    const Matrix3& uvmat = texture->getTransformMatrix();

    for(size_t i = 0; i < size; ++i) {
        const RectF& destRect = destRects[i];
        const RectI& srcRect = srcRects[i];

        float dleft = destRect.x();
        float dtop = destRect.y();
        float dright = destRect.x() + destRect.width();
        float dbottom = destRect.y() + destRect.height();

        float sleft = srcRect.x() * uvmat(1,1) + uvmat(3,1);
        float stop = srcRect.y() * uvmat(2,2) + uvmat(3,2);
        float sright = (srcRect.x() + srcRect.width()) * uvmat(1,1) + uvmat(3,1);
        float sbottom = (srcRect.y() + srcRect.height()) * uvmat(2,2) + uvmat(3,2);

        vertexData[i*6+0].x = dleft;
        vertexData[i*6+0].y = dtop;
        vertexData[i*6+0].u = sleft;
        vertexData[i*6+0].v = stop;

        vertexData[i*6+1].x = dright;
        vertexData[i*6+1].y = dtop;
        vertexData[i*6+1].u = sright;
        vertexData[i*6+1].v = stop;

        vertexData[i*6+2].x = dright;
        vertexData[i*6+2].y = dbottom;
        vertexData[i*6+2].u = sright;
        vertexData[i*6+2].v = sbottom;

        vertexData[i*6+3].x = dleft;
        vertexData[i*6+3].y = dtop;
        vertexData[i*6+3].u = sleft;
        vertexData[i*6+3].v = stop;

        vertexData[i*6+4].x = dright;
        vertexData[i*6+4].y = dbottom;
        vertexData[i*6+4].u = sright;
        vertexData[i*6+4].v = sbottom;

        vertexData[i*6+5].x = dleft;
        vertexData[i*6+5].y = dbottom;
        vertexData[i*6+5].u = sleft;
        vertexData[i*6+5].v = sbottom;
    }
}

GPUCommand::~GPUCommand()
{
    cancel();
}

bool GPUCommand::acquire()
{
    if(m_commandBuffer)
        return true;

    m_commandBuffer = SDL_AcquireGPUCommandBuffer(g_painter->getDevice());
    return m_commandBuffer != nullptr;
}

SDL_GPUTexture* GPUCommand::acquireSwapchain()
{
    SDL_GPUTexture* texture = nullptr;
    if(!SDL_WaitAndAcquireGPUSwapchainTexture(m_commandBuffer, g_window->getSDLWindow(), &texture, &m_width, &m_height)) {
        SDL_Log("Acquire swapchainTexture: %s", SDL_GetError());
        return nullptr;
    }
    return texture;
}

void GPUCommand::cancel()
{
    if(m_commandBuffer) {
        SDL_CancelGPUCommandBuffer(m_commandBuffer);
        m_commandBuffer = nullptr;
    }
}

void GPUCommand::submit(bool wait)
{
    if(!m_commandBuffer)
        return;
    
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(m_commandBuffer);
    if(fence && wait)
        SDL_WaitForGPUFences(g_painter->getDevice(), true, &fence, 1);
    m_commandBuffer = nullptr;
    m_width = 0;
    m_height = 0;
}

bool Painter::beginRender()
{
    if(m_gpuCommand.acquire()) {
        reset();
        m_drawnPrimitives = 0;
        m_drawCalls = 0;
        m_frameBuffers[0]->reset();
        return true;
    }
    return false;
}

void Painter::endRender()
{
    m_lastDrawnPrimitives = m_drawnPrimitives;
    m_lastDrawCalls = m_drawCalls;
    m_stateId = 0;
}

void Painter::flushRender()
{
    swapBuffers();

    /*if(m_async)
        m_taskQueue.flush();

    bool processed = m_frameCounter.processFrame();
    m_frameCounter.update();

    if(!m_vsync && processed)
        g_clock.setFrameNanos(1000000000.0/std::max<double>(m_frameCounter.getPartialFps(), g_platform.getRefreshRate()));

    g_clock.nextFrame();*/
}

void Painter::swapBuffers()
{
    draw();
    m_frameIndex = (m_frameIndex + 1) % FramesInFlight;
    m_gpuCommand.submit(false);
}

void Painter::pushState(bool doReset)
{
    m_olderStates[m_oldStateIndex].copy(m_state);
    m_oldStateIndex++;

    if(doReset)
        reset();
}

void Painter::popState(bool doReset)
{
    m_oldStateIndex--;
    if(doReset)
        reset();
    else
        m_state.copy(m_olderStates[m_oldStateIndex]);
}

void Painter::clear(const Color& color)
{
    BufferManagerPtr bufferManager = m_frameBuffers[m_currentFBO];
    if(!bufferManager)
        return;
    bufferManager->clear(color);
}

void Painter::draw()
{
    SDL_GPUCommandBuffer* commandBuffer = m_gpuCommand.getCommand();
    if(!commandBuffer)
        return;

    BufferManagerPtr bufferManager = m_frameBuffers[m_currentFBO];
    if(!bufferManager)
        return;

    bufferManager->uploadPendingTextures(commandBuffer);

    SDL_GPUTexture* texture = bufferManager->getTexture();
    uint32_t width = 0, height = 0;
    if(!texture) {
        if(m_currentFBO == 0) {
            texture = m_gpuCommand.acquireSwapchain();
            width = m_gpuCommand.width();
            height = m_gpuCommand.height();
        }

        if(!texture)
            return;
    } else {
        width = bufferManager->getWidth();
        height = bufferManager->getHeight();
    }

    static std::vector<SDL_GPUColorTargetInfo> colorTargets(1);

    const Color& clearColor = bufferManager->getClearColor();
    
    SDL_zero(colorTargets[0]);
    colorTargets[0].texture = texture;
    colorTargets[0].load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargets[0].store_op = SDL_GPU_STOREOP_STORE;
    colorTargets[0].clear_color = SDL_FColor{ clearColor.rF(), clearColor.gF(), clearColor.bF(), clearColor.aF() };

    SDL_GPUBuffer* buffer = bufferManager->getBuffer(m_frameIndex);
    if(!buffer)
        return;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), (uint32_t)colorTargets.size(), NULL);

    bufferManager->upload(m_frameIndex);

    static SDL_GPUBufferBinding binding;
    binding.buffer = buffer;
    binding.offset = 0;

    SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);

    Program* drawProgram = nullptr;
    int updateFlags = 0;
    int32_t lastState = -1;
    RectI frameBufferRect(0, 0, width >> colorTargets[0].mip_level, height >> colorTargets[0].mip_level);
    SDL_GPUViewport viewport;
    SDL_Rect rect;
    for(DrawCommand& drawCommand : *bufferManager.get()) {
        PainterState& drawState = m_states[drawCommand.state];
        if(lastState != drawState.id) {
            if(lastState == -1) {
                updateFlags = MustUpdateProgramResource;
                drawProgram = drawState.program;
                if(!drawState.clipRect.isEmpty())
                    updateFlags |= MustUpdateClipRect;
                if(!drawState.viewport.isEmpty())
                    updateFlags |= MustUpdateViewport;
            } else
                updateFlags = drawState.flags;
            lastState = (int32_t)drawState.id;
        }

        if(!drawState.program) {
            Program* program = g_programs.get(drawState.blendMode, drawCommand.type, drawCommand.texture != nullptr);
            if(drawProgram != program) {
                drawProgram = program;
                updateFlags = MustUpdateProgramResource;
            }
        }

        if(updateFlags & MustUpdateProgram)
            drawProgram->bind(renderPass);

        if(updateFlags & MustUpdateColor)
            drawProgram->setColor(drawState.color);

        if(updateFlags & MustUpdateLineWidth) {
            // apply geometry shader?
        }

        if(updateFlags & MustUpdatePointSize)
            drawProgram->setSize(drawState.pointSize);

        if(updateFlags & MustUpdateClipRect) {
            if(drawState.viewport.size() == drawState.resolution) {
                rect.h = drawState.clipRect.left();
                rect.y = drawState.resolution.h - drawState.clipRect.bottom() - 1;
                rect.w = drawState.clipRect.width();
                rect.h = drawState.clipRect.height();
            } else {
                rect.x = (int)((drawState.clipRect.left()                              /(float)drawState.resolution.w) * drawState.viewport.width());
                rect.y = (int)(((drawState.resolution.h - drawState.clipRect.bottom() - 1)/(float)drawState.resolution.h) * drawState.viewport.height());
                rect.w = (int)((drawState.clipRect.width()                             /(float)drawState.resolution.w) * drawState.viewport.width());
                rect.h = (int)((drawState.clipRect.height()                            /(float)drawState.resolution.h) * drawState.viewport.height());
            }

            SDL_SetGPUScissor(renderPass, &rect);
        }

        if(updateFlags & MustUpdateResolution)
            drawProgram->setResolution(drawState.resolution);

        if(updateFlags & MustUpdateViewport) {
            viewport.x = (float)drawState.viewport.x();
            viewport.y = (float)drawState.viewport.y();
            viewport.w = (float)drawState.viewport.width();
            viewport.h = (float)drawState.viewport.height();
            viewport.min_depth = 0;
            viewport.max_depth = 1;

            SDL_SetGPUViewport(renderPass, &viewport);
        }

        if(updateFlags & MustUpdateProjectionTransformMatrix) {
            Matrix3 projectionTransformMatrix = drawState.projectionMatrix * drawState.transformMatrix;
            drawProgram->setProjectionTransformMatrix(projectionTransformMatrix);
        }

        drawProgram->pushData(commandBuffer);

        drawCommand.bindTexture(renderPass);

        SDL_DrawGPUPrimitives(renderPass, (uint32_t)drawCommand.vertexCount, 1, (uint32_t)drawCommand.offset, 0);

        if(updateFlags & MustUpdateViewport) {
            viewport.x = 0;
            viewport.y = 0;
            viewport.w = (float)frameBufferRect.width();
            viewport.h = (float)frameBufferRect.height();
            viewport.min_depth = 0;
            viewport.max_depth = 1;

            SDL_SetGPUViewport(renderPass, &viewport);
        }

        if(updateFlags & MustUpdateClipRect) {
            rect.x = frameBufferRect.x();
            rect.y = frameBufferRect.y();
            rect.w = frameBufferRect.width();
            rect.h = frameBufferRect.height();
            SDL_SetGPUScissor(renderPass, &rect);
        }

        updateFlags = 0;
    }
    SDL_EndGPURenderPass(renderPass);
}
