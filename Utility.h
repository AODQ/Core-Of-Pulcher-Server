/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef __UTILITY_H_
#define __UTILITY_H_
#pragma once

#include "Vector.h"

#include <functional>
#include <utility>
#include <random>
#include <string>

// general utility purposed things
namespace Util {
  extern std::random_device rand;
  extern std::mt19937 rgen;
}

void assert(bool condition, std::string error_msg);

class Player_Info;

namespace Util {
  extern const float E,
                     Log10E,
                     Log2E,
                     Pi,
                     Tau,
                     Max_float,
                     Min_float,
                     Epsilon;

  float R_Rand(float bot, float top);

  template <typename T>
  T R_Max(T x, T y);

  template <typename T>
  T R_Min(T x, T y);

  bool Find_Substr(std::string&, std::string);
  
  // if string can not safely convert to T, will return 0. Only works on
  // native types (uint32_t for example)
  template <typename T>
  T Str_To_T(const std::string& x);

  AOD::Vector Rotate_Point(AOD::Vector origin, AOD::Vector point, float ang);

  template <typename T, typename... Args>
  void Reconstruct(T& obj, Args&&... args);

  inline float To_Rad(float x) {
    return x * (Util::Pi/180.0f);
  }

  inline float To_Deg(float x) {
    return x * (180.0f/Util::Pi);
  }
  
  // will extract all tokens in a string using regex. Ei:
  // rgx = "[\w\d]+"
  std::vector<std::string> Extract_Tokens(const std::string&,
                                                std::string& rgx);
  template <typename T>
  class Linked_List_Node {
  public:
    T data;
    Linked_List_Node<T>* next, *prev;
    Linked_List_Node(T data);
  };

  template <typename T>
  class Linked_List {
    Linked_List_Node<T*>* header, *last;
    int size;
  public:
    Linked_List();
    ~Linked_List();
    Linked_List_Node<T*>* R_Head();
    Linked_List_Node<T*>* R_Tail();
    void Push(T* data);
    T* Pop();
    int R_Size() const;
  };

  class Time {
    uint16_t hours, minutes, seconds, milliseconds;
    void Refresh_Value();
  public:
    Time(uint16_t ms = 0, uint16_t min = 0, uint16_t sec = 0, uint16_t h = 0);
    void Set_Time(uint16_t ms = 0, uint16_t min = 0,
                  uint16_t sec = 0, uint16_t h = 0);
    void Set_Time(uint32_t ms);
    void Add_Time(uint16_t ms = 0, uint16_t min = 0,
                  uint16_t sec = 0, uint16_t h = 0);

    std::string R_Time_Str();
    uint16_t R_Hours() const;
    uint16_t R_Minutes() const;
    uint16_t R_Seconds() const;
    uint16_t R_Milliseconds() const;
  };
  
  template <typename T>
  inline std::vector<uint8_t> Pack_Num(T num);
  template <typename T>
  inline void Prepend_Pack_Num(std::vector<uint8_t>& vec, T num);

  inline std::vector<uint8_t> Pack_String(const std::string& str);
  inline void Append_Pack_String(std::vector<uint8_t>&, const std::string&); 

  template <typename T, typename _IT> 
  inline T Unpack_Num(_IT begin, int& c_it);

  template <typename T>
  inline T Unpack_Num(const std::vector<uint8_t>&);

  template <typename T>
  inline T Unpack_Num(std::vector<uint8_t>&, int&);

  inline std::string Unpack_Str(const std::vector<uint8_t>& vec, int& c_it);

  template <typename T>
  inline void Prepend(std::vector<T>&, const std::vector<T>&);

  // Casts an unsigned IP integer to a string
  std::string R_IP(uint32_t host);
  // Returns size of file in bytes (will open the file in binary format)
  uint64_t R_File_Size(const char* filename);

  void Loop_Through_Clients(std::function<void(Player_Info*)>&&);
}

// asserts expression X. If !X, outputs Y and returns Z
#define ret_assert2(X, Y, Z) if ( !(X) ) {\
  AOD::Output(Y);\
  /*__debugbreak();*/\
  return Z; }
// asserts expression X. If !X, outputs Y and returns void()
#define ret_assert(X, Y) if ( !(X) ) {\
  AOD::Output(Y);\
  __debugbreak();\
  return; }

// asserts expression. if expression, writes to err file and
// program exits.
void ex_assert(bool expression, std::string err_msg);

#include "Utility.inl"

#endif