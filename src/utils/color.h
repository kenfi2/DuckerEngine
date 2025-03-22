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

#ifndef COLOR_H
#define COLOR_H

namespace stdext {
    template<typename T>
    T clamp(T x, T min, T max) { return std::max<T>(min, std::min<T>(x, max)); }
}

class Color {
public:
    explicit Color(int rgba) { setRGBA((uint32_t)rgba); }

    Color() : m_r(1.0f), m_g(1.0f), m_b(1.0f), m_a(1.0f) { }
    Color(uint32_t rgba) { setRGBA(rgba); }
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) : m_r(r / 255.0f), m_g(g / 255.0f), m_b(b / 255.0f), m_a(a / 255.0f) { }
    Color(int r, int g, int b, int a = 0xFF) : m_r(r / 255.0f), m_g(g / 255.0f), m_b(b / 255.0f), m_a(a / 255.0f) { }
    Color(float r, float g, float b, float a = 1.0f) : m_r(r), m_g(g), m_b(b), m_a(a) { }

    bool isValid() const { return m_r >= 0 && m_g >= 0 && m_b >= 0 && m_a >= 0 && m_r <= 1 && m_g <= 1 && m_b <= 1 && m_a <= 1; }

    uint8_t a() const { return (uint8_t)(m_a * 255.0f); }
    uint8_t b() const { return (uint8_t)(m_b * 255.0f); }
    uint8_t g() const { return (uint8_t)(m_g * 255.0f); }
    uint8_t r() const { return (uint8_t)(m_r * 255.0f); }

    float aF() const { return m_a; }
    float bF() const { return m_b; }
    float gF() const { return m_g; }
    float rF() const { return m_r; }

    uint32_t rgba() const { return (uint32_t)r() | (uint32_t)g() << 8 | (uint32_t)b() << 16 | (uint32_t)a() << 24; }

    void normalize() { m_r = stdext::clamp(m_r, 0.0f, 1.0f); m_g = stdext::clamp(m_g, 0.0f, 1.0f); m_b = stdext::clamp(m_b, 0.0f, 1.0f); m_a = stdext::clamp(m_a, 0.0f, 1.0f); }

    void setRed(int r) { m_r = uint8_t(r) / 255.0f; }
    void setGreen(int g) { m_g = uint8_t(g) / 255.0f; }
    void setBlue(int b) { m_b = uint8_t(b) / 255.0f; }
    void setAlpha(int a) { m_a = uint8_t(a) / 255.0f; }

    void setRed(float r) { m_r = r; }
    void setGreen(float g) { m_g = g; }
    void setBlue(float b) { m_b = b; }
    void setAlpha(float a) { m_a = a; }

    void setRGB(Color other) { m_r = other.rF(); m_g = other.gF(); m_b = other.bF(); }
    void setRGB(float r, float g, float b) { m_r = r; m_g = g; m_b = b; }

    void setRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) { m_r = r / 255.0f; m_g = g / 255.0f; m_b = b / 255.0f; m_a = a / 255.0f; }
    void setRGBA(uint32_t rgba) { setRGBA((rgba >> 0) & 0xff, (rgba >> 8) & 0xff, (rgba >> 16) & 0xff, (rgba >> 24) & 0xff); }

    Color operator+(float v) const { return Color(m_r + v, m_g + v, m_b + v, m_a + v); }
    Color& operator+=(float v) { m_r += v; m_g += v; m_b += v; m_a += v; return *this; }
    Color operator-(float v) const { return Color(m_r - v, m_g - v, m_b - v, m_a - v); }
    Color& operator-=(float v) { m_r -= v; m_g -= v; m_b -= v; m_a -= v; return *this; }

    Color operator+(const Color& other) const { return Color(m_r + other.m_r, m_g + other.m_g, m_b + other.m_b, m_a + other.m_a); }
    Color& operator+=(const Color& other) { m_r += other.m_r; m_g += other.m_g; m_b += other.m_b; m_a += other.m_a; return *this; }
    Color operator-(const Color& other) const { return Color(m_r - other.m_r, m_g - other.m_g, m_b - other.m_b, m_a - other.m_a); }
    Color& operator-=(const Color& other) { m_r -= other.m_r; m_g -= other.m_g; m_b -= other.m_b; m_a -= other.m_a; return *this; }

    Color operator*(float v) const { return Color(m_r * v, m_g * v, m_b * v, m_a * v); }
    Color& operator*=(float v) { m_r *= v; m_g *= v; m_b *= v; m_a *= v; return *this; }
    Color operator/(float v) const { return Color(m_r / v, m_g / v, m_b / v, m_a / v); }
    Color& operator/=(float v) { m_r /= v; m_g /= v; m_b /= v; m_a /= v; return *this; }

    Color operator*(const Color& other) const { return Color(m_r * other.m_r, m_g * other.m_g, m_b * other.m_b, m_a * other.m_a); }
    Color& operator*=(const Color& other) { m_r *= other.m_r; m_g *= other.m_g; m_b *= other.m_b; m_a *= other.m_a; return *this; }
    Color operator/(const Color& other) const { return Color(m_r / other.m_r, m_g / other.m_g, m_b / other.m_b, m_a / other.m_a); }
    Color& operator/=(const Color& other) { m_r /= other.m_r; m_g /= other.m_g; m_b /= other.m_b; m_a /= other.m_a; return *this; }

    Color& operator=(uint32_t rgba) { setRGBA(rgba); return *this; }
    bool operator==(uint32_t rgba) const { return this->rgba() == rgba; }

    //Color& operator=(const Color& other) { m_r = other.m_r; m_g = other.m_g; m_b = other.m_b; m_a = other.m_a; return *this; }
    bool operator==(const Color& other) const { return other.rgba() == rgba(); }
    bool operator!=(const Color& other) const { return other.rgba() != rgba(); }
    bool operator<(const Color& other) const { return rgba() < other.rgba(); }
    bool operator>(const Color& other) const { return rgba() > other.rgba(); }

    static uint8_t to8bit(const Color& color) {
        uint8_t c = 0;
        c += (color.r() / 51) * 36;
        c += (color.g() / 51) * 6;
        c += (color.b() / 51);
        return c;
    }

    static Color from8bit(int color) {
        if(color >= 216 || color <= 0)
            return Color(0, 0, 0);

        int r = int(color / 36) % 6 * 51;
        int g = int(color / 6) % 6 * 51;
        int b = color % 6 * 51;
        return Color(r, g, b);
    }

private:
	float m_r, m_g, m_b, m_a;
};

#endif
