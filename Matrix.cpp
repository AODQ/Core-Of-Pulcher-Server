#include "Matrix.h"

Matrix::Matrix(float _a, float _b, float _c, float _d, float _tx, float _ty) {
  a = _a, b = _b, c = _c, d = _d, tx = _tx, ty = _ty;
  rot = prev_rot = 0;
  scale = {1,1}; 
}

void Matrix::Identity() {
  a = 1;  b = 0;    c = 0;
  d = 1;  tx = 0;  ty = 0;
}

void Matrix::Translate(const AOD::Vector& vec) {
  tx += vec.x;
  ty += vec.y;
}

void Matrix::Translate(float x, float y) {
  tx += x;
  ty += y;
}

void Matrix::Set_Translation(float x, float y) {
  tx = x;
  ty = y;
}

void Matrix::Set_Translation(const AOD::Vector& vec) {
  tx = vec.x;
  ty = vec.y;
}

void Matrix::Compose(const AOD::Vector& pos, float RADIANS,
                     const AOD::Vector& scale) {
  Identity();

  Scale( scale );
  Rotate( rot );
  Set_Translation( pos );
}

void Matrix::Rotate(float r) {
  float x = std::cosf(r),
        y = std::sinf(r);

  float a1 = a * x - b * y;
  b = a * y + b * x;
  a = a1;

  float c1 = c * x - d * y;
  d = c * y + d * x;
  c = c1;

  float tx1 = tx * x - ty * y;
  ty = tx * y + ty * x;
  tx = tx1;
}

void Matrix::Scale(const AOD::Vector& sc) {
  a *= sc.x;
  b *= sc.y;

  c *= sc.x;
  d *= sc.y;

  tx *= sc.x;
  ty *= sc.y;
}
