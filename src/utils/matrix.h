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

#ifndef MATRIX_H
#define MATRIX_H

#include <array>
#include <cstring>
#include <initializer_list>
#include <sstream>

template<int N, int M, typename T = float>
class Matrix
{
public:
    Matrix() { setIdentity(); }
    explicit Matrix(int) {} // construct without initializing identity matrix
    Matrix(const Matrix<N,M,T>& other) = default;
    template<int N2, int M2, typename T2>
    Matrix(const Matrix<N2,M2,T2>& other) { *this = other; }
    template<typename U>
    Matrix(const std::initializer_list<U>& values) { *this = values; }
    template<typename U>
    Matrix(const U *values) { *this = values; }

    void setIdentity();
    bool isIdentity() const;
    void fill(T value);

    Matrix<M,N,T> transposed() const;
    void transpose() { *this = transposed(); }

    T *data() { return m[0]; }
    const T *data() const { return m[0]; }

    T& operator()(int row, int column) { return m[row-1][column-1]; }
    T operator()(int row, int column) const { return m[row-1][column-1]; }

    Matrix<N,M,T>& operator=(const Matrix<N,M,T>& other) = default;
    template<int N2, int M2, typename T2>
    Matrix<N,M,T>& operator=(const Matrix<N2,M2,T2>& other);
    template<typename U>
    Matrix<N,M,T>& operator=(const std::initializer_list<U>& values);
    template<typename U>
    Matrix<N,M,T>& operator=(const U *values);
    Matrix<N,M,T>& operator+=(const Matrix<N,M,T>& other);
    Matrix<N,M,T>& operator-=(const Matrix<N,M,T>& other);
    Matrix<N,M,T>& operator*=(T factor);
    Matrix<N,M,T>& operator/=(T divisor);
    bool operator==(const Matrix<N,M,T>& other) const;
    bool operator!=(const Matrix<N,M,T>& other) const;

private:
    T m[N][M];
};

template<int N, int M, typename T>
void Matrix<N,M,T>::setIdentity() {
    for(int i=0;i<N;++i) {
        for(int j=0;j<M;++j) {
            if(i==j)
                m[i][j] = 1.0f;
            else
                m[i][j] = 0.0f;
        }
    }
}

template<int N, int M, typename T>
bool Matrix<N,M,T>::isIdentity() const {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            if((i==j && m[i][j] != 1.0f) || (i!=j && m[i][j] != 0.0f))
                return false;
    return true;
}

template<int N, int M, typename T>
void Matrix<N,M,T>::fill(T value) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] = value;
}

template<int N, int M, typename T>
Matrix<M,N,T> Matrix<N,M,T>::transposed() const {
    static_assert(M==N, "M==N");
    Matrix<M,N,T> result(1);
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            result.m[j][i] = m[i][j];
    return result;
}

template<int N, int M, typename T>
template<int N2, int M2, typename T2>
Matrix<N,M,T>& Matrix<N,M,T>::operator=(const Matrix<N2,M2,T2>& other) {
    static_assert(N2 >= N && M2 >= M, "incompatible sizes");
    for(int i=1;i<=N;++i)
        for(int j=1;j<=M;++j)
            (*this)(i,j) = other(i,j);
    return *this;
}

template<int N, int M, typename T>
template<typename U>
Matrix<N,M,T>& Matrix<N,M,T>::operator=(const std::initializer_list<U>& values) {
    auto it = values.begin();
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] = *(it++);
    return *this;
}

template<int N, int M, typename T>
template<typename U>
Matrix<N,M,T>& Matrix<N,M,T>::operator=(const U *values) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] = values[i*N + j];
    return *this;
}

template<int N, int M, typename T>
Matrix<N,M,T>& Matrix<N,M,T>::operator+=(const Matrix<N,M,T>& other) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] += other.m[i][j];
    return *this;
}

template<int N, int M, typename T>
Matrix<N,M,T>& Matrix<N,M,T>::operator-=(const Matrix<N,M,T>& other) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] -= other.m[i][j];
    return *this;
}

template<int N, int M, typename T>
Matrix<N,M,T>& Matrix<N,M,T>::operator*=(T factor) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] *= factor;
    return *this;
}

template<int N, int M, typename T>
Matrix<N,M,T>& Matrix<N,M,T>::operator/=(T divisor) {
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            m[i][j] /= divisor;
    return *this;
}

template<int N, int M, typename T>
bool Matrix<N,M,T>::operator==(const Matrix<N,M,T>& other) const
{
    for(int i=0;i<N;++i)
        for(int j=0;j<M;++j)
            if(m[i][j] != other.m[i][j])
                return false;
    return true;
}

template<int N, int M, typename T>
bool Matrix<N,M,T>::operator!=(const Matrix<N,M,T>& other) const
{
    return !(*this == other);
}

template<int N, int M, typename T>
std::ostream& operator<<(std::ostream& out, const Matrix<N,M,T>& mat)
{
    for(int i=1;i<=N;++i) {
        for(int j=1;j<=M;++j) {
            out << mat(i,j);
            if(j != M)
                out << " ";
        }
        out << "\n";
    }
    return out;
}

template<int N, int M, typename T>
std::istream& operator>>(std::istream& in, Matrix<N,M,T>& mat)
{
    for(int i=0;i<N;++i) {
        for(int j=0;j<M;++j) {
            in >> mat(i,j);
        }
    }
    return in;
}

// faster comparing for 3x3 matrixes
template<>
inline bool Matrix<3,3,float>::operator==(const Matrix<3,3,float>& other) const
{
    return m[0][0] == other.m[0][0] && m[1][1] == other.m[1][1] &&
           m[2][1] == other.m[2][1] && m[2][0] == other.m[2][0] &&
           m[1][2] == other.m[1][2] && m[0][2] == other.m[0][2] &&
           m[1][0] == other.m[1][0] && m[0][1] == other.m[0][1] &&
           m[2][2] == other.m[2][2];
}

template<int M, int N, int P, int Q, typename T1, typename T2>
Matrix<M, Q, T1> operator*(const Matrix<M,N,T1>& a, const Matrix<P,Q,T2>& b)
{
    static_assert(N==P, "N==P");
    Matrix<M,Q,T1> c(1);
    for(int i=1;i<=M;++i) {
        for(int j=1;j<=Q;++j) {
            T1 sum = 0;
            for(int k=1;k<=N;++k)
                sum += a(i,k) * b(k,j);
            c(i,j) = sum;
        }
    }
    return c;
}

template<int M, int N, typename T>
Matrix<M, N, T> operator+(const Matrix<M,N,T>& a, const Matrix<M,N,T>& b) {  Matrix<M,N,T> c(a); c += b; return c; }

template<int M, int N, typename T>
Matrix<M, N, T> operator-(const Matrix<M,N,T>& a, const Matrix<M,N,T>& b) {  Matrix<M,N,T> c(a); c -= b; return c; }

template<int M, int N, typename T>
Matrix<M, N, T> operator*(const Matrix<M,N,T>& a, float b) {  Matrix<M,N,T> c(a); c *= b; return c; }

template<int M, int N, typename T>
Matrix<M, N, T> operator/(const Matrix<M,N,T>& a, float b) {  Matrix<M,N,T> c = a; c /= b; return c; }

typedef Matrix<4,4> Matrix4x4;
typedef Matrix<3,3> Matrix3x3;
typedef Matrix<2,2> Matrix2x2;

typedef Matrix4x4 Matrix4;
typedef Matrix3x3 Matrix3;
typedef Matrix2x2 Matrix2;

#endif
