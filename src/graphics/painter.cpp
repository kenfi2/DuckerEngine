#include <window.h>
#include "shaders.h"

#include "engine.h"
#include "program.h"
#include "painter.h"

#include <graphics/texture/texture.h>
#include <ui/ui.h>

static SDL_GPUShaderFormat g_shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB;

bool Painter::init()
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

    reset();

    return m_bufferManager.init(m_gpuDriver);
}

void Painter::destroy()
{
    m_bufferManager.clear();
    SDL_ReleaseWindowFromGPUDevice(m_gpuDevice, g_window->getSDLWindow());
    SDL_DestroyGPUDevice(m_gpuDevice);
    m_gpuDevice = nullptr;
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

void Painter::pushState(bool doReset)
{
    m_painterStates[m_painterStateIndex].m_projectionMatrix = m_projectionMatrix;
    m_painterStates[m_painterStateIndex].m_transformMatrix = m_transformMatrix;
    m_painterStateIndex++;

    if(doReset)
        reset();
}

void Painter::popState(bool doReset)
{
    m_painterStateIndex--;
    if(doReset)
        reset();
    else {
        setProjectionMatrix(m_painterStates[m_painterStateIndex].m_projectionMatrix);
        setTransformMatrix(m_painterStates[m_painterStateIndex].m_transformMatrix);
    }
}

void Painter::reset()
{
    resetProjectionMatrix();
    resetTransformMatrix();
    resetColor();
}

void Painter::resetProjectionMatrix()
{
    float dx = 2.0f / g_window->getWidth();
    float dy = 2.0f / g_window->getHeight();

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
    auto* vertexData = m_bufferManager.acquireBuffer<SolidVertex>((uint32_t)points.size(), PrimitiveTypePointList);
    for(uint32_t i = 0; i < points.size(); ++i) {
        SolidVertex& d = vertexData[i];
        const PointF& p = points[i];
        d.x = p.x;
        d.y = p.y;
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
    auto* vertexData = m_bufferManager.acquireBuffer<SolidVertex>((uint32_t)lines.size(), PrimitiveTypeLineList);
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
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
    auto* vertexData = m_bufferManager.acquireBuffer<SolidVertex>((uint32_t)lines.size(), PrimitiveTypeLineStrip);
    for(uint32_t i = 0; i < lines.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& p = lines[i];
        d.x = p.x;
        d.y = p.y;
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
    // assert(points.size() >= 3);

    if(mode == DrawTriangleStrip) {
        for(uint32_t i = 2; i < points.size(); ++i)
            drawTriangle(points[i-2], points[i-1], points[i]);
    } else if(mode == DrawTriangles) {
        // assert(points.size() % 3 == 0);
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

    PrimitiveType primitiveType;
    if(mode == DrawTriangleStrip)
        primitiveType = PrimitiveTypeTriangleStrip;
    else if(mode == DrawTriangles)
        primitiveType = PrimitiveTypeTriangleList;

    auto* vertexData = m_bufferManager.acquireBuffer<SolidVertex>((uint32_t)points.size(), primitiveType);
    for(uint32_t i = 0; i < points.size(); ++i) {
        auto& d = vertexData[i];
        const PointF& point = points[i];
        d.x = point.x;
        d.y = point.y;
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
    auto* vertexData = m_bufferManager.acquireBuffer<SolidVertex>(4, PrimitiveTypeTriangleStrip);

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
    std::vector<RectF> rectsF(rects.size());
    for(uint32_t i = 0; i < rects.size(); ++i)
        rectsF[i] = rects[i].toRectF();
    drawFilledRects(rectsF);
}

void Painter::drawTexturedRect(const RectF &destRect, const TexturePtr &texture, const RectI &srcRect)
{
    if(!texture)
        return;

    auto* vertexData = m_bufferManager.acquireBuffer<TextureVertex>(6, PrimitiveTypeTriangleList, texture);

    const Matrix3& uvmat = texture->getTransformMatrix();

    float dleft = destRect.x();
    float dtop = destRect.y();
    float dright = destRect.x() + destRect.width();
    float dbottom = destRect.y() + destRect.height();

    float sleft = srcRect.x() * uvmat(1,1) + uvmat(3,1);
    float stop = srcRect.y() * uvmat(2,2) + uvmat(3,2);
    float sright = (srcRect.x() + srcRect.width()) * uvmat(1,1) + uvmat(3,1);
    float sbottom = (srcRect.y() + srcRect.height()) * uvmat(2,2) + uvmat(3,2);

    vertexData[0].x = dleft;
    vertexData[0].y = dtop;
    vertexData[0].u = sleft;
    vertexData[0].v = stop;

    vertexData[1].x = dright;
    vertexData[1].y = dtop;
    vertexData[1].u = sright;
    vertexData[1].v = stop;

    vertexData[2].x = dright;
    vertexData[2].y = dbottom;
    vertexData[2].u = sright;
    vertexData[2].v = sbottom;

    vertexData[3].x = dleft;
    vertexData[3].y = dtop;
    vertexData[3].u = sleft;
    vertexData[3].v = stop;

    vertexData[4].x = dright;
    vertexData[4].y = dbottom;
    vertexData[4].u = sright;
    vertexData[4].v = sbottom;

    vertexData[5].x = dleft;
    vertexData[5].y = dbottom;
    vertexData[5].u = sleft;
    vertexData[5].v = sbottom;
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

    int size = (int)destRects.size();
    auto* vertexData = m_bufferManager.acquireBuffer<TextureVertex>(size * 6, PrimitiveTypeTriangleList, texture);

    const Matrix3& uvmat = texture->getTransformMatrix();

    for(int i = 0; i < size; ++i) {
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

bool GPUCommand::acquire(bool swapchain)
{
    if(m_commandBuffer) {
        SDL_Log("Double draw command acquire.");
        return false;
    }

    m_commandBuffer = SDL_AcquireGPUCommandBuffer(g_painter->getDevice());
    if(swapchain) {
        if(!SDL_AcquireGPUSwapchainTexture(m_commandBuffer, g_window->getSDLWindow(), &m_swapchainTexture, &m_width, &m_height)) {
            SDL_Log("Acquire swapchainTexture: %s", SDL_GetError());
            cancel();
            return false;
        }

        if(!m_swapchainTexture) {
            cancel();
            return false;
        }
    }
    return true;
}

void GPUCommand::cancel()
{
    if(m_commandBuffer) {
        SDL_CancelGPUCommandBuffer(m_commandBuffer);
        m_commandBuffer = nullptr;
    }
}

void GPUCommand::submit(bool waitFences)
{
    if(!m_commandBuffer)
        return;

    if(waitFences) {
        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(m_commandBuffer);
        if(fence)
            SDL_WaitForGPUFences(g_painter->getDevice(), true, &fence, 1);
    }
    else
        SDL_SubmitGPUCommandBuffer(m_commandBuffer);
    m_commandBuffer = nullptr;
    m_swapchainTexture = nullptr;
    m_width = 0;
    m_height = 0;
}

void Painter::beginFrame()
{
    if(!m_gpuCommand.acquire(m_currentFrame == DrawFrame)) {
        m_lostFrames++;
        return;
    }

    m_lostFrames = 0;
    if(m_currentFrame == DrawFrame)
        clear();
    else
        m_bufferManager.map();
}

void Painter::endFrame()
{
    if(m_currentFrame == DrawFrame) {
        draw();
        m_bufferManager.reset();
    } else
        m_bufferManager.upload(m_gpuCommand.command());

    m_gpuCommand.submit();
    m_currentFrame = (m_currentFrame + 1) % LastFrame;
}

void Painter::clear()
{
    SDL_GPUCommandBuffer* commandBuffer = m_gpuCommand.command();
    if(!commandBuffer)
        return;

    SDL_GPUTexture* swapchainTexture = m_gpuCommand.swapchain();
    if(!swapchainTexture)
        return;

    SDL_GPUColorTargetInfo color_target_info;
    SDL_zero(color_target_info);
    color_target_info.texture = swapchainTexture;
    color_target_info.clear_color.a = 1.0f;
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;

    SDL_EndGPURenderPass(SDL_BeginGPURenderPass(commandBuffer, &color_target_info, 1, NULL));
}

void Painter::draw()
{
    SDL_GPUCommandBuffer* commandBuffer = m_gpuCommand.command();
    if(!commandBuffer)
        return;

    SDL_GPUTexture* swapchainTexture = m_gpuCommand.swapchain();
    if(!swapchainTexture)
        return;

    SDL_GPUColorTargetInfo colorInfo;
    SDL_zero(colorInfo);
    colorInfo.clear_color.a = 1.0f;
    colorInfo.load_op = SDL_GPU_LOADOP_LOAD;
    colorInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorInfo.texture = swapchainTexture;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorInfo, 1, NULL);
    for(const auto& it : m_bufferManager.getBuffers()) {
        Program* program = it.getProgram();
        if(program) {
            program->pushData(commandBuffer, 0, m_projectionTransformMatrix, sizeof(m_projectionTransformMatrix));
            program->bind(renderPass);
        }

        TexturePtr texture = it.getTexture();
        if(texture)
            texture->bind(renderPass);

        DrawBuffer* drawBuffer = it.getDrawBuffer();
        if(drawBuffer)
            drawBuffer->draw(renderPass, it);
    }
    SDL_EndGPURenderPass(renderPass);
}
