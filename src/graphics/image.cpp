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

Pixmap Image::convertToPixmap(uint32_t format, RectI clip, SizeI outSize, bool doFlip, int outPitch)
{
    // format => can vary depending on the painter/platform
    // outSize => is generally in power of two in iOS
    // doFlip => generally always true in OpenGL
    // outPitch => generally set in DX9


    if(!clip.isValid())
        clip = RectI(0, 0, m_size);

    if(!outSize.isValid())
        outSize = clip.size();

    Pixmap pb;
    pb.width = outSize.w;
    pb.height = outSize.h;
    pb.format = format;
    pb.bytes_per_pixel = SDL_BYTESPERPIXEL(format);
    pb.pitch = (outPitch != 0) ? outPitch : pb.bytes_per_pixel * pb.width;

    int pitch = outSize.w * 4;
    std::vector<uint32_t> pixels;

    // reuse image buffer to avoid mem allocations
    // bool reuse = (ref_count() == 1);

    /* if( && outSize == clip.size()) {
        crop(clip);
        if(doFlip)
            flip();
        std::swap(pixels, m_pixels);
        m_size = SizeI(0,0);
    } else { */
        Image tmpImage(outSize);
        tmpImage.paste(PointI(0,0), shared_from_this(), clip, doFlip);
        std::swap(pixels, tmpImage.getPixels());
    // }

    // convert only when needed
    if(format == SDL_PIXELFORMAT_ABGR8888 && pitch == pb.pitch) {
        // if(reuse) {
            // std::swap(pixels, pb.buffer);
        // } else {
            pb.buffer.resize(pb.width * pb.height);
            memcpy(pb.buffer.data(), pixels.data(), outSize.h * pitch);
        // }
    } else {
        pb.buffer.resize(pb.width * pb.height);
        SDL_ConvertPixels(pb.width, pb.height, SDL_PIXELFORMAT_ABGR8888, pixels.data(), pitch, (SDL_PixelFormat)format, pb.buffer.data(), pb.pitch);
    }

    return pb;
}

void Image::paste(const PointI& dest, const ImagePtr& srcImage, const RectI& srcRect, bool doFlip)
{
    // assert(srcRect.isValid());
    int w = srcRect.width();
    int h = srcRect.height();
    int sx = srcRect.x();
    int sy = srcRect.y();
    for(int y = 0; y < h; ++y)
        memcpy(getPixelData(dest.x, dest.y + y), srcImage->getPixelData(sx, sy + (doFlip ? h-y-1 : y)), w*4);
}

