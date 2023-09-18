#ifndef QVECTOR5D_H
#define QVECTOR5D_H

#include "halfedge.h"
#include<QVector2D>
#include<QVector3D>

class QVector5D {
public:
  QVector5D();
  QVector5D(QVector2D coords, QVector3D color);
  QVector2D coords() const;
  QVector3D color() const;
  float x() const;
  float y() const;
  float r() const;
  float g() const;
  float b() const;
  QVector5D& operator*=(float factor);
  QVector5D& operator*=(const QVector5D &vector);
  QVector5D& operator/=(float factor);
  QVector5D& operator/=(const QVector5D &vector);
  QVector5D& operator+=(const QVector5D &vector);
  QVector5D& operator-=(const QVector5D &vector);

private:
  QVector2D m_coords;
  QVector3D m_color;
};

const QVector5D	operator*(float factor, const QVector5D &vector);
const QVector5D	operator*(const QVector5D &vector, float factor);
const QVector5D	operator*(const QVector5D &v1, const QVector5D &v2);
const QVector5D	operator+(const QVector5D &v1, const QVector5D &v2);
const QVector5D	operator-(const QVector5D &v1, const QVector5D &v2);
const QVector5D	operator-(const QVector5D &vector);
const QVector5D	operator/(const QVector5D &vector, float divisor);
const QVector5D	operator/(const QVector5D &vector, const QVector5D &divisor);
const QVector<float>& operator<<(QVector<float> &stream, const QVector5D &vector);

#endif // QVECTOR5D_H
