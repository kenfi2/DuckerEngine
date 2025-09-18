#include "texture.h"

#include <utils/include.h>
#include <graphics/painter.h>
#include <graphics/image.h>

Texture::~Texture()
{
    if(m_texture)
        SDL_ReleaseGPUTexture(g_painter->getDevice(), m_texture);
    if(m_sampler)
        SDL_ReleaseGPUSampler(g_painter->getDevice(), m_sampler);
}

void Texture::generate()
{
    SDL_GPUTextureCreateInfo textureInfo;
    SDL_zero(textureInfo);
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.width = m_size.w;
    textureInfo.height = m_size.h;
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    m_texture = SDL_CreateGPUTexture(g_painter->getDevice(), &textureInfo);
    m_gpuSize = m_size;
    setupTranformMatrix();
    updateSampler();
}

void Texture::uploadPixels(const ImagePtr &imagePtr)
{
    m_image = imagePtr;
    m_size = m_gpuSize = m_image->getSize();
    setupTranformMatrix();
    g_painter->addPendingTexture(shared_from_this());
}

void Texture::upload(SDL_GPUCommandBuffer* commandBuffer)
{
    SDL_GPUTextureCreateInfo textureInfo;
    SDL_zero(textureInfo);
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.width = m_image->getWidth();
    textureInfo.height = m_image->getHeight();
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    m_texture = SDL_CreateGPUTexture(g_painter->getDevice(), &textureInfo);
    updateSampler();

    SDL_GPUTransferBufferCreateInfo tbInfo;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.props = 0;
    tbInfo.size = m_image->getPixelDataSize();

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo);
    uint8_t* textureData = (uint8_t*)SDL_MapGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer, false);
    memcpy(textureData, m_image->getPixelData(), m_image->getPixelDataSize());
    SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer);

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo tti;
    tti.offset = 0;
    tti.pixels_per_row = 0;
    tti.rows_per_layer = 0;
    tti.transfer_buffer = textureTransferBuffer;

    SDL_GPUTextureRegion dest;
    SDL_zero(dest);
    dest.texture = m_texture;
    dest.w = m_image->getWidth();
    dest.h = m_image->getHeight();
    dest.d = 1;

    SDL_UploadToGPUTexture(copyPass, &tti, &dest, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer);
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
    
    if(m_sampler)
        SDL_ReleaseGPUSampler(g_painter->getDevice(), m_sampler);
    m_sampler = SDL_CreateGPUSampler(g_painter->getDevice(), &samplerInfo);
}

void Texture::setupTranformMatrix()
{
    m_transformMatrix = {
        1.0f / m_gpuSize.w,  0.0f,                  0.0f,
        0.0f,                1.0f / m_gpuSize.h,    0.0f,
        0.0f,                0.0f,                  1.0f
    };
}

void Texture::bind(SDL_GPURenderPass* renderPass)
{
    static SDL_GPUTextureSamplerBinding binding;
    binding.texture = m_texture;
    binding.sampler = m_sampler;

    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);
}
