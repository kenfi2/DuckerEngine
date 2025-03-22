#ifndef IMAGE_H
#define IMAGE_H

#include <utils/include.h>
#include <utils/size.h>

class Image {
public:
    Image(const SizeI& size);
    Image(int width, int height);
    Image(const uint32_t *data, int width, int height);
    Image(std::vector<uint32_t> data, const SizeI& size);

    uint32_t getWidth() const { return m_size.w; }
    uint32_t getHeight() const { return m_size.h; }
    SizeI getSize() const { return m_size; }

    void setPixel(uint32_t x, uint32_t y, uint32_t pixel) { m_pixels[m_size.w*y + x] = pixel; }
    void setPixelRGBA(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) { setPixel(x, y, (r << 0) | (g << 8) | (b << 16) | (a << 24)); }
    int getPitch() { return m_size.w * 4; }
    int getPixelDataSize() { return m_size.area() * 4; }
    uint8_t* getPixelData(uint32_t x = 0, uint32_t y = 0) { return (uint8_t*)&m_pixels[m_size.w*y + x]; }

private:
    SizeI m_size;
    std::vector<uint32_t> m_pixels;
};

#endif
