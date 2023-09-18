#include "qvector5d.h"
#include "vertex.h"
#include <QDataStream>

QVector5D::QVector5D() {}

QVector5D::QVector5D(QVector2D coords, QVector3D color) {
  m_coords = coords;
  m_color = color;
}

QVector2D QVector5D::coords() const {
  return m_coords;
}

QVector3D QVector5D::color() const {
  return m_color;
}

float QVector5D::x() const {
  return m_coords.x();
}

float QVector5D::y() const {
  return m_coords.y();
}

float QVector5D::r() const {
  return m_color.x();
}

float QVector5D::g() const {
  return m_color.y();
}

float QVector5D::b() const {
  return m_color.z();
}

QVector5D& QVector5D::operator*=(float factor)  {
  m_coords *= factor;
  m_color *= factor;
  return *this;
}

QVector5D& QVector5D::operator*=(const QVector5D &vector) {
  m_coords *= vector.coords();
  m_color *= vector.color();
  return *this;
}

QVector5D& QVector5D::operator/=(float factor)  {
  m_coords /= factor;
  m_color /= factor;
  return *this;
}

QVector5D& QVector5D::operator/=(const QVector5D &vector) {
  m_coords /= vector.coords();
  m_color /= vector.color();
  return *this;
}

QVector5D& QVector5D::operator+=(const QVector5D &vector) {
  m_coords += vector.coords();
  m_color += vector.color();
  return *this;
}

QVector5D& QVector5D::operator-=(const QVector5D &vector) {
  m_coords -= vector.coords();
  m_color -= vector.color();
  return *this;
}

const QVector5D	operator*(float factor, const QVector5D &vector) {
  return QVector5D(factor * vector.coords(), factor * vector.color());
}

const QVector5D	operator*(const QVector5D &vector, float factor) {
  return QVector5D(vector.coords() * factor, vector.color() * factor);
}

const QVector5D	operator*(const QVector5D &v1, const QVector5D &v2) {
  return QVector5D(v1.coords() * v2.coords(), v1.color() * v2.color());
}

const QVector5D	operator+(const QVector5D &v1, const QVector5D &v2) {
  return QVector5D(v1.coords() + v2.coords(), v1.color() + v2.color());
}

const QVector5D	operator-(const QVector5D &v1, const QVector5D &v2) {
  return QVector5D(v1.coords() - v2.coords(), v1.color() - v2.color());
}

const QVector5D	operator-(const QVector5D &vector) {
  return QVector5D(-vector.coords(), -vector.color());
}

const QVector5D	operator/(const QVector5D &vector, float divisor) {
  return QVector5D(vector.coords() / divisor, vector.color() / divisor);
}

const QVector5D	operator/(const QVector5D &vector, const QVector5D &divisor) {
  return QVector5D(vector.coords() / divisor.coords(), vector.color() / divisor.color());
}

const QVector<float>& operator<<(QVector<float>&stream, const QVector5D &vector) {
  stream << vector.x() << vector.y() << vector.r() << vector.g() << vector.b();
  return stream;
}
