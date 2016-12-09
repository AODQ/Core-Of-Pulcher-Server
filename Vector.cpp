/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Matrix.h"
#include "Vector.h"
#include <cmath>

using AODVec = AOD::Vector;
AODVec::Vector() { x = y = 0; };
AODVec::Vector(float _x, float _y) {
  x = _x;
  y = _y;
}
AOD::Vector::Vector(const std::initializer_list<float>& list) {
  x = *list.begin();
  y = *(list.begin()+1);
}
AOD::Vector::Vector(const std::initializer_list<int>& list) {
  x = *list.begin();
  y = *(list.begin()+1);
}

// operator overloads
AODVec AODVec::operator+(const Vector& _vector) const {
  return Vector(x + _vector.x, y + _vector.y);
}
AODVec AODVec::operator-(const Vector& _vector) const {
  return Vector(x - _vector.x, y - _vector.y);
}
AODVec AODVec::operator/(const Vector& _vector) const {
  return {x/_vector.x, y/_vector.y};
}
AODVec AODVec::operator*(float n) const { return Vector(x * n, y * n); }
AODVec AODVec::operator/(float n) const { return Vector(x / n, y / n); }
AODVec& AODVec::operator/=(float fl) {
  x /= fl;
  y /= fl;
  return *this;
}
AODVec& AODVec::operator*=(float fl) {
  x *= fl;
  y *= fl;
  return *this;
}


AODVec& AODVec::operator+=(const Vector& _vector) {
  x += _vector.x;
  y += _vector.y;
  return (*this);
}
AODVec& AODVec::operator-=(const Vector& _vector) {
  x -= _vector.x;
  y -= _vector.y;
  return (*this);
}

AODVec& AODVec::operator*=(const Vector& _vector) {
  x *= _vector.x;
  y *= _vector.y;
  return (*this);
}
AODVec AOD::Vector::operator*(const Vector & _vector) const {
  return AOD::Vector(
    x * _vector.x,
    y * _vector.y);
}

AODVec& AODVec::operator/=(const Vector& _vector) {
  x /= _vector.x;
  y /= _vector.y;
  return (*this);
}

std::ostream& operator<<(std::ostream& os, const AODVec& _Vector) {
  std::cout.setf ( std::ios::fixed, std::ios::floatfield );
  return os << '<' << _Vector.x << ", " << _Vector.y << '>';
  std::cout.unsetf ( std::ios::floatfield );
}

// utility methods
void AODVec::Normalize() {
  float mag = Magnitude();
  if ( mag > 0 ) {
    x /= mag;
    y /= mag;
  }
}

float AODVec::Distance(const Vector& _vector) {
  return sqrt((x*x - _vector.x*_vector.x) + (y*y - _vector.y*_vector.y));
}

float AODVec::Angle() {
  return atan2f(y, x);
}

float AODVec::Angle(const Vector& _vector) {
  return atan2f(_vector.y - y, _vector.x - x);
}
float AODVec::Magnitude() {
  return sqrt((x*x) + (y*y));
}

float AODVec::Dot_Product(const AODVec& _vector) const {
  return x*_vector.x + y*_vector.y;
}
// projects other vector onto this one
void AODVec::Project(const AODVec& _vector) {
  float dot_prod = Dot_Product(_vector);
  x = (dot_prod / (std::pow(_vector.x, 2) +
                   std::pow(_vector.y,2)) ) * _vector.x;
  y = (dot_prod / (std::pow(_vector.x, 2) +
                   std::pow(_vector.y,2)) ) * _vector.y;
}
// projects projectee onto projected
AODVec AODVec::Project(const Vector& projected, const Vector& projectee) {
  AODVec vector = projected;
  vector.Project(projectee);
  return vector;
}

// gives right hand normal of vector
void AODVec::Right_Normal(const Vector& vec) {
  x =  (vec.x - x);
  y = -(vec.y - y);
}
// gives left hand normal of vector
void AODVec::Left_Normal(const Vector& vec) {
  x = -(vec.x - x);
  y =  (vec.y - y);
}

AODVec AODVec::Reflect(const AODVec& I, const AODVec& N) {
  return I - (N*2.f * I) * N;
}

AOD::Vector AOD::Vector::Transform(const Matrix & mat, const Vector& vec) {
  AOD::Vector v;
  v.x = vec.x * mat.a + vec.y * mat.c + mat.tx;
  v.y = vec.x * mat.b + vec.y * mat.d + mat.ty;
  return v;
}