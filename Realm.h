/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef REALM_H_
#define REALM_H_
#pragma once

#include <vector>
#include "Object.h"

namespace AOD {
  class Object;
}

namespace AOD {
  class Realm {
    std::vector<AOD::Object*> objects;

    std::vector<AOD::Object*> objs_to_rem;
  public:
    void Reset();

    enum UPDATE_FLAGS {
      UPDATE_REMOVE  = 1,
      UPDATE_REFRESH = 2,
      UPDATE_ALL     = 3
    };
    void Update(UPDATE_FLAGS = UPDATE_ALL);

    void Add(AOD::Object*);
    void Remove(AOD::Object*);
    inline size_t R_Objs_Size()   const { return objects.size();     }
    inline size_t R_Objs_To_Rem() const { return objs_to_rem.size(); }
  };
  
  extern AOD::Realm* realm;
  void Add(AOD::Object*);
  void Remove(AOD::Object*);
}

#endif