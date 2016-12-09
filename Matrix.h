#ifndef MATRIX_H_
#define MATRIX_H_
#pragma once

#include "Vector.h"

// All angles (rotate, compose) are in RADIANS
class Matrix {
public:
  float a, b, c, d, tx, ty;
  float rot, prev_rot;
  AOD::Vector scale;

  Matrix(float a = 1, float b = 0, float c = 0,
         float d = 1, float tx = 0, float ty = 0);
  void Identity();
  void Translate(const AOD::Vector& vec);
  void Translate(float x, float y);
  void Set_Translation(float x, float y);
  void Set_Translation(const AOD::Vector& vec);
  void Compose(const AOD::Vector& pos, float RADIANS,
               const AOD::Vector& scale);
  void Rotate(float RADIANS);
  void Scale (const AOD::Vector&);
};

#endif