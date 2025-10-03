#ifndef INCLUDE_H
#define INCLUDE_H

#define _CRT_SECURE_NO_WARNINGS

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

class Image;
using ImagePtr = std::shared_ptr<Image>;

class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class FrameBuffer;
using FrameBufferPtr = std::shared_ptr<FrameBuffer>;

class BufferManager;
using BufferManagerPtr = std::shared_ptr<BufferManager>;

class RenderBuffer;
using RenderBufferPtr = std::shared_ptr<RenderBuffer>;

enum TriangleDrawMode {
    DrawTriangles = 1000,
    DrawTriangleFan,
    DrawTriangleStrip
};

enum BlendMode {
    BlendMode_NoBlend,
    BlendMode_Blend,
    BlendMode_Multiply,
    BlendMode_Add,
    BlendMode_MultiplyMixed,
    BlendMode_Last
};

#endif
