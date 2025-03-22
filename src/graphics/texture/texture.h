#ifndef TEXTURE_H
#define TEXTURE_H

#include <utils/include.h>
#include <utils/size.h>
#include <utils/matrix.h>

class Texture {
public:
    Texture() { }
    ~Texture();

    const Matrix3& getTransformMatrix() const { return m_transformMatrix;}
    SizeI getSize() const { return m_size; }

    void uploadPixels(const ImagePtr& imagePtr);
    void updateSampler();
    void setupTranformMatrix();

    void bind(SDL_GPURenderPass* renderPass);

private:
    Matrix3 m_transformMatrix;
    SizeI m_gpuSize;
    SizeI m_size;

    SDL_GPUTexture* m_texture = nullptr;
    SDL_GPUSampler* m_sampler = nullptr;

    bool m_repeat = false;
    bool m_mipmapFilter = false;
    bool m_hasPixels = false;
    bool m_hasMipMaps = false;
    bool m_smooth = false;
    bool m_opaque = false;
};

#endif
