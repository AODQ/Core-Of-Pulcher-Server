#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_
#pragma once

template <typename T>
inline lerp(float percent, const T& A, const T& B ) {
  return A + ((B - A) * percent);
}



#endif