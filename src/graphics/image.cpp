#include "image.h"

Image::Image(const SizeI& size) :
    m_size(size)
{
    m_pixels.resize(size.area(), 0);
}

Image::Image(int width, int height) :
    m_size(SizeI(width, height))
{
    m_pixels.resize(m_size.area(), 0);
}

Image::Image(const uint32_t* data, int width, int height) :
    m_size(SizeI(width, height))
{
    m_pixels.resize(m_size.area());
    memcpy(m_pixels.data(), data, m_size.area() * 4);
}

Image::Image(std::vector<uint32_t> data, const SizeI& size) :
    m_pixels(std::move(data)), m_size(size)
{
}
