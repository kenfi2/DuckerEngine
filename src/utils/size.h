/*
 * Copyright (c) 2013-2014
 * edubart <https://github.com/edubart>
 * baxnie <https://github.com/Baxnie>
 * Andre <andreantunes31@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above developers. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef SIZE_H
#define SIZE_H

namespace Luna {
enum AspectRatioMode {
    IgnoreAspectRatio = 0,
    KeepAspectRatio,
    KeepAspectRatioByExpanding
};
}

template<class T>
class TPoint;

template<class T>
class TSize
{
public:
    TSize() : w(-1), h(-1) { }
    explicit TSize(int) { }
    TSize(T width, T height) : w(width), h(height) { }
    TSize(const TSize<T>& other) : w(other.w), h(other.h) { }

    TPoint<int> toPoint() const;
    TPoint<float> toPointF() const;
    TPoint<uint8_t> toPointU8() const;

    TSize<int> toSize() const { return TSize<int>(std::lround(w), std::lround(h)); }
    TSize<float> toSizeF() const { return TSize<float>(w, h); }
    TSize<uint8_t> toSizeU8() const { return TSize<uint8_t>(w, h); }

    bool isNull() const { return w==0 && h==0; }
    bool isEmpty() const { return w<1 || h<1; }
    bool isValid() const { return w>=0 && h>=0; }
    bool hasArea() const { return w>0 && h>0; }

    void resize(T width, T height) { w = width; h = height; }

    T max() const { return (w > h) ? w : h; }
    T min() const { return (w < h) ? w : h; }

    TSize<T> operator-() const { return TSize<T>(-w, -h); }
    TSize<T> operator+(const TSize<T>& other) const { return TSize<T>(w + other.w, h + other.h);   }
    TSize<T>& operator+=(const TSize<T>& other) { w+=other.w; h+=other.h; return *this; }
    TSize<T> operator-(const TSize<T>& other) const { return TSize<T>(w - other.w, h - other.h);   }
    TSize<T>& operator-=(const TSize<T>& other) { w-=other.w; h-=other.h; return *this; }
    TSize<T> operator*(const TSize<T>& other) const { return TSize<T>((T)other.w*w, (T)h*other.h);  }
    TSize<T>& operator*=(const TSize<T>& other) { w=(T)other.w*w; h=(T)h*other.h; return *this; }
    TSize<T> operator/(const TSize<T>& other) const { return TSize<T>((T)w/other.w, (T)h/other.h);  }
    TSize<T>& operator/=(const TSize<T>& other) { (T)w/=other.w; (T)h/=other.h; return *this; }
    TSize<T> operator*(const float v) const { return TSize<T>((T)w*v, (T)h*v);  }
    TSize<T>& operator*=(const float v) { w=(T)w*v; h=(T)h*v; return *this; }
    TSize<T> operator/(const float v) const { return TSize<T>((T)w/v, (T)h/v);  }
    TSize<T>& operator/=(const float v) { w/=v; h/=v; return *this; }

    bool operator<=(const TSize<T>&other) const { return w<=other.w || h<=other.h; }
    bool operator>=(const TSize<T>&other) const { return w>=other.w || h>=other.h; }
    bool operator<(const TSize<T>&other) const { return w<other.w || h<other.h; }
    bool operator>(const TSize<T>&other) const { return w>other.w || h>other.h; }

    TSize<T>& operator=(const TSize<T>& other) { w = other.w; h = other.h; return *this; }
    bool operator==(const TSize<T>& other) const { return other.w==w && other.h==h; }
    bool operator!=(const TSize<T>& other) const { return other.w!=w || other.h!=h; }

    TSize<T> expandedTo(const TSize<T>& other) const { return TSize<T>(std::max(w,other.w), std::max(h,other.h)); }
    TSize<T> boundedTo(const TSize<T>& other) const { return TSize<T>(std::min(w,other.w), std::min(h,other.h)); }

    TSize<T> normalized() const { T maxValue = max(); return TSize<T>(w / maxValue, h / maxValue); }

    void normalize() {
        T maxValue = max();
        w /= maxValue;
        h /= maxValue;
    }

    void scale(const TSize<T>& s, Luna::AspectRatioMode mode) {
        if(mode == Luna::IgnoreAspectRatio || w == 0 || h == 0) {
            w = s.w;
            h = s.h;
        } else {
            bool useHeight;
            T rw = (s.h * w) / h;

            if(mode == Luna::KeepAspectRatio)
                useHeight = (rw <= s.w);
            else // mode == Luna::KeepAspectRatioByExpanding
                useHeight = (rw >= s.w);

            if(useHeight) {
                w = rw;
                h = s.h;
            } else {
                h = (s.w * h)/w;
                w = s.w;
            }
        }
    }
    void scale(T w, T h, Luna::AspectRatioMode mode) { scale(TSize<T>(w, h), mode); }

    float ratio() const { return (float)w/h; }
    T area() const { return w*h; }

    T getW() const { return w; }
    T getH() const { return h; }

    void setW(T _w) { w = _w; }
    void setH(T _h) { h = _h; }

    T w, h;
};

typedef TSize<int> SizeI;
typedef TSize<float> SizeF;
typedef TSize<uint8_t> SizeU8;

template<class T>
std::ostream& operator<<(std::ostream& out, const TSize<T>& size)
{
    out << size.w << " " << size.h;
    return out;
}

template<class T>
std::istream& operator>>(std::istream& in, TSize<T>& size)
{
    DPUnit w, h;
    in >> w >> h;
    size.w = (T)w;
    size.h = (T)h;
    return in;
}

#include "point.h"

template<class T>
TPoint<int> TSize<T>::toPoint() const { return TPoint<int>(std::round((double)w), std::round((double)h)); }

template<class T>
TPoint<float> TSize<T>::toPointF() const { return TPoint<float>(w, h); }

template<class T>
TPoint<uint8_t> TSize<T>::toPointU8() const { return TPoint<uint8_t>(std::round((double)w), std::round((double)h)); }

#endif
