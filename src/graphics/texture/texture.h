#ifndef TEXTURE_H
#define TEXTURE_H

#include <utils/include.h>
#include <utils/size.h>
#include <utils/matrix.h>

class GPUCommand;
class Texture : public std::enable_shared_from_this<Texture> {
public:
    Texture();
    ~Texture();

    const Matrix3& getTransformMatrix() const { return m_transformMatrix; }

    SizeI getSize() const { return m_size; }
    void setSize(const SizeI& size) { m_size = size; }

    void setSmooth(bool smooth) { m_smooth = smooth; }

    void uploadPixels(const ImagePtr& imagePtr);
    void updateSampler();
    void setupTranformMatrix();

    uint32_t getId() const { return m_id; }

private:
    Matrix3 m_transformMatrix;
    SizeI m_gpuSize;
    SizeI m_size;

    uint32_t m_id = 0;

    bool m_repeat = false;
    bool m_mipmapFilter = false;
    bool m_hasPixels = false;
    bool m_hasMipMaps = false;
    bool m_smooth = false;
    bool m_opaque = false;
};

#endif
