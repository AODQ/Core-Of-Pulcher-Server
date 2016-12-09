/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef VECTOR_H_
#define VECTOR_H_
#pragma once
#include <iostream>

class Matrix;

namespace AOD {
  class Vector {
  public:
    float x, y;
    Vector::Vector();
    Vector::Vector(float x, float y);
    //very hacky... but worthwhile
    Vector::Vector(const std::initializer_list<float>&);
    Vector::Vector(const std::initializer_list<int>&);

    // overload ops - float args
    Vector operator*(float n) const;
    Vector operator/(float n) const;
    Vector& operator/=(float);
    Vector& operator*=(float);

    // overloaded ops - vector args

    Vector operator+(const Vector& _vector) const;
    Vector operator-(const Vector& _vector) const;
    Vector operator*(const Vector& _vector) const;
    Vector operator/(const Vector& _vector) const;
    Vector& operator+=(const Vector& _vector);
    Vector& operator-=(const Vector& _vector);
    Vector& operator*=(const Vector& _vector);
    Vector& operator/=(const Vector& _vector);



    // utilities

    // Sets vector to be distance of 1
    void Normalize();
    // returns angle of vector (origin {0,0})
    float Angle();
    // returns magnitude (length) of vector
    float Magnitude();
    // returns distance between two vectors
    float Distance(const Vector& _vector);
    // returns angle between two vectors
    float Angle(const Vector& _vector);
    // Is the relative orientation of this and _vector. Negative value means
    // they point in the opposite direction -- positive the same direction
    float Dot_Product(const Vector& _vector) const;
    // projects _vector onto this
    void Project(const Vector& _vector);
    // projects projectee (right hand arg) onto projected (left hand arg).
    Vector Project(const Vector& projected, const Vector& projectee);
    // right hand, the vector perpendicular to this on right side
    void Right_Normal(const Vector& _vec);
    // left hand, the vector perpendicular to this on left side
    void Left_Normal(const Vector& _vec);

    // static utils
    // Reflects vectors (must be normalized). Thanks to OpenGL SB6
    // I -> vector to reflect, N -> surface to be reflected from
    static Vector Reflect(const Vector& I, const Vector& N);

    // transforms vector based on matrix and returns new vector
    static AOD::Vector Transform(const Matrix& mat, const Vector& vec);
  };
}

std::ostream& operator<<(std::ostream& os, const AOD::Vector&);
#endif