#include "texture.h"

#include <utils/include.h>
#include <graphics/painter.h>
#include <graphics/image.h>

static SDL_GPUTextureCreateInfo textureInfo;

Texture::~Texture()
{
    if(m_texture)
        SDL_ReleaseGPUTexture(g_painter->getDevice(), m_texture);
    if(m_sampler)
        SDL_ReleaseGPUSampler(g_painter->getDevice(), m_sampler);
}

void Texture::uploadPixels(const ImagePtr &imagePtr)
{
    SDL_GPUTextureCreateInfo textureInfo;
    SDL_zero(textureInfo);
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.width = imagePtr->getWidth();
    textureInfo.height = imagePtr->getHeight();
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    m_texture = SDL_CreateGPUTexture(g_painter->getDevice(), &textureInfo);
    m_size = m_gpuSize = imagePtr->getSize();
    setupTranformMatrix();
    updateSampler();

    SDL_GPUTransferBufferCreateInfo tbInfo;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.props = 0;
    tbInfo.size = imagePtr->getPixelDataSize();

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(g_painter->getDevice(), &tbInfo);
    uint8_t* textureData = (uint8_t*)SDL_MapGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer, false);
    memcpy(textureData, imagePtr->getPixelData(), imagePtr->getPixelDataSize());
    SDL_UnmapGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer);

    GPUCommand gpuCommand;
    if(!gpuCommand.acquire(g_painter->getDevice())) {
        SDL_GPUCommandBuffer* uploadCmdBuf = gpuCommand.command();
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

        SDL_GPUTextureTransferInfo tti;
        tti.offset = 0;
        tti.pixels_per_row = 0;
        tti.rows_per_layer = 0;
        tti.transfer_buffer = textureTransferBuffer;

        SDL_GPUTextureRegion dest;
        SDL_zero(dest);
        dest.texture = m_texture;
        dest.w = imagePtr->getWidth();
        dest.h = imagePtr->getHeight();
        dest.d = 1;

        SDL_UploadToGPUTexture(copyPass, &tti, &dest, false);
        SDL_EndGPUCopyPass(copyPass);
        SDL_ReleaseGPUTransferBuffer(g_painter->getDevice(), textureTransferBuffer);
        gpuCommand.submit(g_painter->getDevice());
    }
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
