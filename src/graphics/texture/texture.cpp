#include "texture.h"

#include <utils/include.h>
#include <graphics/painter.h>
#include <graphics/image.h>

Texture::Texture()
{
    g_painter->genTextures(1, &m_id);
}

Texture::~Texture()
{
    g_painter->deleteTextures(1, &m_id);
}

void Texture::uploadPixels(const ImagePtr& imagePtr)
{
    m_size = m_gpuSize = imagePtr->getSize();
    setupTranformMatrix();
    updateSampler();

    g_painter->uploadTextureData(shared_from_this(), imagePtr->getPixelData(), imagePtr->getPixelDataSize());
}

void Texture::updateSampler()
{
    SDL_GPUSamplerCreateInfo samplerInfo;
    SDL_zero(samplerInfo);
    samplerInfo.min_filter = m_mipmapFilter ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
    samplerInfo.mag_filter = m_smooth ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
    samplerInfo.mipmap_mode = m_mipmapFilter ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = m_repeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = m_repeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = m_repeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    g_painter->updateTextureSampler(1, &m_id, &samplerInfo);
}

void Texture::setupTranformMatrix()
{
    m_transformMatrix = {
        1.0f / m_gpuSize.w,  0.0f,                  0.0f,
        0.0f,                1.0f / m_gpuSize.h,    0.0f,
        0.0f,                0.0f,                  1.0f
    };
}
