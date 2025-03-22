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

#ifndef POINT_H
#define POINT_H

#include <sstream>
#include <cmath>

template<class T>
class TSize;

template<class T>
class TPoint
{
public:
    TPoint() : x(0), y(0) { }
    explicit TPoint(int) { }
    TPoint(T x, T y) : x(x), y(y) { }
    TPoint(const TPoint<T>& other) : x(other.x), y(other.y) { }

    static TPoint<T> invalid() { return TPoint<T>(INFINITY, INFINITY); }

    bool isNull() const { return x==0 && y==0; }
    bool isValid() const { return x != INFINITY && y != INFINITY; }

    TPoint<int> toPoint() const { return TPoint<int>(std::lround(x), std::lround(y)); }
    TPoint<float> toPointF() const { return TPoint<float>((float)x, (float)y); }
    TPoint<uint8_t> toPointU8() const { return TPoint<uint8_t>(x, y); }

    bool isInsideTriangle(const TPoint<float>& p1, const TPoint<float>& p2, const TPoint<float>& p3) const {
        auto sign = [&](const TPoint<float>& v1, const TPoint<float>& v2) -> float {
            return (x - v2.x) * (v1.y - v2.y) - (v1.x - v2.x) * (y - v2.y);
        };

        float d1 = sign(p1, p2);
        float d2 = sign(p2, p3);
        float d3 = sign(p3, p1);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);
    }

    TPoint<T> operator-() const { return TPoint<T>(-x, -y); }

    TPoint<T> operator+(const TPoint<T>& other) const { return TPoint<T>(x + other.x, y + other.y); }
    TPoint<T>& operator+=(const TPoint<T>& other) { x+=other.x; y+=other.y; return *this; }
    TPoint<T> operator-(const TPoint<T>& other) const { return TPoint<T>(x - other.x, y - other.y); }
    TPoint<T>& operator-=(const TPoint<T>& other) { x-=other.x; y-=other.y; return *this; }
    TPoint<T> operator*(const TPoint<T>& other) const { return TPoint<T>(x * other.x, y * other.y); }
    TPoint<T>& operator*=(const TPoint<T>& other) { x*=other.x; y*=other.y; return *this; }
    TPoint<T> operator/(const TPoint<T>& other) const { return TPoint<T>(x/other.x, y/other.y); }
    TPoint<T>& operator/=(const TPoint<T>& other)   { x/=other.x; y/=other.y; return *this; }

    TPoint<T> operator+(T other) const { return TPoint<T>(x + other, y + other); }
    TPoint<T>& operator+=(T other) { x+=other; y+=other; return *this; }
    TPoint<T> operator-(T other) const { return TPoint<T>(x - other, y - other); }
    TPoint<T>& operator-=(T other) { x-=other; y-=other; return *this; }
    TPoint<T> operator*(float v) const { return TPoint<T>(x*v, y*v); }
    TPoint<T>& operator*=(float v) { x*=v; y*=v; return *this; }
    TPoint<T> operator/(float v) const { return TPoint<T>(x/v, y/v); }
    TPoint<T>& operator/=(float v) { x/=v; y/=v; return *this; }

    TPoint<T> operator&(int a) { return TPoint<T>(x & a, y & a); }
    TPoint<T>& operator&=(int a) { x &= a; y &= a; return *this; }

    bool operator<=(const TPoint<T>&other) const { return x<=other.x && y<=other.y; }
    bool operator>=(const TPoint<T>&other) const { return x>=other.x && y>=other.y; }
    bool operator<(const TPoint<T>&other) const { return x<other.x || (x==other.x && y<other.y); }
    bool operator>(const TPoint<T>&other) const { return x>other.x && y>other.y; }

    TPoint<T>& operator=(const TPoint<T>& other) { x = other.x; y = other.y; return *this; }
    bool operator==(const TPoint<T>& other) const { return other.x==x && other.y==y; }
    bool operator!=(const TPoint<T>& other) const { return other.x!=x || other.y!=y; }

    float length() const { return hypot(x, y); }
    T manhattanLength() const { return std::abs(x) + std::abs(y); }

    float distanceFrom(const TPoint<T>& other) const { return TPoint<T>(x - other.x, y - other.y).length(); }
    float angleFrom(const TPoint<T>& other) const {
        float a = atan2f(y - other.y, other.x - x);
        if(a < 0)
            a += 2 * SDL_PI_D;
        return a;
    }

    void translate(T tx, T ty) { x += tx; y += ty; }
    void translate(const TPoint<T>& p) { x += p.x; y += p.y; }

    TPoint<T> translated(T tx, T ty) { return TPoint<T>(x + tx, y + ty); }
    TPoint<T> translated(const TPoint<T>& p) { return TPoint<T>(x + p.x, y + p.y); }

    void floor() { x = (int)x; y = (int)y; }

    T getX() const { return x; }
    T getY() const { return y; }

    void setX(T _x) { x = _x; }
    void setY(T _y) { y = _y; }

    T x, y;
};

typedef TPoint<int> PointI;
typedef TPoint<float> PointF;
typedef TPoint<uint8_t> PointU8;

template<class T>
std::ostream& operator<<(std::ostream& out, const TPoint<T>& point)
{
    out << point.x << " " << point.y;
    return out;
}

struct PointHasher {
    std::size_t operator()(const PointI& point) const {
        return (point.x * 8192) + point.y;
    }
};

struct PointFHasher {
    std::size_t operator()(const PointF& point) const {
        return static_cast<std::size_t>((point.x * 8192) + point.y);
    }
};

#endif
