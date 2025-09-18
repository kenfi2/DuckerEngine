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
        // m_gpuDriver = "direct3d12";
        m_gpuDriver = "vulkan";
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
            m_gpuDevice = SDL_CreateGPUDevice(g_shaderFormats, false, gpuDriver.c_str());
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
        m_frameBuffers[fboId]->setTexture(texture->get());
}

void Painter::preDraw(SDL_GPURenderPass *renderPass)
{
    if(m_mustUpdateResolution) {
        updateResolution(renderPass);
        m_mustUpdateResolution = false;
    }

    if(m_mustUpdateViewport) {
        updateViewport(renderPass);
        m_mustUpdateViewport = false;
    }
}

void Painter::translate(float x, float y)
{
    Matrix3 translateMatrix = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
           x,    y, 1.0f
    };

    setTransformMatrix(m_transformMatrix * translateMatrix);
}

void Painter::setResolution(const SizeI &resolution)
{
    if(m_resolution == resolution)
        return;
    m_resolution = resolution;
    resetProjectionMatrix();
}

void Painter::setViewport(const RectI &viewport)
{
    if(m_viewport == viewport)
        return;
    m_viewport = viewport;
    m_mustUpdateViewport = true;
}

void Painter::reset()
{
    resetColor();
    resetProjectionMatrix();
    resetTransformMatrix();
}

void Painter::refresh()
{
    updateProgram();
}

void Painter::resetProjectionMatrix()
{
    float dx = 2.0f / m_resolution.w;
    float dy = 2.0f / m_resolution.h;

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

void Painter::updateProgram()
{
}

void Painter::updateProjectionTransformMatrix()
{
    static Matrix3 projectionTransformMatrix;
    projectionTransformMatrix = m_transformMatrix * m_projectionMatrix;

    m_projectionTransformMatrix[0] = projectionTransformMatrix(1,1);
    m_projectionTransformMatrix[1] = projectionTransformMatrix(1,2);
    m_projectionTransformMatrix[2] = 0.0f;
    m_projectionTransformMatrix[3] = projectionTransformMatrix(1,3);

    m_projectionTransformMatrix[4] = projectionTransformMatrix(2,1);
    m_projectionTransformMatrix[5] = projectionTransformMatrix(2,2);
    m_projectionTransformMatrix[6] = 0.0f;
    m_projectionTransformMatrix[7] = projectionTransformMatrix(2,3);

    m_projectionTransformMatrix[8] = 0.0f;
    m_projectionTransformMatrix[9] = 0.0f;
    m_projectionTransformMatrix[10] = 1.0f;
    m_projectionTransformMatrix[11] = 0.0f;

    m_projectionTransformMatrix[12] = projectionTransformMatrix(3,1);
    m_projectionTransformMatrix[13] = projectionTransformMatrix(3,2);
    m_projectionTransformMatrix[14] = 0.0f;
    m_projectionTransformMatrix[15] = projectionTransformMatrix(3,3);
}

void Painter::updateResolution(SDL_GPURenderPass*)
{
}

void Painter::updateViewport(SDL_GPURenderPass* renderPass)
{
    SDL_GPUViewport viewport;
    SDL_zero(viewport);
    viewport.x = m_viewport.x();
    viewport.y = m_viewport.y();
    viewport.w = m_viewport.width();
    viewport.h = m_viewport.height();
    SDL_SetGPUViewport(renderPass, &viewport);
}

void Painter::setProjectionMatrix(const Matrix3 &projectionMatrix)
{
    m_projectionMatrix = projectionMatrix;
    updateProjectionTransformMatrix();
}

void Painter::setTransformMatrix(const Matrix3& transformMatrix)
{
    m_transformMatrix = transformMatrix;
    updateProjectionTransformMatrix();
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
    auto* vertexData = m_frameBuffers[m_currentFBO]->add(points.size(), PrimitiveTypePointList);

    for(uint32_t i = 0; i < points.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = points[i];
        d.x = p.x;
        d.y = p.y;
        d.u = 0.f;
        d.v = 0.f;
        d.r = m_color.rF();
        d.g = m_color.gF();
        d.b = m_color.bF();
        d.a = m_color.aF();
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
    auto* vertexData = m_frameBuffers[m_currentFBO]->add(lines.size(), PrimitiveTypeLineList);
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
        d.u = 0.f;
        d.v = 0.f;
        d.r = m_color.rF();
        d.g = m_color.gF();
        d.b = m_color.bF();
        d.a = m_color.aF();
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
    auto* vertexData = m_frameBuffers[m_currentFBO]->add(lines.size(), PrimitiveTypeLineStrip);
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
        d.u = 0.f;
        d.v = 0.f;
        d.r = m_color.rF();
        d.g = m_color.gF();
        d.b = m_color.bF();
        d.a = m_color.aF();
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

    auto* vertexData = m_frameBuffers[m_currentFBO]->add(points.size(), primitiveType);
    for(uint32_t i = 0; i < points.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& point = points[i];
        d.x = point.x;
        d.y = point.y;
        d.u = 0.0f;
        d.v = 0.0f;
        d.r = m_color.rF();
        d.g = m_color.gF();
        d.b = m_color.bF();
        d.a = m_color.aF();
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
    auto* vertexData = m_frameBuffers[m_currentFBO]->add(4, PrimitiveTypeTriangleStrip);

    vertexData[0].x = rect.left();
    vertexData[0].y = rect.top();

    vertexData[1].x = rect.right();
    vertexData[1].y = rect.top();

    vertexData[2].x = rect.left();
    vertexData[2].y = rect.bottom();

    vertexData[3].x = rect.right();
    vertexData[3].y = rect.bottom();

    for(uint32_t i = 0; i < 4; ++i) {
        vertexData[i].r = m_color.rF();
        vertexData[i].g = m_color.gF();
        vertexData[i].b = m_color.bF();
        vertexData[i].a = m_color.aF();
    }
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
    auto* vertexData = m_frameBuffers[m_currentFBO]->add(size * 6, PrimitiveTypeTriangleStrip, texture);

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
    // m_olderStates[m_oldStateIndex].program = m_program;
    m_olderStates[m_oldStateIndex].resolution = m_resolution;
    m_olderStates[m_oldStateIndex].viewport = m_viewport;
    m_olderStates[m_oldStateIndex].transformMatrix = m_transformMatrix;
    m_olderStates[m_oldStateIndex].projectionMatrix = m_projectionMatrix;
    m_olderStates[m_oldStateIndex].color = m_color;
    m_olderStates[m_oldStateIndex].opacity = m_opacity;
    m_olderStates[m_oldStateIndex].lineWidth = m_lineWidth;
    m_olderStates[m_oldStateIndex].pointSize = m_pointSize;
    m_olderStates[m_oldStateIndex].blendMode = m_blendMode;
    m_olderStates[m_oldStateIndex].clipRect = m_clipRect;
    m_oldStateIndex++;

    if(doReset)
        reset();
}

void Painter::popState(bool doReset)
{
    m_oldStateIndex--;
    if(doReset)
        reset();
    else {
        setResolution(m_olderStates[m_oldStateIndex].resolution);
        setViewport(m_olderStates[m_oldStateIndex].viewport);
        setTransformMatrix(m_olderStates[m_oldStateIndex].transformMatrix);
        setProjectionMatrix(m_olderStates[m_oldStateIndex].projectionMatrix);
        setColor(m_olderStates[m_oldStateIndex].color);
    }
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
    if(!texture) {
        if(m_currentFBO == 0)
            texture = m_gpuCommand.acquireSwapchain();
        if(!texture)
            return;
    }

    static std::vector<SDL_GPUColorTargetInfo> colorTargets(1);
    
    SDL_zero(colorTargets[0]);
    colorTargets[0].texture = texture;
    colorTargets[0].load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargets[0].store_op = SDL_GPU_STOREOP_STORE;
    if(m_currentFBO == 0)
        colorTargets[0].clear_color = SDL_FColor{ 1.0f, 0.0f, 0.0f, 1.0f };
    else
        colorTargets[0].clear_color = SDL_FColor{ 0.0f, 0.0f, 0.0f, 0.0f };

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), (uint32_t)colorTargets.size(), NULL);
    preDraw(renderPass);
    SDL_GPUBuffer* buffer = bufferManager->getBuffer(m_frameIndex);
    Program* drawProgram = nullptr;
    if(buffer) {
        bufferManager->upload(m_frameIndex);

        static SDL_GPUBufferBinding binding;
        binding.buffer = buffer;
        binding.offset = 0;

        SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);

        for(DrawCommand& drawCommand : *bufferManager.get()) {
            Program* program = g_programs.get(drawCommand.type, drawCommand.texture != nullptr);
            if(program) {
                if(!drawProgram) {
                    struct CBO {
                        float ModelViewProj[16];
                    };
            
                    CBO cbo;
                    memcpy(cbo.ModelViewProj, m_projectionTransformMatrix, sizeof(m_projectionTransformMatrix));
            
                    program->setConstantBufferValue(0, VertexShader, cbo);
                    
                    program->pushData(commandBuffer);
                    drawProgram = program;
                }
                program->bind(renderPass);
            }

            drawCommand.bindTexture(renderPass);

            SDL_DrawGPUPrimitives(renderPass, (uint32_t)drawCommand.vertexCount, 1, (uint32_t)drawCommand.offset, 0);
        }
    }
    SDL_EndGPURenderPass(renderPass);
}
