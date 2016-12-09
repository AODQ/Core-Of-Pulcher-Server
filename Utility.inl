  #ifndef UTILITY_INL_
#define UTILITY_INL_
#pragma once

#include <sstream>
#include <iterator>
#include <type_traits>
#include "Utility.h"

// -- linked list -------------------------------------------------------------

// -- linked list node --
template <typename T>
Util::Linked_List_Node<T>::Linked_List_Node(const T _d) {
  prev = next = nullptr;
  data = _d;
}

// -- linked list --

template <typename T>
Util::Linked_List<T>::Linked_List() {
  header = last = nullptr;
  size = 0;
}

template <typename T>
Util::Linked_List<T>::~Linked_List() {
  while ( true ) {
    if ( !header ) return;
    Linked_List_Node<T*>* t = header->next;
    delete header;
    header = t;
  }
}

template <typename T>
Util::Linked_List_Node<T*>* Util::Linked_List<T>::R_Head()
 { return header; }

template <typename T>
Util::Linked_List_Node<T*>* Util::Linked_List<T>::R_Tail()
  { return last; }

template <typename T>
void Util::Linked_List<T>::Push(T* data) {
  if ( size++ == 0 ) {
    header = last = new Linked_List_Node<T*>(data);
    return;
  }
  last->next = new Linked_List_Node<T*>(data);
  auto* _l = last;
  last = last->next;
  last->prev = _l;
}

template <typename T>
T* Util::Linked_List<T>::Pop() {
  if ( size == 0 ) return nullptr;
  --size;
  if ( size != 0 )
    last->prev->next = nullptr;
  Linked_List_Node<T*>* _l = last;
  last = last->prev;
  return _l->data;
}

template <typename T>
int Util::Linked_List<T>::R_Size() const { return size; }

// --- miscel

template <typename T>
T Util::Str_To_T(const std::string& o) {
  std::istringstream con(o);
  T res;
  return con >> res ? res : 0;
}

template<typename T, typename ...Args>
void Util::Reconstruct(T& obj, Args&&... args) {
  static_assert(!std::has_virtual_destructor<T>::value,
                "obj can't have virtual destructor\n");
  obj.~T();
  new (&obj) T(std::forward<Args>(args)...);
}

template <typename T>
inline std::vector<uint8_t> Util::Pack_Num(T num) {
  uint8_t* it = (uint8_t*)&num,
         * last = it + sizeof(T);
  std::vector<uint8_t> vec;
  for (; it != last; ++ it )
    vec.push_back(*it);
  return vec;
}

template <typename T>
inline void Util::Prepend_Pack_Num(std::vector<uint8_t>& vec, T num) {
  Prepend(vec, Pack_Num(num));
}

inline std::vector<uint8_t> Util::Pack_String(const std::string& str) {
  std::vector<uint8_t> v = {};
  if ( str.size() == 0 ) return {0};
  for ( auto i : str )
    v.push_back(i);
  while ( v[v.size()-1] == '\0' )
    v.pop_back();
  if ( v[v.size() - 1] != '\0' )
    v.push_back('\0');
  return v;
}

inline void Util::Append_Pack_String(std::vector<uint8_t>& v,
                                     const std::string& s) {
  Prepend(v, Pack_String(s));
}

template <typename T, typename _IT>
inline T Util::Unpack_Num(_IT begin, int& c_it) {
  T g = 0;
  begin += c_it;
  std::_Is_iterator<_IT>;
  uint8_t* it = (uint8_t*)&g;
  for ( int i = 0; i != sizeof(T); ++ i, ++ c_it, ++begin ) {
    it[i] = *begin;
  }
  return g;
}

template <typename T>
inline T Util::Unpack_Num(const std::vector<uint8_t>& vec) {
  int c = 0;
  return Unpack_Num<T>(vec.begin(), c);
}

template <typename T>
inline T Util::Unpack_Num(std::vector<uint8_t>& vec, int& it) {
  return Unpack_Num<T>(vec.begin(), it);
}


inline std::string Util::Unpack_Str(const std::vector<uint8_t>& str,
                                    int& c_it) {
  std::string g;
  for ( ; c_it != str.size() && str[c_it] != '\0'; ++ c_it )
    g.push_back(str[c_it]);
  ++ c_it; 
  return g;
}
template <typename T>
inline void Util::Prepend(std::vector<T>& vec, const std::vector<T>& app) {
  vec.insert(vec.end(), app.begin(), app.end());
}
#endif