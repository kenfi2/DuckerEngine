#ifndef PAINTERSTATE_H
#define PAINTERSTATE_H

#include <utils/color.h>
#include <utils/point.h>
#include <utils/rect.h>
#include <utils/size.h>
#include <utils/matrix.h>
#include <utils/include.h>

enum {
    MustUpdateColor = 1 << 0,
    MustUpdateLineWidth = 1 << 1,
    MustUpdatePointSize = 1 << 2,
    MustUpdateBlendMode = 1 << 3,
    MustUpdateClipRect = 1 << 4,
    MustUpdateResolution = 1 << 5,
    MustUpdateViewport = 1 << 6,
    MustUpdateProjectionTransformMatrix = 1 << 7,
    MustUpdateProgram = 1 << 8,
    MustUpdateProgramResource = MustUpdateProgram | MustUpdateColor | MustUpdateLineWidth | MustUpdatePointSize | MustUpdateBlendMode | MustUpdateResolution | MustUpdateProjectionTransformMatrix
};

enum {
    MaxMultiTextures = 16
};

class Program;

struct PainterState {
    PainterState() = default;

    void copy(const PainterState& state);

    std::array<uint32_t, MaxMultiTextures> textures;

    Program* program = nullptr;
    SizeI resolution;
    RectI viewport;
    Matrix3 transformMatrix;
    Matrix3 projectionMatrix;
    Color color = 0xffffffff;
    float opacity = 1.0f;
    float lineWidth = 1.0f;
    float pointSize = 1.0f;
    BlendMode blendMode = BlendMode_Blend;
    RectI clipRect;
    size_t id = 0;
    int flags = 0;
    uint32_t bindedTextures = 0;
};

inline void PainterState::copy(const PainterState& state)
{
    if(state.bindedTextures != 0)
        textures = state.textures;
    program = state.program;
    resolution = state.resolution;
    viewport = state.viewport;
    transformMatrix = state.transformMatrix;
    projectionMatrix = state.projectionMatrix;
    color = state.color;
    opacity = state.opacity;
    lineWidth = state.lineWidth;
    pointSize = state.pointSize;
    blendMode = state.blendMode;
    clipRect = state.clipRect;
    id = state.id;
    flags = state.flags;
    bindedTextures = state.bindedTextures;
}

#endif
